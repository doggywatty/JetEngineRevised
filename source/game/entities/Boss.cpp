// source/game/entities/Boss.cpp
#include "Boss.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "Deco3d.h"
#include "Puppet.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../Statistics.h"
#include "../gamestates/GS_Gameplay.h"
#include "../gamemanagers/GameSoundMgr.h"
#include "sound/soundmgr.h"
#include "project.h"
#include <algorithm>
#include <cmath>
#include "scene/scenemgr.h"

jet::String Boss::k_tmplName("Boss");
const jet::String Boss::k_posOffset("posOffset");
const jet::String Boss::k_health("health");
const jet::String Boss::k_timeToResTore("timeToRestore");
const jet::String Boss::k_winEngineEvent("winEngineEvent");
const jet::String Boss::k_vectorSoundLabel("ev_sfx_vector_ship");
const jet::String Boss::k_bossSoundStateIntroLoop("introloop");
const jet::String Boss::k_bossSoundStateOutro("outro");

// the engine events the boss sends the minion (the .so's virtual getters)
static const int k_startMinionEvent = 0x47;       // @0x599d80
static const int k_endMinionEvent = 0x4a;         // @0x599d88
static const int k_startDyingMinionEvent = 0x48;  // @0x599d90
static const int k_exitMinionEvent = 0x49;        // @0x599d98
// the engine event of the state machine when the boss is hit (IncreaseHealth)
static const int k_hitEngineEvent = 6;            // @0x5a45cc
// the state label of the boss when it is beaten (IncreaseHealth) and the one the minion gets when it is set up (SetBehaviorState)
static const int k_defeatedStateLabel = 10;       // @0x5a45b4
static const int k_bossStateLabel = 8;            // @0x5a469c (SetBehaviorState's active tail)
static const int k_minionAppearStateLabel = 1;    // @0x5a4650 (SetBehaviorState active: the minion's label)
// the ActorState type of the states the GameLevel's gameplay changes on (SM_OnChangeState)
static const int k_appearStateType = 0xF;         // @0x5a4afc
static const int k_endStateType = 0x1E;           // @0x5a4bec
// the time (ms) of the pacesetter's reach beyond which the boss keeps its place (Update)
static const float k_approachTimeMs = 100.0f;     // @0x5a4a48
// the time (ms) the sound takes to stop when the boss is gone (~Boss)
static const int k_soundStopTime = 500;           // @0x5a442c

// @0x5a4370
Boss::Boss(const clara::Template *t)
    : Actor(t), m_tutorialBit(0), m_positionOffset(0.0f, 0.0f, 0.0f), m_bossSoundHandle(-1), m_behaviorState(BossState_Hidden), m_dyingTimeLeft(0), m_dyingTime(0),
      m_health(0.0f), m_maxHealth(0.0f), m_timeToRestore(0), m_timeToRestoreCount(0), m_winEngineEvent(0), m_puppet(0) {}

// @0x51bb84 (Puppet::LinkToBoss): the puppet is the boss's, and the boss's state machine events go through the puppet's
void Boss::SetPuppet(Puppet *p) {
    m_puppet = p;
    SM_LinkTo(p);
}

// @0x5a43e4
Boss::~Boss() {
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->Stop(m_bossSoundHandle, k_soundStopTime);
}

// @0x5a44bc
void Boss::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Actor::PostInit();
    GetParam(k_posOffset, m_positionOffset);
    GetParam(k_health, m_maxHealth);
    m_health = m_maxHealth;
    m_timeToRestoreCount = 0;
    GetParam(k_timeToResTore, m_timeToRestore);
    GetParam(k_winEngineEvent, m_winEngineEvent);
    SetBehaviorState(BossState_Hidden);
}

// @0x5a4570: the health goes up (or down) by the amount within 0 and the maximum; it starts to come back after the restore time. A boss with
// health still has the hit's engine event; one with none is beaten (defeated state).
void Boss::IncreaseHealth(float amount) {
    m_health = math::clamp(m_health + amount, 0.0f, m_maxHealth);
    m_timeToRestoreCount = m_timeToRestore;
    if (m_health == 0.0f) {
        SM_SetStateLabel(k_defeatedStateLabel);
        SetBehaviorState(BossState_Defeated);
    } else {
        SM_OnEngineEvent(k_hitEngineEvent);
    }
}

// @0x5a4a5c
void Boss::EnableBossBehavior(bool enable) {
    if (enable) SetBehaviorState(BossState_Active);
}

// @0x5a4a6c
void Boss::DisableBossBehavior(bool disable) {
    if (disable) SetBehaviorState(BossState_Dying);
}

