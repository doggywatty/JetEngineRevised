// core/jet/include/scene/camera.h
#ifndef CORE_JET_INCLUDE_SCENE_CAMERA_H
#define CORE_JET_INCLUDE_SCENE_CAMERA_H

#include <cmath>
#include "smath.h"
#include "jenum.h"
#include "core/pstring.h"
#include "scene/frustum.h"
#include "scene/modelbase.h"
#include "video/display.h"

namespace jet::scene {
// A view: position and rotation, a projection (perspective or orthogonal, optionally off centre)
// and the frustums the scene culls with. The matrices are refreshed on demand.
class Camera {  // sizeof 0x3C0
public:
    Camera(const math::vec3<float>&, const math::quat<float>&);
    Camera(const Camera&);
    Camera &operator=(const Camera&);
    ~Camera();
    void Load(const jet::scene::ModelBase::CameraData&);
    math::line3<float> GetRayFromScreenCoordinates(const math::vec2<float>&);
    bool Project(math::vec3<float>&, const math::vec3<float>&);
    bool Project(math::vec2<float>&, const math::vec3<float>&);
    bool Unproject(math::vec3<float>&, const math::vec3<float>&);
    void SetViewport(const math::rect<unsigned int>&);
    math::rect<unsigned int> GetViewport();
    math::vec2<unsigned int> GetViewportSize();
    void SetPosition(const math::vec3<float>&);
    const math::vec3<float> &GetPosition() const { return m_position; }
    void SetRotation(const math::quat<float>&);
    const math::quat<float> &GetRotation() const { return m_rotation; }
    void SetTransform(const math::vec3<float>&, const math::quat<float>&);
    const math::vec3<float> &GetFrontVector() { Refresh(); return m_frontVector; }
    const math::vec3<float> &GetUpVector() { Refresh(); return m_upVector; }
    const math::vec3<float> &GetRightVector() { Refresh(); return m_rightVector; }
    const math::mat4<float> &GetViewProjectionTransform() { Refresh(); return m_viewProjectionTransform; }
    const math::mat4<float> &GetProjectionTransform() { Refresh(); return m_projectionTransform; }
    const math::mat4<float> &GetProjectionTransformOriented() { Refresh(); return m_projectionTransformOriented; }
    const math::mat4<float> &GetViewTransform() { Refresh(); return m_viewTransform; }
    bool IsOrthogonal() const { return m_isOrthogonal; }
    void SetOrthogonal(bool o) { m_isOrthogonal = o; m_projDataChanged = true; }
    float GetOrthogonalZoom() const { return m_orthoZoom; }
    void SetOrthogonalZoom(float z) { m_orthoZoom = z; m_projDataChanged = true; }
    void SetAspectRatio(float a) { m_aspectRatio = a; m_projDataChanged = true; }
    float GetAspectRatio();
    float GetFov() const { return m_fovy; }
    void SetFov(float f) { if (m_fovy != f) { m_fovy = f; m_projDataChanged = true; } }
    float GetHorizontalFov();
    void SetHorizontalFov(float);
    float GetNearDistance() const { return m_nearDist; }
    void SetNearDistance(float n) { if (m_nearDist != n) { m_nearDist = n; m_projDataChanged = true; } }
    float GetFarDistance() const { return m_farDist; }
    void SetFarDistance(float f) { if (m_farDist != f) { m_farDist = f; m_projDataChanged = true; } }
    void SetProjectionOffCenter(const math::rect<float> &r) { m_offCenter = r; m_projDataChanged = true; }
    const math::rect<float> &GetProjectionOffCenter() const { return m_offCenter; }
    const jet::scene::Frustum &GetFrustum() { Refresh(); return m_frustum; }
    const jet::scene::Frustum &GetUntransformedFrustum() { Refresh(); return m_untransformedFrustum; }
    void ComputeFrustumPlanes(int, int, int, int, math::plane<float>*);
    const jet::String &GetInstanceName() const { return m_instanceName; }
protected:
    bool HasViewportChanged() const;
    void Refresh() { if (m_projDataChanged || m_worldDataChanged || HasViewportChanged()) RefreshTransforms(); }
    void RefreshTransforms();
    math::mat4<float> ComputePerspectiveMatrix(float fov, float aspect, float n, float f) const;
    math::mat4<float> ComputeOrthographicMatrix(float w, float h, float n, float f) const;
    math::rect<unsigned int> m_viewport;  // 0x0: inclusive corners, empty for the whole render target
    math::rect<unsigned int> m_cachedViewport;  // 0x10: the render target the matrices were built for
    math::vec3<float> m_position;  // 0x20
    math::quat<float> m_rotation;  // 0x2C
    bool m_worldDataChanged;  // 0x3C
    math::vec3<float> m_frontVector;  // 0x40
    math::vec3<float> m_upVector;  // 0x4C
    math::vec3<float> m_rightVector;  // 0x58
    float m_aspectRatio;  // 0x64: 0 for the viewport's
    math::rect<float> m_offCenter;  // 0x68
    bool m_isOrthogonal;  // 0x78
    math::mat4<float> m_viewTransform;  // 0x7C
    math::mat4<float> m_projectionTransform;  // 0xBC
    math::mat4<float> m_projectionTransformOriented;  // 0xFC
    math::mat4<float> m_viewProjectionTransform;  // 0x13C
    bool m_projDataChanged;  // 0x17C
    jet::String m_instanceName;  // 0x180
    float m_fovy;  // 0x184
    float m_nearDist;  // 0x188
    float m_farDist;  // 0x18C
    float m_orthoZoom;  // 0x190
    unsigned m_cachedOrientation;  // 0x194
    jet::scene::Frustum m_frustum;  // 0x198
    jet::scene::Frustum m_untransformedFrustum;  // 0x2AC
};
}

#endif
