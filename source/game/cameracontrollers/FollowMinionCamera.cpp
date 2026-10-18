// source/game/cameracontrollers/FollowMinionCamera.cpp
#include "FollowMinionCamera.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../actors/statemachine/ActorState.h"
#include "../actors/statemachine/ActorStateSet.h"
#include "../entities/Minion.h"
#include "core/util.h"
#include "scene/scenemgr.h"
#include "stream/istream.h"
#include <algorithm>
#include <cmath>

jet::String FollowMinionCamera::k_cameraType("FollowMinionCamera");

typedef CameraFollowEntity::FollowCameraConfigStruct Config;

// @0x58fc4c: the delayers take the times of the entity's data; the first running camera data is the look the camera starts with
FollowMinionCamera::FollowMinionCamera(Config *config)
    : m_position(config->position), m_rotation(config->rotation), m_fov(config->fov), m_localPosition(0, 0, 0), m_localRotation(0, 0, 0, 1),
      m_targetZDelayer(0.0f, 0.0f, config->targetZDelayTime),
      m_globalPositionDelayer(math::vec3<float>(0, 0, 0), math::vec3<float>(0, 0, 0), config->globalPositionDelayTime),
      m_localPositionDelayer(math::vec3<float>(0, 0, 0), math::vec3<float>(0, 0, 0), config->localPositionDelayTime),
      m_rotationDelayer(math::quat<float>(0, 0, 0, 1), math::quat<float>(0, 0, 0, 1), config->rotationDelayTime), m_leanDelayer(0.0f, 0.0f, 20),
      m_config(config) {
    if (!config->runningCameraData.empty()) SetCameraData(&config->runningCameraData[0]);
}

// @0x58fe18
void FollowMinionCamera::PostInit() {
    m_position = m_config->position;
    m_rotation = m_config->rotation;
    m_vibrationTimer = 0;
    InitCameraPositionAndRotation();
}

// the focus the camera is placed from and looks at: the entity's target, else the minion
static CameraTarget *FocusOf(const Config *c) {
    if (c->focusTarget) return c->focusTarget;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    return minion;
}

// @0x58fe5c: the camera's place from the focus in the focus's frame (the inverse of what the entity gave), and the delayers at where they are
void FollowMinionCamera::InitCameraPositionAndRotation() {
    CameraTarget *focus = FocusOf(m_config);
    const math::vec3<float> focusPos = focus->GetFocusPosition();
    math::quat<float> focusRot = focus->GetFocusRotation();
    focusRot.x = -focusRot.x;
    focusRot.y = -focusRot.y;
    focusRot.z = -focusRot.z;
    m_localPosition = focusRot * (m_position - focusPos);
    m_localRotation = focusRot * m_config->rotation;
    m_vibrationTimer = 0;
    m_localPositionDelayer.SetCurrentValue(GetPosition() - focusPos);
    m_localPositionDelayer.SetTargetValue(GetPosition() - focusPos);
    m_globalPositionDelayer.SetCurrentValue(focusPos);
    m_globalPositionDelayer.SetTargetValue(focusPos);
    m_rotationDelayer.SetCurrentValue(GetRotation());
    m_rotationDelayer.SetTargetValue(GetRotation());
    m_targetZDelayer.SetCurrentValue(focusPos.z);
    m_targetZDelayer.SetTargetValue(focusPos.z);
}

// @0x590b78: a crash starts the shake, which runs down
void FollowMinionCamera::UpdateTimers(unsigned dt) {
    CameraTarget *target = FocusOf(m_config);
    Minion *minion = dynamic_cast<Minion*>(target);
    if (minion && minion->TakeAfterCrashFlag()) m_vibrationTimer = m_config->vibrationAttenuationTime;
    if (m_vibrationTimer > 0) m_vibrationTimer -= (float)dt;
}

