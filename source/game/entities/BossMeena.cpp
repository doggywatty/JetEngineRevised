// source/game/entities/BossMeena.cpp
#include "BossMeena.h"
#include <cmath>
#include "Deco3d.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "Puppet.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../cheats/CheatsMgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "../input/InputMgr.h"
#include "../interfaces/menus/Menu_PortraitMessage.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "../Player.h"
#include "../Statistics.h"
#include "sound/soundmgr.h"
#include "param.h"
#include "scene/model.h"
#include "scene/node.h"

jet::String BossMeena::k_tmplName("BossMeena");
const jet::String BossMeena::k_offsetInPathParam("offsetInPath");
const jet::String BossMeena::k_changeLaneWaitTimeParam("changeLaneWaitTime");
const jet::String BossMeena::k_changeLaneTimeParam("changeLaneTime");
const jet::String BossMeena::k_engineEventOnCrashParam("engineEventOnCrash");
const jet::String BossMeena::k_ramAttackTimeParam("ramAttackTime");
const jet::String BossMeena::k_ramBackTimeParam("ramBackTime");
const jet::String BossMeena::k_ramWinEngineEventParam("ramWinEngineEvent");
const jet::String BossMeena::k_puppetShootDummy("meena_weapon_Dummy");
const jet::String BossMeena::k_meenaSoundLabel("ev_sfx_Meena_bike");

// the engine events of Meena (its .so virtuals and the events its states send): the start and the end of the minion's run with her, the
// dying and the exit, and the events of the ram (the attack's start 0x60, its shot 0x65, the minion's ram 0x61)
static const int k_startMinionEvent = 0x6F;      // @0x5a50c0
static const int k_endMinionEvent = -1;          // @0x5a50c8
static const int k_dyingMinionEvent = 0x70;      // @0x5a50d0
static const int k_exitMinionEvent = 0x70;       // @0x5a50d8
static const int k_ramStartEvent = 0x60;         // @0x5a7730 (StartAttack)
static const int k_shootEvent = 0x65;            // @0x5a5054 (OnShoot)
static const int k_ramMinionEvent = 0x61;        // @0x5a7204 (UpdateMeleeAttack)
// the state types of Meena's movement: the attack's states (0x3C, 0x3D), the lane's states (0x3E, 0x3F), the ram's (0x41, 0x42)
static const int k_attackStateType = 0x3C;       // @0x5a50e0
static const int k_attackStateType2 = 0x3D;
static const int k_laneStateType = 0x3E;         // @0x5a6f04
static const int k_laneStateType2 = 0x3F;
static const int k_ramStateType = 0x41;
// the ram's point at which the minion is blocked (the offset's value below it, UpdateMeleeAttack and IsBlockingMinion)
static const float k_ramBlockOffset = 1.5f;      // @0x5a71b4
// the time (ms) the sound takes to stop after the ram (@0x5a71cc)
static const int k_soundStopTime = 500;
// the time (ms) the game takes to stop for the ram's tutorial (@0x5a7610)
static const int k_tutoStopTransitionTime = 2000;

// @0x5a6b54
BossMeena::BossMeena(const clara::Template *t)
    : Boss(t), m_lastLane(1), m_currentLane(1), m_laneDelayer(), m_offsetInPath(0), m_offsetInPathDelayer(), m_originalRot(0, 0, 0, 1),
      m_changeLaneWaitTime(0), m_changeLaneTime(0), m_engineEventOnCrash(-1), m_ramAttackTime(0), m_ramBackTime(0), m_ramWinEngineEvent(0),
      m_ramTutoDisplayed(Player::GetInstance()->GetMeenaRamTutoDisplayed()) {
    m_tutorialBit = 1;  // @0x5a6bf4
}

// @0x5a6c44
BossMeena::~BossMeena() {}

// @0x5a6ccc
void BossMeena::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Boss::PostInit();
    GetParam(k_offsetInPathParam, m_offsetInPath);
    GetParam(k_changeLaneWaitTimeParam, m_changeLaneWaitTime);
    GetParam(k_changeLaneTimeParam, m_changeLaneTime);
    GetParam(k_engineEventOnCrashParam, m_engineEventOnCrash);
    GetParam(k_ramAttackTimeParam, m_ramAttackTime);
    GetParam(k_ramBackTimeParam, m_ramBackTime);
    GetParam(k_ramWinEngineEventParam, m_ramWinEngineEvent);
    m_originalRot = GetRotation();
    // the lane change starts from its own lane (zero), and the offset along the path is the data's offset with no time
    m_laneDelayer.SetDelay(m_changeLaneTime);
    m_laneDelayer.SetCurrentValue(0.0f);
    m_laneDelayer.SetTargetValue(0.0f);
    m_offsetInPathDelayer.SetDelay(0);
    m_offsetInPathDelayer.SetCurrentValue(m_offsetInPath);
    m_offsetInPathDelayer.SetTargetValue(m_offsetInPath);
}

// @0x5a74f8
bool BossMeena::IsInLane(LaneIndex l) { return m_currentLane == (int)l; }

