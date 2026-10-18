// framework/include/camera/directcamera.h
#ifndef FRAMEWORK_INCLUDE_CAMERA_DIRECTCAMERA_H
#define FRAMEWORK_INCLUDE_CAMERA_DIRECTCAMERA_H

#include "camera/cameracontroller.h"

// A camera sitting where it is told to
class DirectCamera : public CameraController {  // sizeof 0x24
public:
    DirectCamera(const math::vec3<float> &p, const math::quat<float> &r, float fov) : m_position(p), m_rotation(r), m_fov(fov) {}
    virtual math::vec3<float> GetPosition() const { return m_position; }
    virtual math::quat<float> GetRotation() const { return m_rotation; }
    virtual float GetFov() const { return m_fov; }
    virtual const jet::String &GetType() const { return k_cameraType; }
    virtual void Update(const unsigned) {}
    virtual void Serialize(jet::stream::IStreamW&) const;
    virtual void Deserialize(const jet::stream::IStream&);
    void SetPosition(const math::vec3<float> &p) { m_position = p; }
    void SetRotation(const math::quat<float> &r) { m_rotation = r; }
    void SetFov(float f) { m_fov = f; }
    static jet::String k_cameraType;
private:
    math::vec3<float> m_position;  // 0x4
    math::quat<float> m_rotation;  // 0x10
    float m_fov;  // 0x20
};

#endif
