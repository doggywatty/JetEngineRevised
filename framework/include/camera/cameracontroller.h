// framework/include/camera/cameracontroller.h
#ifndef FRAMEWORK_INCLUDE_CAMERA_CAMERACONTROLLER_H
#define FRAMEWORK_INCLUDE_CAMERA_CAMERACONTROLLER_H

#include "smath.h"
#include "core/pstring.h"
#include "stream/istream.h"

// Something the camera manager can follow: a position, a rotation and a field of view (radians)
class CameraController {  // sizeof 0x4
public:
    virtual math::vec3<float> GetPosition() const = 0;
    virtual math::quat<float> GetRotation() const = 0;
    virtual float GetFov() const = 0;
    virtual const jet::String &GetType() const = 0;
    virtual void Update(const unsigned) = 0;
    virtual void Serialize(jet::stream::IStreamW&) const = 0;
    virtual void Deserialize(const jet::stream::IStream&) = 0;
    virtual void SetShakeTime(float) {}
    virtual ~CameraController() {}
};

#endif