// @0x5a750c: in the lane, and the offset along the path is short of the ram's block
bool BossMeena::IsBlockingMinion(LaneIndex l) {
    if (!IsInLane(l)) return false;
    return m_offsetInPathDelayer.GetCurrentValue() < k_ramBlockOffset;
}

// @0x5a7730: the attack starts with the ram's engine event
void BossMeena::StartAttack() { SM_OnEngineEvent(k_ramStartEvent); }

// @0x5a5054
void BossMeena::OnShoot() { SM_OnEngineEvent(k_shootEvent); }

// @0x5a50e0: the boss can shoot in its attack states (0x3C, 0x3D) and in its ram's (0x41, 0x42)
bool BossMeena::CanShoot() const {
    const ActorState *state = SM_GetState();
    const int type = state ? state->m_type : 0;
    if (type == k_attackStateType || type == k_ramStateType || type == k_ramStateType + 1) return true;
    return type == k_attackStateType2;
}

// @0x5a506c: the bomb is shot from the boss's place
math::vec3<float> BossMeena::GetBombShootPosition() const { return GetPosition(); }

// @0x5a7748: the shot is fired from the puppet's shoot dummy when its model has it, else from the boss's place
math::vec3<float> BossMeena::GetShootPosition() const {
    if (Puppet *p = GetPuppet()) {
        if (Deco3d *deco = p->GetDeco()) {
            if (deco->GetModel()) {
                if (const jet::scene::Node *node = deco->GetModel()->FindNodeByName(k_puppetShootDummy)) return node->GetAbsolutePosition();
            }
        }
    }
    return GetPosition();
}

// @0x5a50a4
const jet::String &BossMeena::GetBossSoundLabel() const { return k_meenaSoundLabel; }

// @0x5a50c0
int BossMeena::GetStartMinionEvent() const { return k_startMinionEvent; }

// @0x5a50c8
int BossMeena::GetEndMinionEvent() const { return k_endMinionEvent; }

// @0x5a50d0
int BossMeena::GetBossStartDyingMinionEvent() const { return k_dyingMinionEvent; }

// @0x5a50d8
int BossMeena::GetBossExitMinionEvent() const { return k_exitMinionEvent; }

// @0x5a7888: the boss's win event goes to its state machine (the obstacle is not used), and the boss's sound stops
void BossMeena::OnMinionHittedByBossObstacle(ObstacleTemplate*) {
    Boss::OnMinionHittedByBossObstacle(0);
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->Stop(m_bossSoundHandle, k_soundStopTime);
}

// @0x5a753c: the ram's first event (0x60) with the tutorial not shown yet starts the tutorial: the dodge message, the tutorial graph of the input
// (2 when the minion is in lane 0, else 1), the input's tutorial with this listener, and the time stopped until the tutorial is done. Every event
// then goes to the state machine.
bool BossMeena::SM_OnEngineEvent(int e) {
    if (!m_ramTutoDisplayed && e == k_ramStartEvent) {
        const int tutoId = GameLevel::GetInstance()->GetMinion()->GetPacesetter()->GetCurrentLane() == 0 ? 2 : 1;
        const jet::String &text = babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("DODGE_MEENA_RAM"));
        Menu_PortraitMessage::ShowMessage(1, text, 0);
        Menu_PortraitMessage::ShowTutoGraph(InputMgr::TutorialInputId{tutoId});
        InputMgr::GetInstance()->SetTutorial(InputMgr::TutorialInputId{tutoId}, this);
        Game::GetInstance()->SetTimeSpeed(0.0f, k_tutoStopTransitionTime, 0);
    }
    return StateMachine::SM_OnEngineEvent(e);
}

// the listener's virtual: the input's tutorial is done (the .so's thunk at 0x5a7728 goes to the same callback)
void BossMeena::OnTutorialInputDone(const InputMgr::TutorialInputId &) { OnTutorialDoneCallback(); }

// @0x5a7684: the message is hidden, the input's tutorial is cleared (no id, no listener), the time runs again at full speed, and the ram's
// tutorial is kept as shown in the player's save
void BossMeena::OnTutorialDoneCallback() {
    Menu_PortraitMessage::HideMessage();
    InputMgr::GetInstance()->SetTutorial(InputMgr::TutorialInputId{0}, 0);
    Game::GetInstance()->SetTimeSpeed(1.0f, 0, 0);
    m_ramTutoDisplayed = true;
    Player::GetInstance()->SetMeenaRamTutoDisplayed(true);
}

// @0x5a77ec: the boss's outro sound, the score of the defeat and the defeat of a Meena
void BossMeena::OnDie() {
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->SetState(m_bossSoundHandle, k_bossSoundStateOutro);
    Statistics *stats = Statistics::GetInstance();
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_score, Game::GetInstance()->GetScoreValue(Game::Score_BossBeaten));
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_defeatMeena, 1);
}

