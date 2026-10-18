// source/game/entities/camera/CameraFollowEntity.cpp
#include "CameraFollowEntity.h"
#include "../Minion.h"
#include "../../GameLevel.h"
#include "../../cameracontrollers/FollowMinionCamera.h"
#include "param.h"
#include "project.h"

jet::String CameraFollowEntity::k_tmplName("CameraFollow");
jet::String CameraFollowEntity::k_RunningCameraConfig_tmplName("RunningCameraConfig");

// @0x55bec4
CameraFollowEntity::CameraFollowEntity(const clara::Template *t) : CameraEntity(t), m_fov(0.7853982f) {}

// @0x55bf04
CameraFollowEntity::~CameraFollowEntity() {
    delete m_cameraConfig;
    delete m_followCamera;
}

// @0x55bf9c
void CameraFollowEntity::Init() {}

// @0x55bfa0: the camera of the data follows the minion (or the focus target the entity names)
void CameraFollowEntity::PostInit() {
    GameEntity::PostInit();
    m_cameraConfig = GetCameraConfig();
    m_followCamera = new FollowMinionCamera(m_cameraConfig);
    m_followCamera->PostInit();
}

// @0x55bfe8
CameraController *CameraFollowEntity::GetCameraController() { return m_followCamera; }

// @0x55c864
void CameraFollowEntity::SetFocusTarget(CameraTarget *t) {
    if (m_cameraConfig) m_cameraConfig->focusTarget = t;
}

// @0x55bff0: the camera is placed from the focus (the target the data names, else the minion): where the entity is, in the focus's frame,
// and the values the data gives -- the times the positions and the rotation take to ease, the motion, the cinematic times, the vibration
// and the camera data of each state of the minion
CameraFollowEntity::FollowCameraConfigStruct *CameraFollowEntity::GetCameraConfig() {
    FollowCameraConfigStruct *c = new FollowCameraConfigStruct;
    if (const clara::Param *p = FindParamByName(jet::String("FocusTarget"))) {
        const clara::Path &focusPath = p->GetAsPath(0);
        if (!focusPath.IsEmpty()) {
            clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByPath(focusPath);
            c->focusTarget = e ? dynamic_cast<CameraTarget*>(e) : 0;
        }
    }
    CameraTarget *focus = c->focusTarget;
    if (!focus)
        if (Minion *minion = GameLevel::GetInstance()->GetMinion()) focus = minion;
    const math::quat<float> focusRot = focus->GetFocusRotation();
    const math::vec3<float> focusPos = focus->GetFocusPosition();
    c->position = focusRot * GetPosition() + focusPos;
    c->rotation = focusRot * GetRotation();
    GetParam(jet::String("Fov"), c->fov);
    GetParam(jet::String("GlobalPositionDelayTime"), c->globalPositionDelayTime);
    GetParam(jet::String("LocalPositionDelayTime"), c->localPositionDelayTime);
    GetParam(jet::String("TargetZDelayer"), c->targetZDelayTime);
    GetParam(jet::String("RotationDelayTime"), c->rotationDelayTime);
    GetParam(jet::String("UseMotion"), c->useMotion);
    GetParam(jet::String("CinematicCameraTransitionInTime"), c->cinematicCameraTransitionInTime);
    GetParam(jet::String("CinematicCameraTransitionOutTime"), c->cinematicCameraTransitionOutTime);
    GetParam(jet::String("cameraFocusType"), c->cameraFocusType);
    GetParam(jet::String("VibrationAmplitud"), c->vibrationAmplitude);
    GetParam(jet::String("VibrationAttenuationTime"), c->vibrationAttenuationTime);
    const float toRadians = 0.017453292f;
    if (const clara::Param *p = FindParamByName(jet::String("RunningCameraData"))) {
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            clara::DataEntity *e = p->GetAsEntity(i);
            RunningCameraData d;
            e->GetParam(jet::String("CameraOffset"), d.cameraOffset);
            e->GetParam(jet::String("TargetOffset"), d.targetOffset);
            e->GetParam(jet::String("TranslationOffset"), d.translationOffset);
            e->GetParam(jet::String("Time"), d.time);
            e->GetParam(jet::String("TranslationScale"), d.translationScale);
            e->GetParam(jet::String("TransitionTime"), d.transitionTime);
            e->GetParam(jet::String("fov"), d.fov);
            d.fov *= toRadians;
            e->GetParam(jet::String("roll"), d.roll);
            d.roll *= toRadians;
            if (const clara::Param *s = e->FindParamByName(jet::String("State"))) d.stateName = s->GetAsState(0).b;
            c->runningCameraData.push_back(d);
        }
    }
    c->fov *= toRadians;
    return c;
}
