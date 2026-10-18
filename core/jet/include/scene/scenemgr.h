// core/jet/include/scene/scenemgr.h
#ifndef CORE_JET_INCLUDE_SCENE_SCENEMGR_H
#define CORE_JET_INCLUDE_SCENE_SCENEMGR_H

#include <vector>
#include <unordered_set>
#include "smath.h"
#include "core/pstring.h"
#include "scene/camera.h"
#include "scene/frustum.h"
#include "scene/occluder.h"
#include "scene/renderable.h"

namespace jet::scene {

// Per-tag renderable lists, culled (distance / screen radius / frustum / occluders) and drawn per tag
// (ctor @0x68e368, Render @0x68f2d4). Lists are emptied after each Render.
class SceneMgr {
public:
    enum { CullFrustum = 1, CullDistance = 2, CullScreenRadius = 4, CullOcclusion = 8 };
    static const unsigned kTagCount = 32;

    SceneMgr();
    virtual ~SceneMgr();

    void Add(Occluder*);
    void Remove(Occluder*);
    void Add(const jet::String &tag, Renderable*);
    virtual void Add(unsigned tag, Renderable*);
    void Add(Renderable *r) { Add(0u, r); }
    void Remove(const jet::String &tag, Renderable*);
    virtual void Remove(unsigned tag, Renderable*);
    void Remove(Renderable *r) { Remove(0u, r); }
    void AddToAllTags(Renderable*);
    void RemoveFromAllTags(Renderable*);
    unsigned GetRenderableCount(unsigned tag) const { return tag < kTagCount ? (unsigned)m_renderables[tag].size() : 0; }
    Renderable *GetRenderable(unsigned tag, unsigned i) { return tag < kTagCount ? m_renderables[tag][i] : 0; }
    unsigned GetOccluderCount() const { return (unsigned)m_occluders.size(); }
    Occluder *GetOccluder(unsigned i) { return m_occluders[i]; }

    void SetCamera(Camera *c) { m_camera = c; }
    Camera *GetCamera() const { return m_camera; }
    void SetCullCamera(Camera *c) { m_cullCamera = c; }
    virtual Camera *GetCullCamera() const { return m_cullCamera ? m_cullCamera : m_camera; }

    void SetCullMethods(unsigned m) { m_cullMethods = m; }
    unsigned GetCullMethods() const { return m_cullMethods; }
    void SetDistanceCulling(float radius, float distance) { m_distCullRadius = radius; m_distCullDistance = distance; m_cullMethods |= CullDistance; }
    float GetDistanceCullingRadius() const { return m_distCullRadius; }
    float GetDistanceCullingDistance() const { return m_distCullDistance; }
    void SetScreenRadiusCulling(float r) { m_screenCullRadius = r; m_cullMethods |= CullScreenRadius; }
    float GetScreenRadiusCullingRadius() const { return m_screenCullRadius; }
    void SetOcclusionCulling(bool on) { m_cullMethods = on ? (m_cullMethods | CullOcclusion) : (m_cullMethods & ~(unsigned)CullOcclusion); }
    bool IsOcclusionCullingEnabled() const { return (m_cullMethods & 0xF) >> 3; }

    static SceneMgr *GetSceneMgr() { return s_sceneMgr; }
    // Model-space axes of the camera frame: front (0,-1,0), up (0,0,1), right (-1,0,0)
    static const math::vec3<float> &GetFrontVector() { static const math::vec3<float> v(0, -1, 0); return v; }
    static const math::vec3<float> &GetUpVector() { static const math::vec3<float> v(0, 0, 1); return v; }
    static const math::vec3<float> &GetRightVector() { static const math::vec3<float> v(-1, 0, 0); return v; }

    virtual void Render(unsigned tag);
    void Render(const jet::String &tag);
    void Render() { Render(0u); }
    virtual void ClearRenderables(unsigned tag) { if (tag < kTagCount) m_renderables[tag].clear(); }
    void ClearRenderables(const jet::String &tag);
    void ClearRenderablesFromAllTags();

    CullStatusDef::type CullSphere(const math::vec3<float>&, float) const;

    void ShowRenderable(Renderable *r) { m_hidden.erase(r); }
    void HideRenderable(Renderable *r) { m_hidden.insert(r); }
    bool IsRendering() const { return m_isRendering; }

protected:
    void SetupOccluders();
    void ClearOccluders() { m_occluders.clear(); }
    bool Cull(Renderable*) const;
    bool CullDistanceTest(float radius, float distance) const;
    bool CullScreenRadiusTest(float radius, float distance, float *outScreenRadius) const;
    bool CullOcclusionTest(const math::vec3<float>&, float) const;
    void SortOccludersByDistance();

private:
    bool m_isRendering;
    std::vector<Renderable*> m_renderables[kTagCount];
    std::vector<Occluder*> m_occluders;
    Camera *m_camera;
    Camera *m_cullCamera;
    float m_screenHeight;
    float m_tanHalfFov;
    unsigned m_cullMethods;
    float m_distCullDistance;
    float m_distCullRadius;
    float m_screenCullRadius;
    std::unordered_set<Renderable*> m_hidden;
    static SceneMgr *s_sceneMgr;
};
}

#endif
