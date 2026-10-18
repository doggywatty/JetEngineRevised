// source/game/cameracontrollers/FollowMinionCamera.h
#ifndef SOURCE_GAME_CAMERACONTROLLERS_FOLLOWMINIONCAMERA_H
#define SOURCE_GAME_CAMERACONTROLLERS_FOLLOWMINIONCAMERA_H

#include "camera/cameracontroller.h"
#include "core/delayer.h"
#include "../entities/camera/CameraFollowEntity.h"

class ActorState;
class ActorStateSet;

// The camera of the run: it is placed round the minion (the point it looks at and where it stands are eased separately, the height
// slower), leans with the way the minion turns, sways with the time of the run cycle and takes the look of the state the minion is in
// (the running camera data of the entity, blended over its transition time), and shakes after a crash. 0x15C bytes in the .so.
class FollowMinionCamera : public CameraController {
public:
    explicit FollowMinionCamera(CameraFollowEntity::FollowCameraConfigStruct*);
    void PostInit();
    void InitCameraPositionAndRotation();
    virtual math::vec3<float> GetPosition() const { return m_position; }
    virtual math::quat<float> GetRotation() const { return m_rotation; }
    virtual float GetFov() const { return m_fov; }
    virtual const jet::String &GetType() const { return k_cameraType; }
    virtual void Update(const unsigned dt);
    virtual void Serialize(jet::stream::IStreamW&) const;
    virtual void Deserialize(const jet::stream::IStream&);
    virtual void SetShakeTime(float) {}
    // the offset a tutorial adds to the camera's place
    void SetTutorialOffset(const math::vec3<float> &o) { m_tutorialOffset = o; }
    // the focus type of the entity's data (what the "camera" condition of the state triggers asks)
    int GetCameraStyle() const;
    CameraFollowEntity::FollowCameraConfigStruct *GetConfig() const { return m_config; }
    static bool IsFollowMinionCameraCtrl(CameraController *c) { return c && c->GetType().Equals(k_cameraType); }
    void SetCameraData(RunningCameraData*);
    static jet::String k_cameraType;
private:
    void UpdateTimers(unsigned dt);
    void UpdateRunningCameraDataByStateType(const ActorState*, const ActorStateSet*);
    void UpdateRunningEffect(int dt, math::vec3<float> &pos, math::quat<float> &rot, float &fov);
    math::vec3<float> m_tutorialOffset = math::vec3<float>(0, 0, 0);  // 0x4
    math::vec3<float> m_position;                                     // 0x10
    math::quat<float> m_rotation;                                     // 0x1C
    float m_fov;                                                      // 0x2C
    math::vec3<float> m_localPosition;                                // 0x30: where the camera is from the target, in the target's frame
    math::quat<float> m_localRotation;                                // 0x3C
    jet::core::Delayer<float, jet::core::LinearStep> m_targetZDelayer;                // 0x4C
    jet::core::Delayer<math::vec3<float>, jet::core::LinearStep> m_globalPositionDelayer;  // 0x64: the target's place
    jet::core::Delayer<math::vec3<float>, jet::core::LinearStep> m_localPositionDelayer;   // 0x94: the camera's place from it
    jet::core::Delayer<math::quat<float>, jet::core::LinearStep> m_rotationDelayer;        // 0xC4
    jet::core::Delayer<float, jet::core::LinearStep> m_leanDelayer;   // 0x100: how much the minion leans in a turn
    CameraFollowEntity::FollowCameraConfigStruct *m_config;           // 0x118
    math::vec3<float> m_cameraOffset = math::vec3<float>(0, 0, 0);    // 0x11C
    math::vec3<float> m_targetOffset = math::vec3<float>(0, 0, 0);    // 0x128
    math::vec3<float> m_translationOffset = math::vec3<float>(0, 0, 0);  // 0x134
    int m_runTime = 0;                                                // 0x140
    float m_translationScale = 0;                                     // 0x144
    float m_roll = 0;                                                 // 0x148
    float m_runAngle = 0;                                             // 0x14C: where the sway is in its cycle
    int m_transitionTime = 0;                                         // 0x150
    RunningCameraData *m_cameraData = 0;                              // 0x154
    float m_vibrationTimer = 0;                                       // 0x158
};

#endif