// @0x590038
void FollowMinionCamera::Update(const unsigned dt) {
    CameraTarget *target = FocusOf(m_config);
    Minion *minion = dynamic_cast<Minion*>(target);
    // the camera stays where it is while the minion falls into a pit or is dead
    if (minion && (minion->IsFallingInPit() || minion->GetStateFlag265())) return;
    const bool action = Game::GetInstance()->IsInActionPhase();
    if (action) UpdateTimers(dt);
    const math::vec3<float> targetPos = target->GetTargetPosition();
    const math::quat<float> targetRot = target->GetTargetRotation();
    math::vec3<float> offset = targetRot * m_localPosition;
    math::quat<float> rot = targetRot * m_localRotation;
    float fov = m_config->fov;
    math::vec3<float> vibration(0, 0, 0);
    if (m_config->useMotion) {
        // the camera leans with the minion: turned about the vertical by the lean, and set aside the same way
        const float lean = minion ? minion->GetLean() : 0.0f;
        const float absLean = std::fabs(lean);
        m_leanDelayer.SetDelay((int)((double)absLean * 130.0 + (1.0 - (double)absLean) * 20.0));
        m_leanDelayer.SetTargetValue(lean);
        m_leanDelayer.Update((int)dt);
        const float yaw = m_leanDelayer.GetCurrentValue() * 0.6981317f;
        const math::quat<float> yawRot(math::quat<float>::axisZ, yaw);
        rot = yawRot * rot;
        const math::vec3<float> &front = jet::scene::SceneMgr::GetFrontVector();
        const float sway = std::fabs(m_leanDelayer.GetCurrentValue());
        const math::vec3<float> aside = targetRot * math::vec3<float>((front.x + front.x) * sway, (front.y + front.y) * sway, (front.z + front.z) * sway);
        offset = yawRot * (offset - aside);
        const ActorStateSet *set = minion ? minion->SM_CurrentSet() : 0;
        UpdateRunningCameraDataByStateType(minion ? minion->SM_GetState() : 0, set);
        UpdateRunningEffect((int)dt, offset, rot, fov);
    }
    if (m_vibrationTimer > 0 && action && dt != 0) {
        const float amplitude = std::min(std::max(m_vibrationTimer / m_config->vibrationAttenuationTime, 0.0f), 1.0f) * m_config->vibrationAmplitude;
        const int r1 = jet::core::Rand(-1, 1);
        const int r2 = jet::core::Rand(-1, 1);
        const math::vec3<float> up = GetRotation() * math::vec3<float>(0, 0, 1);
        const math::vec3<float> left = GetRotation() * math::vec3<float>(-1, 0, 0);
        vibration += up * (amplitude * (float)r2);
        vibration += left * (amplitude * (float)r1);
    }
    m_localPositionDelayer.SetTargetValue(vibration + offset);
    m_rotationDelayer.SetTargetValue(rot);
    m_globalPositionDelayer.SetTargetValue(targetPos);
    m_targetZDelayer.SetTargetValue(targetPos.z);
    m_localPositionDelayer.Update((int)dt);
    const math::vec3<float> local = m_localPositionDelayer.GetCurrentValue();
    m_rotationDelayer.Update((int)dt);
    const math::quat<float> smoothRot = m_rotationDelayer.GetCurrentValue();
    m_globalPositionDelayer.Update((int)dt);
    math::vec3<float> global = m_globalPositionDelayer.GetCurrentValue();
    m_targetZDelayer.Update((int)dt);
    global.z = m_targetZDelayer.GetCurrentValue();
    m_rotation = smoothRot;
    m_fov = fov;
    m_position = local + global + m_tutorialOffset;
}

// @0x590554: the camera takes the look of the data; what it had when the transition was not over is where it goes on from
void FollowMinionCamera::SetCameraData(RunningCameraData *d) {
    if (m_cameraData == d) return;
    if (!m_cameraData) {
        m_cameraOffset = d->cameraOffset;
        m_targetOffset = d->targetOffset;
        m_translationOffset = d->translationOffset;
        m_runTime = d->time;
        m_translationScale = d->translationScale;
        m_fov = d->fov;
        m_transitionTime = 0;
        m_roll = d->roll;
    } else {
        if (m_transitionTime > 0) {
            // the part of the previous transition that is left
            float t = (float)m_transitionTime / (float)m_cameraData->transitionTime;
            t *= t;
            m_runTime = m_cameraData->time + (int)((double)t * (double)(m_runTime - m_cameraData->time));
            m_translationOffset = math::lerp(m_cameraData->translationOffset, m_translationOffset, t);
            m_cameraOffset = math::lerp(m_cameraData->cameraOffset, m_cameraOffset, t);
            m_targetOffset = math::lerp(m_cameraData->targetOffset, m_targetOffset, t);
            m_translationScale = m_cameraData->translationScale + t * (m_translationScale - m_cameraData->translationScale);
            m_fov = m_cameraData->fov + t * (m_fov - m_cameraData->fov);
            m_roll = m_cameraData->roll + t * (m_roll - m_cameraData->roll);
        }
        m_transitionTime = d->transitionTime;
    }
    m_cameraData = d;
}

