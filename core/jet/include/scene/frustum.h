// core/jet/include/scene/frustum.h
#ifndef CORE_JET_INCLUDE_SCENE_FRUSTUM_H
#define CORE_JET_INCLUDE_SCENE_FRUSTUM_H

#include "smath.h"
#include "jenum.h"

namespace jet::scene {

// The planes' normals point out of the frustum; the cull functions return Inside (0), Outside (1)
// or Intersect (2)
struct PlanesDef { enum type { Far, Near, Left, Right, Bottom, Top }; };
struct CullStatusDef { enum type { Inside, Outside, Intersect }; };

struct Frustum {  // sizeof 0x114
    Frustum(const math::mat4<float>&);
    Frustum(const jet::scene::Frustum&);
    Frustum();
    void SetFrom(const math::mat4<float>&, const math::mat4<float>&, bool, const math::vec3<float>&, const math::vec3<float>&, float, float);
    void SetFrom(const math::mat4<float>&);
    const math::plane<float> &GetPlane(safe_enum<PlanesDef,PlanesDef::type>) const;
    const math::plane<float> *GetPlanes() const;
    math::vec3<float> GetFarLeftUp() const;
    math::vec3<float> GetFarLeftDown() const;
    math::vec3<float> GetFarRightUp() const;
    math::vec3<float> GetFarRightDown() const;
    math::vec3<float> GetNearLeftUp() const;
    math::vec3<float> GetNearLeftDown() const;
    math::vec3<float> GetNearRightUp() const;
    math::vec3<float> GetNearRightDown() const;
    void SetCameraPosition(const math::vec3<float>&);
    const math::vec3<float> &GetCameraPosition() const { return m_cameraPosition; }
    safe_enum<CullStatusDef,CullStatusDef::type> CullSphere(const math::vec3<float>&, float) const;
    safe_enum<CullStatusDef,CullStatusDef::type> CullSphereAround(const math::vec3<float>&, float) const;
    safe_enum<CullStatusDef,CullStatusDef::type> CullSphereAroundFast(const math::vec3<float>&, float) const;
    safe_enum<CullStatusDef,CullStatusDef::type> CullAABB(const math::aabb<float>&) const;
    Frustum &operator=(const Frustum&);
protected:
    void ComputePlanes() const;
    void ComputePlanesFromPoints() const;
    void ComputePoints() const;
    math::mat4<float> m_viewProjMatrix;  // 0x0
    math::vec3<float> m_cameraPosition;  // 0x40
    mutable bool m_dirtyPlanes;  // 0x4C
    mutable math::plane<float> m_planes[6];  // 0x50
    mutable bool m_dirtyPoints;  // 0xB0
    mutable math::vec3<float> m_farLeftUp;  // 0xB4
    mutable math::vec3<float> m_farLeftDown;  // 0xC0
    mutable math::vec3<float> m_farRightUp;  // 0xCC
    mutable math::vec3<float> m_farRightDown;  // 0xD8
    mutable math::vec3<float> m_nearLeftUp;  // 0xE4
    mutable math::vec3<float> m_nearLeftDown;  // 0xF0
    mutable math::vec3<float> m_nearRightUp;  // 0xFC
    mutable math::vec3<float> m_nearRightDown;  // 0x108
};
}

#endif
