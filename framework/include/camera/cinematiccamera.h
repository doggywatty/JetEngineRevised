// framework/include/camera/cinematiccamera.h
#ifndef FRAMEWORK_INCLUDE_CAMERA_CINEMATICCAMERA_H
#define FRAMEWORK_INCLUDE_CAMERA_CINEMATICCAMERA_H

#include "camera/cameracontroller.h"
#include "scene/camera.h"
#include "scene/model.h"

// Follows a camera animated inside a model, under a local transform
class CinematicCamera : public CameraController {  // sizeof 0x24
public:
    CinematicCamera() : m_animatedCamera(0), m_localPosition(0, 0, 0), m_localRotation(0, 0, 0, 1) {}
    void SetAnimatedCamera(const jet::scene::Model *m) { m_animatedCamera = m && m->GetCameraCount() ? &m->GetCamera(0) : 0; }
    virtual math::vec3<float> GetPosition() const;
    virtual math::quat<float> GetRotation() const;
    virtual float GetFov() const { return m_animatedCamera ? m_animatedCamera->GetFov() : 0.7853982f; }
    virtual const jet::String &GetType() const { return k_cameraType; }
    virtual void Update(const unsigned) {}
    virtual void Serialize(jet::stream::IStreamW&) const {}
    virtual void Deserialize(const jet::stream::IStream&) {}
    void SetLocalTransform(const math::vec3<float> &p, const math::quat<float> &r) { m_localPosition = p; m_localRotation = r; }
    math::mat4<float> GetLocalTransform() const;
    static jet::String k_cameraType;
private:
    jet::scene::Camera *m_animatedCamera;  // 0x4
    math::vec3<float> m_localPosition;  // 0x8
    math::quat<float> m_localRotation;  // 0x14
};

#endif
