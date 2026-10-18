// source/game/entities/Runner.cpp
#include "Runner.h"
#include <cmath>
#include "Minion.h"
#include "LargeMinionGameplay.h"
#include "Pacesetter.h"
#include "Deco3d.h"
#include "Collision3d.h"
#include "../dynamicobjects/EffectControl.h"
#include "../GameLevel.h"
#include "../Statistics.h"
#include "../gamemanagers/BackgroundMgr.h"
#include "../gamemanagers/GameSoundMgr.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "scene/scenemgr.h"
#include "project.h"

jet::String Runner::k_tmplName("Runner");
const jet::String Runner::k_speedPgtParamName("SpeedPgt");
const jet::String Runner::k_speedTimeParamName("SpeedTime");
const jet::String Runner::k_startTimeParamName("StartTime");
const jet::String Runner::k_endDistanceParamName("EndDistance");
const jet::String Runner::k_despicablePointsParamName("despicablePoints");
const jet::String Runner::k_EventOnMinionHitParamName("EventOnMinionHit");
const float Runner::k_distanceToTriggerNPCVoiceSq = 400.0f;  // @0x521488
const jet::String Runner::k_NPCVoiceLabel("ev_minion_npc_idle");
const jet::String Runner::k_NPCSfxRunningLabel("ev_minion_runs_along");

// the distance (squared) at which the minion and the runner touch in the lane
static const float k_touchDistanceSq = 0.49f;  // @0x520a58 (literal of Runner::Update)
// the scale of the speed factor and the start time from the data
static const float k_speedPgtScale = 0.01f;    // @0x520934 (literal of Runner::PostInit)
static const float k_startTimeScale = 0.001f;  // @0x520938 (literal of Runner::PostInit)
// the speed of the minion's pacesetter is at this distance squared for a stop by the collision of a side or behind runner
static const float k_sideDotLimit = 0.5f;      // @0x520fb0 (immediate of Runner::CollidesWith)
static const int k_stoppingTime = 1000;        // @0x520e7c (SetRunnerState's stop time, ms)
static const int k_evilEyesEngineEvent = 0x29; // @0x5213f8 (the engine event of an obstacle the runner does not answer)

// @0x520680
Runner::Runner(const clara::Template *t)
    : Actor(t), PacesetterFollower(), ObstacleTarget(), m_runnerState(RunnerState::Waiting), m_startTime(0), m_distanceToStopSq(0), m_spawnLane((LaneIndex)0),
      m_speedPgt(0), m_speedTime(0), m_minionEngineEventOnHit(0), m_largeMinionRunner(false), m_triggerNPCVoice(false), m_despicablePoints(0), m_pacesetter(0) {}

// @0x5206e0
Runner::~Runner() {
    if (m_runnerState != RunnerState::Stopped) SetRunnerState(RunnerState::Stopped);
    if (m_pacesetter) delete m_pacesetter;
}

// @0x520840
void Runner::PostInit() {
    GetParam(k_despicablePointsParamName, m_despicablePoints);
    if (m_flags & F_LIBRARY) return;
    Actor::PostInit();
    InitPacesetter();
    GetParam(k_speedPgtParamName, m_speedPgt);
    m_speedPgt *= k_speedPgtScale;
    GetParam(k_speedTimeParamName, m_speedTime);
    GetParam(k_startTimeParamName, m_startTime);
    m_startTime *= k_startTimeScale;
    GetParam(k_endDistanceParamName, m_distanceToStopSq);
    m_distanceToStopSq *= m_distanceToStopSq;
    GetParam(k_EventOnMinionHitParamName, m_minionEngineEventOnHit);
    m_triggerNPCVoice = true;
}

// @0x5209e8: the minion meets the runner in its lane (the collision of the minion's banana position), then the runner's state takes its turn
void Runner::Update() {
    Actor::Update();
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion->GetPacesetter()) return;
    if (m_runnerState != RunnerState::Stopping && m_runnerState != RunnerState::Stopped) {
        const math::vec3<float> d = minion->GetBananaPosition() - GetPosition();
        if (d.getLengthSq() < k_touchDistanceSq) {
            CollisionPair pair;
            CollidesWith(minion->GetCollision(), pair);
        }
        UpdateObstacleEffectTouch(minion);
    }
    switch (m_runnerState) {
    case RunnerState::Waiting: {
        // it starts when the minion is in reach of it, on its plane and on the other side of the turn
        const float reach = minion->GetPacesetter()->GetTargetSpeed() * m_startTime;
        const float reachSq = reach * reach;
        const math::vec3<float> minionPos = minion->GetPosition();
        const math::quat<float> minionRot = minion->GetRotation();
        const float dA = GetDistanceSqToPlane(minionPos, minionRot);
        const math::quat<float> turned = math::quat<float>(math::quat<float>::axisZ, 3.14159265f) * minionRot;
        const float dB = GetDistanceSqToPlane(minionPos, turned);
        if (dB <= reachSq && dA <= reachSq) SetRunnerState(RunnerState::Running);
        return;
    }
    case RunnerState::Running: {
        const math::vec3<float> minionPos = minion->GetPosition();
        const math::quat<float> minionRot = minion->GetRotation();
        const math::vec3<float> front = minionRot * jet::scene::SceneMgr::GetFrontVector();
        math::vec3<float> dir = GetPosition() - minionPos;
        dir.normalize();
        // a runner the minion has passed (behind it) stops when the minion is far from its plane
        const float distSq = GetDistanceSqToPlane(minionPos, minionRot);
        if (math::dot(front, dir) <= 0 && distSq >= m_distanceToStopSq) SetRunnerState(RunnerState::Stopped);
        else if (m_largeMinionRunner) UpdateLargeMinionGameplay(distSq);
        m_pacesetter->Update();
        return;
    }
    case RunnerState::Stopping:
        // the pacesetter has slowed to a stop: the runner is done
        if (!m_pacesetter || m_pacesetter->GetSpeed() == 0) SetRunnerState(RunnerState::Stopped);
        return;
    case RunnerState::Stopped:
        // the runner stays while its pacesetter's template is there; without it, the runner is deleted with the level
        if (GetPacesetter()) return;
        GameLevel::GetInstance()->DeleteEntityLater(this);
        return;
    }
}

