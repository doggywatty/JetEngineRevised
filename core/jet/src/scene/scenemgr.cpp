// core/jet/src/scene/scenemgr.cpp
#include "scene/scenemgr.h"
#include "video/driver.h"
#include <algorithm>
#include <cmath>

namespace jet::scene {

SceneMgr *SceneMgr::s_sceneMgr = 0;

SceneMgr::SceneMgr()
    : m_isRendering(false), m_camera(0), m_cullCamera(0), m_screenHeight(0), m_tanHalfFov(0), m_cullMethods(CullFrustum) {
    SetDistanceCulling(0.f, 999999.f);
    SetScreenRadiusCulling(3.f);
    s_sceneMgr = this;
}

SceneMgr::~SceneMgr() {
    s_sceneMgr = 0;
}

void SceneMgr::Add(Occluder *o) { m_occluders.push_back(o); }
void SceneMgr::Remove(Occluder *o) { m_occluders.erase(std::remove(m_occluders.begin(), m_occluders.end(), o), m_occluders.end()); }

void SceneMgr::Add(const jet::String &tag, Renderable *r) { Add(video::Driver::GetCurrent()->GetTag(tag), r); }
void SceneMgr::Remove(const jet::String &tag, Renderable *r) { Remove(video::Driver::GetCurrent()->GetTag(tag), r); }
void SceneMgr::Render(const jet::String &tag) { Render(video::Driver::GetCurrent()->GetTag(tag)); }
void SceneMgr::ClearRenderables(const jet::String &tag) { ClearRenderables(video::Driver::GetCurrent()->GetTag(tag)); }

// Skipped when the renderable has nothing in this tag's materials (@0x68e948).
void SceneMgr::Add(unsigned tag, Renderable *r) {
    if (r && !(r->GetRenderTagMask() & (1u << (tag & 0xFF)))) return;
    m_renderables[tag].push_back(r);
}

void SceneMgr::Remove(unsigned tag, Renderable *r) {
    std::vector<Renderable*> &v = m_renderables[tag];
    v.erase(std::remove(v.begin(), v.end(), r), v.end());
}

void SceneMgr::AddToAllTags(Renderable *r) {
    for (unsigned i = 0, n = video::Driver::GetCurrent()->GetTagCount(); i != n; i++) Add(i, r);
}

void SceneMgr::RemoveFromAllTags(Renderable *r) {
    for (unsigned i = 0, n = video::Driver::GetCurrent()->GetTagCount(); i != n; i++) Remove(i, r);
}

void SceneMgr::ClearRenderablesFromAllTags() {
    for (unsigned i = 0; i != kTagCount; i++) ClearRenderables(i);
}

namespace {
struct SortByDistance {
    const Camera *cam;
    bool operator()(Occluder *a, Occluder *b) const {
        math::vec3<float> ca, cb;
        float ra, rb;
        a->GetBoundingSphere(ca, ra);
        b->GetBoundingSphere(cb, rb);
        return math::distanceSq(cam->GetPosition(), ca) < math::distanceSq(cam->GetPosition(), cb);
    }
};
}

void SceneMgr::SortOccludersByDistance() {
    if (m_occluders.empty()) return;
    SortByDistance f = { m_camera };
    std::sort(m_occluders.begin(), m_occluders.end(), f);
}

// Sort by distance, set each occluder up for the camera, then drop occluders another occluder hides
// (@0x68f08c).
void SceneMgr::SetupOccluders() {
    SortOccludersByDistance();
    for (size_t i = 0; i < m_occluders.size(); i++) m_occluders[i]->Setup(m_camera);
    if (m_occluders.size() < 2) return;
    for (size_t i = 0; i < m_occluders.size(); i++) {
        if (!m_occluders[i]) continue;
        for (size_t j = i + 1; j < m_occluders.size(); j++) {
            if (!m_occluders[j]) continue;
            math::vec3<float> c;
            float r;
            m_occluders[j]->GetBoundingSphere(c, r);
            if (m_occluders[i]->Occlude(c, r)) m_occluders[j] = 0;
        }
    }
    m_occluders.erase(std::remove(m_occluders.begin(), m_occluders.end(), (Occluder*)0), m_occluders.end());
}

bool SceneMgr::CullDistanceTest(float radius, float distance) const {
    return (m_cullMethods & CullDistance) && radius <= m_distCullRadius && m_distCullDistance <= distance;
}

bool SceneMgr::CullScreenRadiusTest(float radius, float distance, float *out) const {
    if (!(m_cullMethods & CullScreenRadius) || !(distance >= 0.f)) return false;
    float s = (radius / (distance * m_tanHalfFov)) * m_screenHeight;
    if (out) *out = s;
    return s < m_screenCullRadius;
}

bool SceneMgr::CullOcclusionTest(const math::vec3<float> &c, float r) const {
    if (!(m_cullMethods & CullOcclusion)) return false;
    for (size_t i = 0; i < m_occluders.size(); i++)
        if (m_occluders[i]->Occlude(c, r)) return true;
    return false;
}

CullStatusDef::type SceneMgr::CullSphere(const math::vec3<float> &c, float r) const {
    Camera *cam = GetCullCamera();
    if (!cam) return CullStatusDef::Inside;
    float d = std::sqrt(math::distanceSq(cam->GetPosition(), c));
    if (!(m_cullMethods & CullDistance) || m_distCullRadius < r || d - r < m_distCullDistance) {
        if (CullScreenRadiusTest(r, d, 0)) return CullStatusDef::Outside;
        CullStatusDef::type s = CullStatusDef::Inside;
        if (m_cullMethods & CullFrustum) {
            s = cam->GetFrustum().CullSphere(c, r);
            if (s == CullStatusDef::Outside) return s;
        }
        return CullOcclusionTest(c, r) ? CullStatusDef::Outside : s;
    }
    return CullStatusDef::Outside;
}

// Distance, then screen radius, then frustum (sphere then box), then occluders; true when culled
// (@0x68f808).
bool SceneMgr::Cull(Renderable *r) const {
    Camera *cam = GetCullCamera();
    if (!cam) return false;
    math::vec3<float> c = r->GetBoundingSphereCenter();
    float rad = r->GetBoundingSphereRadius();
    float d = std::sqrt(math::distanceSq(cam->GetPosition(), c));
    r->SetDistanceToCamera(d);
    if ((m_cullMethods & CullDistance) && r->GetDistanceCulling() && rad <= m_distCullRadius && m_distCullDistance <= d - rad) {
        r->SetCullStatus(CullStatusDef::Outside);
        return true;
    }
    float sr = 999999.f;
    if (r->GetScreenRadiusCulling() && CullScreenRadiusTest(rad, d, &sr)) {
        r->SetCullStatus(CullStatusDef::Outside);
        return true;
    }
    r->SetScreenRadius(sr);
    CullStatusDef::type s = CullStatusDef::Inside;
    if ((m_cullMethods & CullFrustum) && r->GetFrustumCulling()) s = cam->GetFrustum().CullSphere(c, rad);
    r->SetCullStatus(s);
    if (s == CullStatusDef::Intersect && r->HasBoundingBox()) {
        s = cam->GetFrustum().CullAABB(r->GetBoundingBox());
        r->SetCullStatus(s);
    }
    if (s == CullStatusDef::Outside) return true;
    if (CullOcclusionTest(c, rad)) {
        r->SetCullStatus(CullStatusDef::Outside);
        return true;
    }
    return false;
}

void SceneMgr::Render(unsigned tag) {
    video::Driver *drv = video::Driver::GetCurrent();
    s_sceneMgr = this;
    m_isRendering = true;
    if (m_camera) drv->SetCamera(m_camera);
    SetupOccluders();
    Camera *cam = GetCullCamera();
    m_screenHeight = cam ? (float)cam->GetViewportSize().y : 1.f;
    m_tanHalfFov = cam ? std::tan(cam->GetFov() * 0.5f) : 1.f;
    drv->SetTagMask(1u << (tag & 0xFF));
    std::vector<Renderable*> &list = m_renderables[tag];
    for (size_t i = 0; i < list.size(); i++) {
        Renderable *r = list[i];
        if (!m_hidden.empty() && m_hidden.count(r)) continue;
        if (r->PreCull() && !Cull(r)) {
            r->PreRender();
            r->Render();
        }
    }
    list.clear();
    ClearOccluders();
    m_isRendering = false;
    drv->ClearTagMask();
}

}