// @0x5a4a7c: a boss that is not the level's boss, and that is exiting, is out of the scene when its object is
bool Boss::IsOutOfScene() const {
    if (this == GameLevel::GetInstance()->GetBoss()) return false;
    if (m_behaviorState != BossState_Exit) return false;
    return Object::IsOutOfScene();
}

// @0x599cb4: a boss can be spawned only when no boss is on the level
bool Boss::CanSpawnNewEntity(LevelTemplateInstance*) const { return GameLevel::GetInstance()->GetBoss() == 0; }

// @0x599cec
bool Boss::CanShoot() const { return true; }

// @0x599ce4
void Boss::StartAttack() {}

// @0x599ce8
void Boss::OnShoot() {}

// @0x599cf4: the bomb is shot from the boss's place
math::vec3<float> Boss::GetBombShootPosition() const { return GetPosition(); }

// @0x599d2c: the shot is fired from the boss's place
math::vec3<float> Boss::GetShootPosition() const { return GetPosition(); }

// @0x599d64
const jet::String &Boss::GetBossSoundLabel() const { return k_vectorSoundLabel; }

// @0x599d80
int Boss::GetStartMinionEvent() const { return k_startMinionEvent; }

// @0x599d88
int Boss::GetEndMinionEvent() const { return k_endMinionEvent; }

// @0x599d90
int Boss::GetBossStartDyingMinionEvent() const { return k_startDyingMinionEvent; }

// @0x599d98
int Boss::GetBossExitMinionEvent() const { return k_exitMinionEvent; }

// @0x5a4dfc: the minion's obstacle hits the boss: the boss's win engine event goes to its state machine (the obstacle is not used)
void Boss::OnMinionHittedByBossObstacle(ObstacleTemplate*) { SM_OnEngineEvent(m_winEngineEvent); }

// @0x5a4df8
void Boss::Render() {}

// @0x5a4c14 (the follow is the lane point of the minion's pacesetter plus the offset turned by the pacesetter's rotation)
math::vec3<float> Boss::GetPersecutionPosition() const {
    Pacesetter *ps = GameLevel::GetInstance()->GetMinion()->GetPacesetter();
    return ps->GetRotation() * m_positionOffset + ps->GetLaneCachePosition(1);
}

// @0x5a4c14
void Boss::UpdatePersecution() { SetPosition(GetPersecutionPosition()); }

// @0x5a67e4 and @0x5a7344: the lane point is the pacesetter's lane point of the lane the offset is in (the offset's whole part moves the lane, its
// fraction goes between the two lanes, each clamped to the three lanes); the place is that point plus the pacesetter's rotation of the path offset,
// which is set back along the scene's front
void Boss::UpdateLanePosition(int lane, float laneOffset, float pathOffset) {
    Pacesetter *ps = GameLevel::GetInstance()->GetMinion()->GetPacesetter();
    if (!ps) return;
    const float sign = laneOffset < 0 ? -1.0f : (laneOffset == 0 ? 0.0f : 1.0f);
    const float magnitude = std::fabs(laneOffset);
    const int whole = (int)(sign * std::floor(magnitude));
    const int wholeCeil = (int)(sign * std::ceil(magnitude));
    const float fraction = laneOffset - (float)whole;
    const int laneA = std::max(0, std::min(2, lane + whole));
    const int laneB = std::max(0, std::min(2, lane + wholeCeil));
    const math::vec3<float> point = math::lerp(ps->GetLaneCachePosition(laneA), ps->GetLaneCachePosition(laneB), std::fabs(fraction));
    m_positionOffset = -(jet::scene::SceneMgr::GetFrontVector() * pathOffset);
    SetPosition(point + ps->GetRotation() * m_positionOffset);
}

// @0x5a45e0: the state changes; the minion, the boss's deco and the state machine follow it
void Boss::SetBehaviorState(Boss::BossState s) {
    m_behaviorState = s;
    GameLevel *level = GameLevel::GetInstance();
    switch (s) {
    case BossState_Hidden:
        m_deco->SetVisible(false);
        break;
    case BossState_Active:
        if (Minion *minion = level->GetMinion()) {
            minion->SM_SetStateLabel(k_minionAppearStateLabel);
            minion->SM_OnEngineEvent(GetStartMinionEvent());
        }
        m_deco->SetVisible(true);
        SM_SetStateLabel(k_bossStateLabel);
        level->SetBossBehaviorOn(true);
        break;
    case BossState_Defeated:
        break;
    case BossState_Dying: {
        if (Minion *minion = level->GetMinion())
            minion->SM_OnEngineEvent(m_health > 0.0f ? GetBossStartDyingMinionEvent() : GetBossExitMinionEvent());
        // the dying time is the length of the state's animation (the state the boss is in), at least one millisecond
        ActorState *state = SM_GetState();
        int time = state ? state->m_animEndTime - state->m_animStartTime : 0;
        m_dyingTime = time < 1 ? 1 : time;
        m_dyingTimeLeft = m_dyingTime;
        break;
    }
    case BossState_Exit:
        if (Minion *minion = level->GetMinion()) {
            const int v = GetEndMinionEvent();
            if (v >= 0) minion->SM_OnEngineEvent(v);
        }
        level->SetBossBehaviorOn(false);
        m_dyingTimeLeft = 0;
        break;
    }
}