// @0x520958: a runner of a large minion wave meets the minion when the minion's balance is on the runner's side and the runner is within the
// hit distance of the mode. The hit goes to the mode, and then the minion collides with the runner.
void Runner::UpdateLargeMinionGameplay(float distSq) {
    LargeMinionGameplay *mode = static_cast<LargeMinionGameplay*>(GameLevel::GetInstance()->GetLargeMinionGameplay());
    if (mode->GetBalanceState() != (int)m_spawnLane || distSq > mode->GetRunnerDistanceToHitSq()) return;
    CollisionPair pair;
    mode->OnRunnerHit(m_pacesetter->GetCurrentLane());
    CollidesWith(GameLevel::GetInstance()->GetMinion()->GetCollision(), pair);
}

// @0x520d54 (second touch test of Update, reached when the minion's obstacle effect in slot 0 (+0x46C) has a model (+4)): the minion's position
// is tested against the runner the same way, and the contact is the same CollidesWith
void Runner::UpdateObstacleEffectTouch(Minion *minion) {
    EffectControl *effect = minion->GetObstacleEffectSlot(0);
    if (!effect || !effect->model) return;
    const math::vec3<float> d = minion->GetPosition() - GetPosition();
    if (d.getLengthSq() < k_touchDistanceSq) {
        CollisionPair pair;
        CollidesWith(minion->GetCollision(), pair);
    }
}

// @0x518e14: a running runner is always in the scene
bool Runner::IsOutOfScene() const {
    if (m_runnerState == RunnerState::Running) return false;
    return Object::IsOutOfScene();
}

// @0x518d2c: the runner is put along its pacesetter, not by the level
void Runner::GoTo(const math::vec3<float>&, const math::quat<float>&) {}

// @0x518d38
void Runner::SetSpawnLane(int lane) { m_spawnLane = (LaneIndex)lane; }

// @0x520df0: the state, and what it starts: the running runner starts its pacesetter (and its sound), the stopping one slows it to a stop
void Runner::SetRunnerState(int state) {
    m_runnerState = state;
    if (state == RunnerState::Stopping) {
        m_pacesetter->SetSpeed(0.0f, k_stoppingTime);
    } else if (state == RunnerState::Stopped) {
        StopPacesetter();
    } else if (state == RunnerState::Running) {
        SM_OnEngineEvent(1);
        StartPacesetter();
        GameSoundMgr::GetInstance()->Play(k_NPCSfxRunningLabel, 0.0f, GetPosition(), 0);
    }
}

// @0x520eac: the minion's collision touches the runner. Beside or behind the minion the runner is hidden and stops; ahead of it, the minion
// hits it (engine event 6) and the runner stops. The minion gets the points of the runner and its engine event.
void Runner::CollidesWith(Collision3d *c, const CollisionPair &pair) {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (c != minion->GetCollision()) return;
    if (m_runnerState == RunnerState::Stopping || m_runnerState == RunnerState::Stopped) return;
    math::vec3<float> dir = GetPosition() - minion->GetPosition();
    dir.normalize();
    const math::quat<float> rot = minion->GetRotation();
    const math::vec3<float> front = rot * jet::scene::SceneMgr::GetFrontVector();
    const math::vec3<float> right = rot * jet::scene::SceneMgr::GetRightVector();
    const bool sideOrBehind = std::fabs(math::dot(dir, right)) >= k_sideDotLimit || math::dot(dir, front) <= 0;
    if (sideOrBehind) {
        Object::CollidesWith(c, pair);
        SetVisible(false);
        if (Deco3d *deco = GetDeco()) {
            deco->SetEnabled(false);
            deco->SetVisible(false);
        }
        SetRunnerState(RunnerState::Stopped);
    } else {
        SM_OnEngineEvent(6);
        SetRunnerState(RunnerState::Stopping);
    }
    if (Collision3d *own = GetCollision()) own->SetEnabled(false);
    minion->IncreaseDespicablePoints(m_despicablePoints, c->GetPosition());
    if (BackgroundMgr::GetInstance()->GetLocation() == 2) Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_evilEyesCount, 1);
    minion->SM_OnEngineEvent(m_minionEngineEventOnHit);
}

