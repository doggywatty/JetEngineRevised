// source/game/entities/camera/CameraTarget.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERATARGET_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERATARGET_H

#include "smath.h"

// What a follow camera looks at (the minion): where it should be pointed (the target) and where the camera is placed from (the focus)
class CameraTarget {
public:
    virtual math::vec3<float> GetTargetPosition() const = 0;
    virtual math::quat<float> GetTargetRotation() const = 0;
    virtual math::vec3<float> GetFocusPosition() const = 0;
    virtual math::quat<float> GetFocusRotation() const = 0;
protected:
    ~CameraTarget() {}
};

#endif