// @0x5a4798: the boss follows the minion (the pacesetter's time to the boss's plane), its state runs, its health comes back and its
// sound follows it
void Boss::Update() {
    GameLevel *level = GameLevel::GetInstance();
    const bool isCurrent = this == level->GetBoss();
    Pacesetter *ps = level->GetMinion()->GetPacesetter();
    // the time of a frame, in ms
    const int dt = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    bool regen = true;
    if (ps) {
        switch (m_behaviorState) {
        case BossState_Hidden:
            if (!isCurrent) regen = false;
            else if (!level->GetMinion()->IsFrozenRun()) {
                // the boss comes out when the pacesetter reaches its plane soon enough (the distance is not asked for)
                if (ps->GetTimeToReachPlane(GetPosition(), ps->GetRotation(), m_cache, 0) <= k_approachTimeMs) SetBehaviorState(BossState_Active);
            }
            break;
        case BossState_Active:
        case BossState_Defeated:
            if (!isCurrent) regen = false;
            else UpdatePersecution();
            break;
        case BossState_Dying: {
            // the boss moves toward its place behind the minion, the closer the nearer its time is to the end
            m_dyingTimeLeft -= dt;
            if (m_dyingTimeLeft <= 0) SetBehaviorState(BossState_Exit);
            const float ratio = (float)m_dyingTimeLeft / (float)m_dyingTime;
            const float k = 1.0f - (1.0f - ratio) * (1.0f - ratio);
            const math::vec3<float> target = ps->GetLaneCachePosition(1) + ps->GetRotation() * m_positionOffset;
            SetPosition(math::lerp(GetPosition(), target, k));
            if (!isCurrent) regen = false;
            break;
        }
        default:
            if (!isCurrent) regen = false;
            break;
        }
    } else if (!isCurrent) {
        regen = false;
    }
    if (regen && m_timeToRestoreCount > 0 && m_behaviorState != BossState_Defeated) {
        m_timeToRestoreCount -= dt;
        if (m_timeToRestoreCount <= 0) {
            m_timeToRestoreCount = 0;
            m_health = m_maxHealth;
        }
    }
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->SetPosition(m_bossSoundHandle, GetPosition());
    Actor::Update();
}

// @0x5a4c94: the boss's intro sound plays when it appears, and it is set to its intro loop
void Boss::OnAppear() {
    if (GetBossSoundLabel().empty()) return;
    if (m_bossSoundHandle < 0) m_bossSoundHandle = GameSoundMgr::GetInstance()->Play(GetBossSoundLabel(), 0.0f, GetPosition(), 0);
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->SetState(m_bossSoundHandle, k_bossSoundStateIntroLoop);
}

// @0x5a4d5c: the boss's sound goes to its outro; a beaten boss adds its score and counts as a defeat of a vector
void Boss::OnDie() {
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->SetState(m_bossSoundHandle, k_bossSoundStateOutro);
    Statistics *stats = Statistics::GetInstance();
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_score, Game::GetInstance()->GetScoreValue(Game::Score_BossBeaten));
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_defeatVector, 1);
}

// @0x5a4acc: the boss's state changes: it appears when it enters the appear state (15), is beaten when it enters a die state, and the
// gameplay is told of the boss starting, being defeated, ending (state 30) and the boss gameplay ending
void Boss::SM_OnChangeState(ActorState *from, ActorState *to) {
    SM_ResetStateTracks();
    if (!to || !from) return;
    if (from->m_type != k_appearStateType && to->m_type == k_appearStateType) {
        OnAppear();
        GS_Gameplay::GetInstance()->OnBossStarted();
    }
    if (!from->IsDieState() && to->IsDieState()) {
        OnDie();
        GS_Gameplay::GetInstance()->OnBossDefeated();
    }
    if (from->m_type != k_endStateType && to->m_type == k_endStateType) GS_Gameplay::GetInstance()->OnBossEnded();
    if (from->m_type == k_endStateType && to->m_type != k_endStateType) {
        GS_Gameplay::GetInstance()->OnBossGameplayEnded();
        return;
    }
    if (!from->IsDieState()) return;
    if (to->IsDieState()) return;
    GS_Gameplay::GetInstance()->OnBossGameplayEnded();
}