// @0x521124: the runner's own pacesetter, paused until the runner starts, which the runner follows
void Runner::InitPacesetter() {
    m_pacesetter = new Pacesetter(clara::Project::GetInstance()->FindTemplateByName(Pacesetter::k_tmplName));
    m_pacesetter->Init();
    m_pacesetter->PostInit();
    m_pacesetter->SetPaused(true);
    m_pacesetter->RegisterFollower(this);
    m_pacesetter->RegisterForUpdate(false);
}

// @0x5211c8: the runner joins its pacesetter at its lane, at the minion's speed times its factor, and is the obstacles' target
void Runner::StartPacesetter() {
    GameLevel *level = GameLevel::GetInstance();
    Minion *minion = level->GetMinion();
    m_pacesetter->SetCurrentGameplayTemplateInstance(level->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance());
    m_pacesetter->SetLanePosition(GetPosition(), 0.0f, m_spawnLane, 0);
    m_pacesetter->ApplyCurrentLaneTransform();
    m_pacesetter->SetPaused(false);
    m_pacesetter->SetSpeed(minion->GetPacesetter()->GetTargetSpeed() * m_speedPgt, m_speedTime);
    LinkTo(m_pacesetter, jet::String::null, 0);
    SetTransform(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
    m_targetPacesetter = m_pacesetter;
    ObstacleTemplate::AddTarget(this);
}

// @0x521348: the pacesetter pauses and is disabled when the runner was linked to it; the runner is unlinked
void Runner::StopPacesetter() {
    if (IsLinked() && GetLinkee() == m_pacesetter) {
        m_pacesetter->SetPaused(true);
        m_pacesetter->SetEnabled(false);
    }
    LinkTo(0, jet::String::null, 0);
}

// @0x5213bc: an obstacle the runner meets stops it, and the obstacle's crash event is the engine event the runner takes (0x29 when it takes none)
void Runner::OnObstacleHit(ObstacleTemplate *o) {
    if (m_runnerState == RunnerState::Stopped) return;
    SetRunnerState(RunnerState::Stopped);
    if (!SM_OnEngineEvent(o->GetEngineEventOnCrash())) SM_OnEngineEvent(k_evilEyesEngineEvent);
    ObstacleTemplate::RemoveTarget(this);
}

// @0x521424: the speed factor the data (or the level) gives the runner: its pacesetter goes at the minion's speed times it, unless paused
void Runner::SetSpeedPgt(float speed, int time) {
    m_speedPgt = speed;
    if (time > 0) m_speedTime = time;
    if (!m_pacesetter || m_pacesetter->IsPaused()) return;
    m_pacesetter->SetSpeed(speed * GameLevel::GetInstance()->GetMinion()->GetPacesetter()->GetTargetSpeed(), m_speedTime);
}

// @0x521488: the voice of the runner plays once when the minion is within reach (20 m) of it, and only once
void Runner::UpdateNPCVoiceTrigger() {
    if (!m_triggerNPCVoice) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion->GetPacesetter()) return;
    const math::vec3<float> voicePos = GetDeco() ? GetDeco()->GetPosition() : GetPosition();
    if (math::distanceSq(voicePos, minion->GetPosition()) > k_distanceToTriggerNPCVoiceSq) return;
    GameSoundMgr::GetInstance()->Play(k_NPCVoiceLabel, 200.0f, GetPosition(), 0);
    m_triggerNPCVoice = false;
}

// @0x5215a0: the template of the runner's pacesetter is gone: the runner stops and is no longer followed
void Runner::OnGameplayTemplateInstanceDestroyed() {
    StopPacesetter();
    SetPacesetter(0);
}

// @0x5215c0: a runner that was running or stopping stops; a stopped one is hidden
void Runner::OnMinionRevive() {
    if (m_runnerState == RunnerState::Running || m_runnerState == RunnerState::Stopping) SetRunnerState(RunnerState::Stopped);
    if (m_runnerState == RunnerState::Stopped && GetDeco()) GetDeco()->SetVisible(false);
}

// @0x518d40: the squared distance from the point to the plane through the position whose normal is the front of the rotation
float Runner::GetDistanceSqToPlane(const math::vec3<float> &pos, const math::quat<float> &rot) const {
    const math::vec3<float> front = rot * jet::scene::SceneMgr::GetFrontVector();
    math::plane<float> plane;
    plane.setPlane(pos, front);
    const math::vec3<float> point = GetPosition();
    const float dist = plane.getDistanceTo(point);
    const math::vec3<float> offset(dist * plane.normal.x, dist * plane.normal.y, dist * plane.normal.z);
    return math::distanceSq(point - offset, GetPosition());
}
