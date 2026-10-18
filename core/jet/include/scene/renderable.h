// core/jet/include/scene/renderable.h
#ifndef CORE_JET_INCLUDE_SCENE_RENDERABLE_H
#define CORE_JET_INCLUDE_SCENE_RENDERABLE_H

#include <boost/noncopyable.hpp>
#include "smath.h"
#include "scene/frustum.h"

namespace jet::scene {
// Cullable thing in a SceneMgr tag list (ctor @0x68e210).
class Renderable : private boost::noncopyable {
public:
    Renderable();
    virtual ~Renderable() {}

    void GetBoundingSphere(math::vec3<float> &c, float &r) const { c = m_center; r = m_radius; }
    const math::vec3<float> &GetBoundingSphereCenter() const { return m_center; }
    float GetBoundingSphereRadius() const { return m_radius; }
    bool HasBoundingBox() const { return m_hasBoundingBox; }
    const math::aabb<float> &GetBoundingBox() const { return m_boundingBox; }

    bool GetDistanceCulling() const { return m_useDistanceCulling; }
    void SetDistanceCulling(bool b) { m_useDistanceCulling = b; }
    bool GetFrustumCulling() const { return m_useFrustumCulling; }
    void SetFrustumCulling(bool b) { m_useFrustumCulling = b; }
    bool GetScreenRadiusCulling() const { return m_useScreenRadiusCulling; }
    void SetScreenRadiusCulling(bool b) { m_useScreenRadiusCulling = b; }

    safe_enum<CullStatusDef, CullStatusDef::type> GetCullStatus() const { return m_cullStatus; }
    void SetCullStatus(safe_enum<CullStatusDef, CullStatusDef::type> s) { m_cullStatus = s; }
    bool IsCulled() const { return m_cullStatus == CullStatusDef::Outside; }
    float GetDistanceToCamera() const { return m_distanceToCamera; }
    void SetDistanceToCamera(float d) { m_distanceToCamera = d; }
    float GetScreenRadius() const { return m_screenRadius; }
    void SetScreenRadius(float r) { m_screenRadius = r; }

    // Bit per render tag in which this has something to draw (0xFFFFFFFF when unknown).
    virtual unsigned GetRenderTagMask() { return 0xFFFFFFFFu; }
    virtual bool PreCull() = 0;
    virtual void PreRender() = 0;
    virtual void Render() = 0;

protected:
    math::vec3<float> m_center;
    float m_radius;
    float m_distanceToCamera;
    float m_screenRadius;
    safe_enum<CullStatusDef, CullStatusDef::type> m_cullStatus;
    math::aabb<float> m_boundingBox;
    bool m_useDistanceCulling;
    bool m_useFrustumCulling;
    bool m_useScreenRadiusCulling;
    bool m_hasBoundingBox;
};
}

#endif
