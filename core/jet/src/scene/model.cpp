// core/jet/src/scene/model.cpp
#include "scene/model.h"
#include "scene/node.h"
#include "scene/multiresmesh.h"
#include "scene/camera.h"
#include "anim/animation.h"
#include "scene/meshinstance.h"
#include "scene/scenemgr.h"
#include <cmath>
#include <algorithm>

namespace jet::scene {

float Model::s_lodScreenSize[4] = { 140.f, 80.f, 20.f, 5.f };

// Model(base) @0x6fde9c: materials copied from the base, a Node per base node (parents resolved
// against this instance), a MultiResMesh per base multires mesh and a Camera per base camera.
Model::Model(boost::shared_ptr<const ModelBase> base)
    : m_autoLod(true), m_base(base), m_animation(0), m_texAnimation(0), m_lod(1), m_lodOffset(0), m_isVisible(true), m_animator(0), m_cc(0), m_ctc(0),
      m_updateMode(1), m_animFlags(0x7f), m_wasAnimated(false), m_lastAnimTime(-1), m_lastTexTime(-1), m_interpCur(0), m_interpTotal(0),
      m_movementNode(-1), m_moveLastTime(0), m_moveLastPos(0, 0, 0), m_moveDelta(0, 0, 0), m_moveAccum(0, 0, 0), m_moveWrapped(false) {
    m_materials.resize(base->GetMaterialCount());
    for (unsigned i = 0; i < m_materials.size(); i++) m_materials[i] = base->GetMaterialData(i).material;

    for (unsigned i = 0; i < 4; i++) m_lodScreenSize[i] = s_lodScreenSize[i];
    m_hasBoundingBox = true;
    m_nodes.resize(base->GetNodeCount());
    for (unsigned i = 0; i < m_nodes.size(); i++) m_nodes[i].SetModel(this);
    for (unsigned i = 0; i < m_nodes.size(); i++) m_nodes[i].Load(base->GetNode(i));

    m_dummies.reserve(base->GetDummyCount());
    for (unsigned i = 0; i < base->GetDummyCount(); i++) {
        const ModelBase::DummyData &dd = base->GetDummy(i);
        m_dummies.push_back(Dummy(&m_nodes[dd.nodeIdx], dd.name, (unsigned)dd.number));
    }

    m_multiResMeshes.resize(base->GetMultiResMeshCount());
    for (unsigned i = 0; i < m_multiResMeshes.size(); i++) {
        m_multiResMeshes[i] = new MultiResMesh();
        m_multiResMeshes[i]->SetModel(this);
        m_multiResMeshes[i]->Load(base->GetMultiResMesh(i));
    }

    // Camera nodes live beside the regular ones; a camera's parent index points into m_nodes.
    m_cameraNodes.resize(base->GetCameraCount());
    m_cameras.resize(base->GetCameraCount());
    for (unsigned i = 0; i < m_cameras.size(); i++) {
        m_cameraNodes[i].SetModel(this);
        m_cameraNodes[i].Load(base->GetCamera(i));
        m_cameras[i] = new Camera(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
        m_cameras[i]->Load(base->GetCamera(i));
    }
}

Model::~Model() {
    delete m_cc;
    delete m_ctc;
    for (unsigned i = 0; i < m_multiResMeshes.size(); i++) delete m_multiResMeshes[i];
    for (unsigned i = 0; i < m_cameras.size(); i++) delete m_cameras[i];
    m_base->RemoveInstance(this);
}

const Node *Model::FindNodeByName(const jet::String &n) const {
    int i = m_base->FindNodeIdxByName(n);
    return i < 0 ? 0 : &m_nodes[(unsigned)i];
}

void Model::SetAllMaterials(const jet::video::Material &m) {
    for (unsigned i = 0; i < m_materials.size(); i++) m_materials[i] = m;
}

// GetLod @0x6fd028
unsigned Model::GetLod() const {
    if (!m_lodOffset) return m_lod;
    int l = (int)m_lod + m_lodOffset;
    if (l < 1) l = 1;
    return l < (int)m_base->GetMaxLod() ? (unsigned)l : m_base->GetMaxLod();
}

void Model::SetTransform(const math::vec3<float> &p, const math::quat<float> &q, const math::vec3<float> &s) {
    m_nodes[0].SetTransform(p, q, s);
}

// GetMaterialsRenderTagMask @0x687284
unsigned Model::GetRenderTagMask() {
    unsigned m = 0;
    for (unsigned i = 0; i < m_materials.size(); i++) m |= m_materials[i].GetAllTagMasks();
    return m;
}

// _UpdateCachedBoundingVolume @0x6877a4: union of the current lods' world boxes, sphere around it
void Model::UpdateBoundingVolume() {
    bool first = true;
    for (unsigned i = 0; i < m_multiResMeshes.size(); i++) {
        MeshInstance *mi = m_multiResMeshes[i]->GetMeshInstance(GetLod());
        if (!mi) continue;
        mi->UpdateBoundingVolume();
        if (first) { m_boundingBox = mi->GetBoundingBox(); first = false; }
        else m_boundingBox.addInternalBox(mi->GetBoundingBox());
    }
    m_boundingBox.repair();
    m_center = (m_boundingBox.minPoint + m_boundingBox.maxPoint) * 0.5f;
    m_radius = std::sqrt((m_boundingBox.maxPoint - m_boundingBox.minPoint).getLengthSq() * 0.25f);
}

math::aabb<float> Model::ComputeAbsoluteAccurateBoundingBox() {
    UpdateBoundingVolume();
    return m_boundingBox;
}

// @0x70114c
math::aabb<float> Model::ComputeAccurateBoundingBox() const {
    math::aabb<float> box;
    bool first = true;
    for (unsigned i = 0; i < m_multiResMeshes.size(); i++)
        for (unsigned lod = 1; lod < ModelBase::k_maxLodCount; lod++) {
            const MeshInstance *mi = m_multiResMeshes[i]->GetLoadedMeshInstance(lod);
            if (!mi) continue;
            if (first) { box = mi->GetBoundingBox(); first = false; }
            else box.addInternalBox(mi->GetBoundingBox());
        }
    return box;
}

bool Model::PreCull() {
    if (m_updateMode == 1 && m_animation) AnimateNodes(false);
    UpdateBoundingVolume();
    return true;
}

// PreRender @0x687954: lod from the projected radius, 5% hysteresis around the current lod
void Model::PreRender() {
    if (m_texAnimation) AnimateMaterials(false);
    SceneMgr *sm = SceneMgr::GetSceneMgr();
    Camera *cam = sm ? sm->GetCullCamera() : 0;
    if (m_autoLod && cam && !cam->IsOrthogonal()) {
        unsigned lod = m_lod;
        if (m_distanceToCamera < m_radius) lod = 1;
        else {
            float s = (m_radius / (std::tan(cam->GetFov() * 0.5f) * m_distanceToCamera)) * (float)cam->GetViewportSize().y;
            const float k = 1.05f;
            switch (m_lod) {
            case 5: if (m_lodScreenSize[3] * k < s) lod = 4; break;
            case 4:
                if (m_lodScreenSize[2] * k < s) lod = 3;
                if (s < m_lodScreenSize[3] * k) lod = 5;
                break;
            case 3:
                if (m_lodScreenSize[1] * k < s) lod = 2;
                if (s < m_lodScreenSize[2] * k) lod = 4;
                break;
            case 2:
                if (m_lodScreenSize[0] * k < s) lod = 1;
                if (s < m_lodScreenSize[1] * k) lod = 3;
                break;
            case 1: if (s < m_lodScreenSize[0] * k) lod = 2; break;
            }
        }
        m_lod = lod < m_base->GetMaxLod() ? lod : m_base->GetMaxLod();
    }
}

// Render @0x687b48
void Model::Render() {
    if (!m_isVisible) return;
    for (unsigned i = 0; i < m_multiResMeshes.size(); i++) m_multiResMeshes[i]->Render();
}

}

namespace jet::scene {

static const jet::String k_movementNode("Movement_Dummy");
static const jet::String k_cameraParentNode("camera001");

Model::Controller &Model::CC() const {
    if (!m_cc) m_cc = new Controller(const_cast<Model*>(this), Controller::Nodes);
    return *m_cc;
}

Model::Controller &Model::CTC() const {
    if (!m_ctc) m_ctc = new Controller(const_cast<Model*>(this), Controller::Textures);
    return *m_ctc;
}

Dummy *Model::FindDummyByName(const jet::String &n, unsigned idx) {
    unsigned seen = 0;
    for (unsigned i = 0; i < m_dummies.size(); i++)
        if (m_dummies[i].GetName().Equals(n)) {
            if (seen == idx) return &m_dummies[i];
            seen++;
        }
    return 0;
}

unsigned Model::GetDummyCountByName(const jet::String &n) const {
    unsigned c = 0;
    for (unsigned i = 0; i < m_dummies.size(); i++)
        if (m_dummies[i].GetName().Equals(n)) c++;
    return c;
}

void Model::ResetDummies() {
    for (unsigned i = 0; i < m_dummies.size(); i++) m_dummies[i].Reset();
}

// RefreshAnimatedItems @0x687cc0: which model nodes / cameras each animation channel drives
void Model::RefreshAnimatedItems() {
    jet::anim::Animation *a = m_animation;
    m_animItems.clear();
    m_cameraItems.clear();
    unsigned n = a->GetNodeCount();
    if (m_animData.empty()) m_animData.resize(m_nodes.size());
    for (unsigned i = 0; i < m_animData.size(); i++) m_animData[i].animated = false;
    m_movementNode = a->FindNodeIdxByName(k_movementNode);
    for (unsigned i = 0; i < n; i++) {
        const jet::String &name = a->GetNodeName(i);
        int node = m_base->FindNodeIdxByName(name);
        if (node >= 1) {
            m_animItems.push_back((int)(i | ((unsigned)node << 16)));
            AnimData &d = m_animData[node];
            d.hasPos = a->HasNodePosChannel(i);
            d.hasScale = a->HasNodeScaleChannel(i);
            d.animated = d.hasScale || a->HasNodeRotChannel(i) || d.hasPos;
            if (!(m_animFlags & 0x80)) {
                if (!d.hasPos) m_nodes[node].SetPosition(a->GetNodeInitPos(i));
                if (!a->HasNodeRotChannel(i)) m_nodes[node].SetRotation(a->GetNodeInitRot(i));
                if (!d.hasScale) m_nodes[node].SetScale(a->GetNodeInitScale(i));
            }
        } else {
            int cam = m_base->FindCameraIdxByName(name);
            if (cam >= 0) m_cameraItems.push_back((int)(i | ((unsigned)cam << 16)));
        }
    }
}

void Model::RefreshAnimatedMaterials() {
    jet::anim::Animation *a = m_texAnimation;
    m_materialItems.clear();
    for (unsigned i = 0; i < a->GetMaterialCount(); i++) {
        int m = m_base->FindMaterialIdxByName(a->GetMaterialName(i));
        if (m >= 0) m_materialItems.push_back((int)(i | ((unsigned)m << 16)));
    }
}

void Model::ResetCaches() {
    for (unsigned i = 0; i < m_animData.size(); i++) m_animData[i].cache[0] = m_animData[i].cache[1] = m_animData[i].cache[2] = -1;
}

// SaveNodeOldData @0x687c38: the pose interpolation starts from
void Model::SaveNodeOldData(unsigned i) {
    const Node &n = m_nodes[i];
    AnimData &d = m_animData[i];
    if ((n.GetFlags() & 4) && (m_animFlags & 1)) {
        d.oldPos = n.GetAbsolutePosition();
        d.oldRot = n.GetAbsoluteRotation();
    } else {
        d.oldPos = n.GetPosition();
        d.oldRot = n.GetRotation();
    }
}

bool Model::SetAnimation(jet::anim::Animation *a, short interpolation) {
    Controller &cc = CC();
    cc.SetLoop(0, a ? (int)a->GetDuration() : 0);
    if (a && (m_animation != a || true)) {
        if (!a->IsLoaded()) a->Load();
        if (m_animData.empty()) m_animData.resize(m_nodes.size());
        m_animation = a;
        RefreshAnimatedItems();
        if (m_animItems.empty() && m_cameraItems.empty()) { m_animation = 0; return false; }
    } else {
        m_animation = a;
    }
    if (!m_wasAnimated || !(m_animFlags & 0x80)) interpolation = 0;
    m_interpTotal = interpolation;
    m_lastAnimTime = -1;
    m_interpCur = 0;
    m_wasAnimated = false;
    cc.Reset();
    m_movementNode = a ? a->FindNodeIdxByName(k_movementNode) : -1;
    m_moveLastTime = cc.GetTime();
    m_moveDelta = m_moveAccum = math::vec3<float>(0, 0, 0);
    ResetCaches();
    ResetDummies();
    for (unsigned i = 0; i < m_animData.size(); i++) SaveNodeOldData(i);
    return true;
}

bool Model::SetTextureAnimation(jet::anim::Animation *a) {
    Controller &ctc = CTC();
    ctc.SetLoop(0, a ? (int)a->GetDuration() : 0);
    if (m_texAnimation == a && (!a || !ctc.IsFinished())) return true;
    if (a) {
        if (!a->IsLoaded()) a->Load();
        m_texAnimation = a;
        RefreshAnimatedMaterials();
        if (m_materialItems.empty()) { m_texAnimation = 0; return false; }
    } else {
        m_texAnimation = 0;
    }
    m_lastTexTime = -1;
    ctc.Reset();
    return true;
}

// RefreshMovementDelta @0x6882c4: how far the Movement_Dummy node moved since the last time
void Model::RefreshMovementDelta(bool apply) const {
    Model *self = const_cast<Model*>(this);
    if (m_movementNode < 0 || !m_animation) return;
    int t = CC().GetTime();
    if (t == m_moveLastTime) return;
    math::vec3<float> p(0, 0, 0);
    bool has = m_animation->SampleNodePos(p, (unsigned)m_movementNode, (unsigned)t);
    if (!has) {
        if (apply) self->m_moveDelta = math::vec3<float>(0, 0, 0);
    } else {
        math::vec3<float> d;
        if (!m_moveWrapped) d = p - m_moveLastPos;
        else d = math::vec3<float>(0, 0, 0);
        self->m_moveLastPos = p;
        if (apply) {
            self->m_moveDelta = d;
            self->m_moveAccum = m_moveAccum + d;
        }
    }
    self->m_moveLastTime = t;
}

// AnimateNodes @0x68892c: sample the animation at the controller's time into the nodes; while an
// interpolation runs, blend from the pose it started at
void Model::AnimateNodes(bool force) {
    int time = CC().GetTime();
    if (m_lastAnimTime == time && !force) return;
    if (time < CC().GetPrevTime()) ResetCaches();
    m_wasAnimated = true;
    m_lastAnimTime = time;
    float f = 1.0f;
    bool blend = false;
    if (!IsCulled() && m_interpCur < m_interpTotal) {
        f = m_interpTotal ? (float)m_interpCur / (float)m_interpTotal : 0.0f;
        blend = true;
    }
    for (unsigned i = 1; i < m_nodes.size(); i++)
        if (!(m_nodes[i].GetFlags() & 0x10000) && !m_animData[i].animated) m_nodes[i].SetTransform(m_base->GetNode(i).position, m_base->GetNode(i).rotation, m_base->GetNode(i).scale);
    for (int k = (int)m_animItems.size() - 1; k >= 0; k--) {
        unsigned item = (unsigned)m_animItems[k], a = item & 0xffff, ni = item >> 16;
        Node &n = m_nodes[ni];
        if (n.GetFlags() & 0x10000) continue;
        AnimData &d = m_animData[ni];
        math::vec3<float> pos, scale;
        math::quat<float> rot;
        if (m_animation->SampleNodePos(pos, a, (unsigned)time, &d.cache[0])) n.SetPosition(pos);
        if (m_animation->SampleNodeRot(rot, a, (unsigned)time, &d.cache[1])) n.SetRotation(rot);
        if (d.hasScale && m_animation->SampleNodeScale(scale, a, (unsigned)time, &d.cache[2])) n.SetScale(scale);
    }
    if (blend)
        for (int k = (int)m_animItems.size() - 1; k >= 0; k--) {
            unsigned ni = (unsigned)m_animItems[k] >> 16;
            Node &n = m_nodes[ni];
            if ((n.GetFlags() & 0x10000) || (n.GetFlags() & 0x81)) continue;
            AnimData &d = m_animData[ni];
            math::vec3<float> pos = n.GetPosition();
            const bool absBlend = (n.GetFlags() & 4) && (m_animFlags & 1);
            if (absBlend && n.GetParent()) {
                math::vec3<float> local = math::transform(math::inverse(n.GetParent()->GetAbsoluteTransform()), d.oldPos);
                pos = local + (pos - local) * f;
            } else if (m_animFlags & 4) {
                pos = d.oldPos + (pos - d.oldPos) * f;
            }
            n.SetPosition(pos);
            math::quat<float> rot = n.GetRotation();
            if (absBlend && n.GetParent()) {
                math::quat<float> pr = n.GetParent()->GetAbsoluteRotation();
                math::quat<float> old = math::quat<float>(-pr.x, -pr.y, -pr.z, pr.w) * d.oldRot;
                rot = math::lerp(old, rot, f);
            } else if (m_animFlags & 2) {
                rot = math::lerp(d.oldRot, rot, f);
            }
            n.SetRotation(rot);
        }
    if (!m_cameraItems.empty()) {
        Node *parent = const_cast<Node*>(FindNodeByName(k_cameraParentNode));
        for (int k = (int)m_cameraItems.size() - 1; k >= 0; k--) {
            unsigned item = (unsigned)m_cameraItems[k], a = item & 0xffff;
            Camera &cam = *m_cameras[item >> 16];
            math::vec3<float> pos, scale;
            math::quat<float> rot;
            if (m_animation->SampleNodePos(pos, a, (unsigned)time)) {
                if (parent) pos = math::transform(parent->GetAbsoluteTransform(), pos);
                cam.SetPosition(pos);
            }
            if (m_animation->SampleNodeRot(rot, a, (unsigned)time)) cam.SetRotation(rot);
            if (m_animation->HasNodeScaleChannel(a)) {
                m_animation->SampleNodeScale(scale, a, (unsigned)time);
                cam.SetFov(2.0f * std::atan(std::tan(scale.x * 0.0087266462f) / scale.y));
            }
        }
    }
}

// _AnimateMaterials @0x6891ec: the texture coordinate offset of each animated material layer
void Model::AnimateMaterials(bool force) {
    int time = CTC().GetTime();
    if (m_lastTexTime == time && !force) return;
    m_lastTexTime = time;
    for (int k = (int)m_materialItems.size() - 1; k >= 0; k--) {
        unsigned item = (unsigned)m_materialItems[k], am = item & 0xffff, mi = item >> 16;
        jet::video::Material &mat = m_materials[mi];
        if (!mat.GetRenderPassCount()) continue;
        unsigned layers = std::min(mat.GetTextureLayerCount(0), m_texAnimation->GetMaterialTexChannelCount(am));
        for (unsigned l = 0; l < layers; l++) {
            math::vec2<float> p(0, 0);
            math::mat4<float> m;
            m.setIdentity();
            if (m_texAnimation->SampleMaterialPos(p, am, l, (unsigned)time)) { m.m[12] = p.x; m.m[13] = p.y; }
            bool moved = std::fabs(m.m[12]) > 1.1920929e-07f || std::fabs(m.m[13]) > 1.1920929e-07f;
            jet::video::TextureLayer &layer = mat.GetTextureLayer(0, l);
            if (moved) layer.SetTransform(m);
            else layer.ClearTransform();
        }
    }
}

// _Update @0x688870
void Model::Update(int dt) {
    if (m_cc && m_cc->IsActive()) {
        m_cc->Update(dt);
        m_interpCur = (short)((int)(m_cc->GetSpeed() * (float)dt) + m_interpCur);
        if (m_interpTotal < m_interpCur) m_interpCur = m_interpTotal;
    }
    RefreshMovementDelta(true);
    if (m_updateMode == 0 && m_animation) AnimateNodes(false);
    if (m_ctc && m_ctc->IsActive()) m_ctc->Update(dt);
}

}
