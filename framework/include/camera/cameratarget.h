// framework/include/camera/cameratarget.h
#ifndef FRAMEWORK_INCLUDE_CAMERA_CAMERATARGET_H
#define FRAMEWORK_INCLUDE_CAMERA_CAMERATARGET_H

class CameraTarget {  // sizeof 0x4
public:
    virtual math::vec3<float> GetTargetPosition() = 0;
    virtual math::quat<float> GetTargetRotation() = 0;
    virtual math::vec3<float> GetFocusPosition() = 0;
    virtual math::quat<float> GetFocusRotation() = 0;
    virtual ~CameraTarget();
};

#endif
