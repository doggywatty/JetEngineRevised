// source/game/entities/camera/CameraFollowEntity.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERAFOLLOWENTITY_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERAFOLLOWENTITY_H

#include <vector>
#include "CameraEntity.h"
#include "CameraTarget.h"

class ActorState;
class FollowMinionCamera;

// How the camera looks while the minion is in a state ("RunningCameraData" of the camera entity, 0x44 bytes in the .so): it eases to these
// values over the transition time, and sways round them over the time of a run cycle
struct RunningCameraData {
    jet::String stateName;                      // 0x0: the state it is for
    int stateType = -1;                         // 0x4: its type, found from the name when the minion first is in the state set
    int transitionTime = 0;                     // 0x8
    math::vec3<float> cameraOffset;             // 0xC: from the point it looks at, along the target's rotation
    math::vec3<float> targetOffset;             // 0x18: the point it looks at, from the target
    math::vec3<float> translationOffset;        // 0x24: how far the sway goes
    int time = 0;                               // 0x30: the time of a sway cycle
    float translationScale = 0;                 // 0x34
    float fov = 0;                              // 0x38 (radians)
    float roll = 0;                             // 0x3C (radians)
    const ActorState *state = 0;                // 0x40
};

// A camera that follows the minion (the "CameraFollow" template): the data of how the camera is placed round the minion and how it eases
class CameraFollowEntity : public CameraEntity {  // sizeof 0x9C
public:
    // what the entity gives the camera it makes (0x58 bytes in the .so)
    struct FollowCameraConfigStruct {
        math::vec3<float> position;                  // 0x0: where the camera is placed from the focus
        math::quat<float> rotation;                  // 0xC
        float fov = 0;                               // 0x1C (radians)
        CameraTarget *focusTarget = 0;               // 0x20
        int globalPositionDelayTime = 0;             // 0x24
        int localPositionDelayTime = 0;              // 0x28
        int rotationDelayTime = 0;                   // 0x2C
        int targetZDelayTime = 0;                    // 0x30
        bool useMotion = false;                      // 0x34
        int cinematicCameraTransitionInTime = 0;     // 0x38
        int cinematicCameraTransitionOutTime = 0;    // 0x3C
        float vibrationAmplitude = 0;                // 0x40
        float vibrationAttenuationTime = 0;          // 0x44
        int cameraFocusType = 0;                     // 0x48
        std::vector<RunningCameraData> runningCameraData;  // 0x4C
    };
    explicit CameraFollowEntity(const clara::Template*);
    virtual ~CameraFollowEntity();
    virtual void Init();
    virtual void PostInit();
    FollowCameraConfigStruct *GetCameraConfig();
    virtual CameraController *GetCameraController();
    void SetFocusTarget(CameraTarget*);
    static jet::String k_tmplName;
    static jet::String k_RunningCameraConfig_tmplName;
private:
    float m_fov;                              // 0x8C
    FollowMinionCamera *m_followCamera = 0;   // 0x90
    FollowCameraConfigStruct *m_cameraConfig = 0;  // 0x94
};

#endif