// @0x5a6e44: the offset along the path and the lane change are timed by their delayers while they run; the ram's attack state rests the offset's
// delayer (the minion's engine event 4); then the lane follows the lane change, the ram is checked, and the place is set
void BossMeena::UpdatePersecution() {
    const int ms = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    if (m_offsetInPathDelayer.IsUpdating()) m_offsetInPathDelayer.Update(ms);
    if (const ActorState *state = SM_GetState()) {
        if ((state->m_type == k_attackStateType || state->m_type == k_attackStateType2) && !m_offsetInPathDelayer.IsUpdating()) SM_OnEngineEvent(4);
    }
    if (m_laneDelayer.IsUpdating()) {
        m_laneDelayer.Update(ms);
        UpdateCurrentLane();
    }
    UpdateMeleeAttack();
    UpdatePosition();
}

// @0x5a7150: the ram. A minion in the boss's lane while the offset is short of the block starts the ram: the offset is taken to the ram's
// target, the minion gets the crash event, the boss its win event, and the ram's sound stops. The first gate (@0x5a7158..0x5a7198) is the
// cheat invincibility (CheatsMgr::IsInvincible, cheats on and the invincible cheat on): no ram.
void BossMeena::UpdateMeleeAttack() {
    if (CheatsMgr::GetInstance()->IsInvincible()) return;
    // the power up gate (@0x5a71e4): the minion is not rammed while the invincible power up is on (the .so reads the status at index 2)
    if (PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::Invincible)) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!IsInLane(minion->GetPacesetter()->GetCurrentLane())) return;
    if (m_offsetInPathDelayer.GetCurrentValue() >= k_ramBlockOffset) return;
    SM_OnEngineEvent(k_ramMinionEvent);
    m_offsetInPathDelayer.SetTargetValue(m_offsetInPathDelayer.GetCurrentValue());
    minion->SM_OnEngineEvent(m_engineEventOnCrash);
    SM_OnEngineEvent(m_ramWinEngineEvent);
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->Stop(m_bossSoundHandle, k_soundStopTime);
}

// @0x5a7270: the lane change's whole part (rounded from its magnitude) moves the boss's lane in its direction, and the change keeps only its
// fraction (current and target both)
void BossMeena::UpdateCurrentLane() {
    const float x = m_laneDelayer.GetCurrentValue();
    const float target = m_laneDelayer.GetTargetValue();
    const int n = (int)std::floor(std::fabs(x) + 0.5f);
    if (n <= 0) return;
    const int dir = x < 0 ? -1 : 1;
    const int moved = n * dir;
    m_currentLane += moved;
    m_laneDelayer.SetCurrentValue(x - (float)moved);
    m_laneDelayer.SetTargetValue(target - (float)moved);
}

// @0x5a7344: the lane point and the path offset place the boss (the shared lane placement)
void BossMeena::UpdatePosition() { UpdateLanePosition(m_currentLane, m_laneDelayer.GetCurrentValue(), m_offsetInPathDelayer.GetCurrentValue()); }

// @0x5a6f04: the lane change and the offset's delayer follow the state change (the states are Meena's action types: 0x3C attack, 0x3D its
// end, 0x3E and 0x3F the lane states, 0x41 the ram). Entering a lane state from another one, the lane change goes from the last lane to the current
// one; the attack and its end set the offset's delayer (its ram delay, then the path's offset); the end of the attack's end sets the lane change to
// the next lane; and the ram's state sets the lane change to the minion's lane.
void BossMeena::SM_OnChangeState(ActorState *from, ActorState *to) {
    Boss::SM_OnChangeState(from, to);
    if (!to || !from) return;
    const int f = from->m_type, t = to->m_type;
    if (f != k_laneStateType && f != k_laneStateType2 && (t == k_laneStateType || t == k_laneStateType2)) {
        m_laneDelayer.SetCurrentValue(0.0f);
        m_laneDelayer.SetTargetValue((float)m_lastLane - (float)m_currentLane);
    }
    if (f != k_attackStateType && t == k_attackStateType) {
        m_offsetInPathDelayer.SetDelay(m_ramAttackTime);
        m_offsetInPathDelayer.SetTargetValue(0.0f);
    }
    if (f != k_attackStateType2 && t == k_attackStateType2) {
        m_offsetInPathDelayer.SetDelay(m_ramBackTime);
        m_offsetInPathDelayer.SetTargetValue(m_offsetInPath);
    }
    if (f == k_attackStateType2 && t != k_attackStateType2) {
        m_offsetInPathDelayer.SetCurrentValue(m_offsetInPath);
        m_offsetInPathDelayer.SetTargetValue(m_offsetInPath);
        m_laneDelayer.SetCurrentValue(0.0f);
        m_laneDelayer.SetTargetValue(1.0f - (float)m_currentLane);
    }
    if (f != k_ramStateType && t == k_ramStateType) {
        m_laneDelayer.SetCurrentValue(0.0f);
        m_laneDelayer.SetTargetValue((float)GameLevel::GetInstance()->GetMinion()->GetPacesetter()->GetCurrentLane() - (float)m_currentLane);
    }
}