// @0x590720: the look for the state the minion is in: the data of the state itself, else the first of the data for the type of the state,
// else the first
void FollowMinionCamera::UpdateRunningCameraDataByStateType(const ActorState *state, const ActorStateSet *set) {
    if (!state || m_config->runningCameraData.empty()) return;
    RunningCameraData *best = &m_config->runningCameraData[0];
    bool foundByType = false;
    for (RunningCameraData &d : m_config->runningCameraData) {
        if (d.stateType == -1 && !d.stateName.empty() && set) {
            if (const ActorState *s = set->FindStateByName(d.stateName)) {
                d.state = s;
                d.stateType = s->m_type;
            }
        }
        if (!foundByType && d.stateType == state->m_type) {
            best = &d;
            foundByType = true;
        }
        if (d.state == state) {
            best = &d;
            break;
        }
    }
    SetCameraData(best);
}

// @0x590810: the sway round the look of the data (the camera and the point it looks at move together by the translation offset along a
// cosine of the run cycle), the roll, and the blend from the look it had to the one of the data over the transition time
void FollowMinionCamera::UpdateRunningEffect(int dt, math::vec3<float> &pos, math::quat<float> &rot, float &fov) {
    math::vec3<float> translationOffset = m_translationOffset, cameraOffset = m_cameraOffset, targetOffset = m_targetOffset;
    int runTime = m_runTime;
    float translationScale = m_translationScale, roll = m_roll;
    if (m_transitionTime > 0) {
        m_transitionTime = std::max(m_transitionTime - dt, 0);
        const RunningCameraData *d = m_cameraData;
        float t = (float)m_transitionTime / (float)d->transitionTime;
        t *= t;
        runTime = d->time + (int)((double)t * (double)(runTime - d->time));
        translationOffset = math::lerp(d->translationOffset, translationOffset, t);
        cameraOffset = math::lerp(d->cameraOffset, cameraOffset, t);
        targetOffset = math::lerp(d->targetOffset, targetOffset, t);
        translationScale = d->translationScale + t * (translationScale - d->translationScale);
        fov = d->fov + (fov - d->fov) * t;
        roll = d->roll + t * (roll - d->roll);
        if (m_transitionTime == 0) {
            RunningCameraData *done = m_cameraData;
            m_cameraData = 0;
            SetCameraData(done);
        }
    }
    m_runAngle += 6.2831855f / (float)runTime * (float)dt;
    const float halfRoll = roll * 0.5f;
    const math::vec3<float> sway(std::cos(m_runAngle * 0.5f) * translationOffset.x, 0, std::cos(m_runAngle) * translationOffset.z);
    const math::vec3<float> swayWorld = rot * sway;
    const math::vec3<float> camera = pos + rot * cameraOffset + swayWorld;
    const math::vec3<float> look = pos + rot * targetOffset + swayWorld * translationScale;
    math::vec3<float> dir = look - camera;
    dir.normalize();
    const math::vec3<float> up = rot * jet::scene::SceneMgr::GetUpVector();
    math::quat<float> lookRot;
    lookRot.setFromMat3(math::lookAt(dir, up));
    const math::quat<float> rollRot(0, std::sin(halfRoll), 0, std::cos(halfRoll));
    pos = camera;
    rot = lookRot * rollRot;
}

// @0x590bf8
void FollowMinionCamera::Serialize(jet::stream::IStreamW &s) const { s << m_position << m_rotation << m_fov; }

// @0x590c2c
void FollowMinionCamera::Deserialize(const jet::stream::IStream &s) {
    jet::stream::IStream &in = const_cast<jet::stream::IStream&>(s);
    in >> m_position >> m_rotation >> m_fov;
}

// the focus type of the entity's data (what the "camera" condition of the state triggers asks)
int FollowMinionCamera::GetCameraStyle() const { return m_config->cameraFocusType; }
