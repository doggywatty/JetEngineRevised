// source/game/entities/BossMacho.cpp
#include "BossMacho.h"
#include "Deco3d.h"
#include "ObstacleTemplate.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../Statistics.h"
#include "sound/soundmgr.h"
#include "param.h"
#include "scene/model.h"
#include "scene/node.h"

jet::String BossMacho::k_tmplName("BossMacho");
const jet::String BossMacho::k_offsetInPathParam("offsetInPath");
const jet::String BossMacho::k_changeLaneWaitTimeParam("changeLaneWaitTime");
const jet::String BossMacho::k_changeLaneTimeParam("changeLaneTime");
const jet::String BossMacho::k_engineEventOnCrashParam("engineEventOnCrash");
// the sound label of El Macho is empty (@0x5a513c: the string at k_machoSoundLabel)
const jet::String BossMacho::k_machoSoundLabel("");

// the engine events of El Macho (its .so virtuals): the shot, the minion's start and end, and the dying and exit ones
static const int k_shootEngineEvent = 0x7B;         // @0x5a511c
static const int k_startMinionEvent = 0x7E;         // @0x5a5154
static const int k_endMinionEvent = -1;             // @0x5a515c
static const int k_dyingMinionEvent = 0x7F;         // @0x5a5164
static const int k_exitMinionEvent = 0x7F;          // @0x5a516c
// the node the bomb is shot from (@0x5a69ac)
static const char *k_bombShootNode = "fx08_Dummy";

// @0x5a6518
BossMacho::BossMacho(const clara::Template *t)
    : Boss(t), m_lastLane(1), m_lane(1), m_laneDelayer(), m_offsetInPath(0), m_offsetInPathDelayer(), m_originalRot(0, 0, 0, 1),
      m_changeLaneWaitTime(0), m_changeLaneTime(0), m_engineEventOnCrash(-1) {
    m_tutorialBit = 2;  // @0x5a65c0
}

// @0x5a65dc
BossMacho::~BossMacho() {}

// @0x5a665c
void BossMacho::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Boss::PostInit();
    GetParam(k_offsetInPathParam, m_offsetInPath);
    GetParam(k_changeLaneWaitTimeParam, m_changeLaneWaitTime);
    GetParam(k_changeLaneTimeParam, m_changeLaneTime);
    GetParam(k_engineEventOnCrashParam, m_engineEventOnCrash);
    m_originalRot = GetRotation();
    // the lane change starts from its own lane (zero), and the offset along the path is set to the data's offset with no time
    m_laneDelayer.SetDelay(m_changeLaneTime);
    m_laneDelayer.SetCurrentValue(0.0f);
    m_laneDelayer.SetTargetValue(0.0f);
    m_offsetInPathDelayer.SetDelay(0);
    m_offsetInPathDelayer.SetCurrentValue(m_offsetInPath);
    m_offsetInPathDelayer.SetTargetValue(m_offsetInPath);
}

// @0x5a6998
bool BossMacho::IsInLane(LaneIndex l) const { return m_lane == (int)l; }

// @0x5a5118: El Macho does not start an attack by itself
void BossMacho::StartAttack() {}

// @0x5a511c: the shot is the engine event 0x7B of the boss's state machine
void BossMacho::OnShoot() { SM_OnEngineEvent(k_shootEngineEvent); }

// @0x5a69ac: the bomb is shot from the deco's dummy node (fx08_Dummy) when the model has it, else from the boss's place
math::vec3<float> BossMacho::GetBombShootPosition() const {
    if (m_deco && m_deco->GetModel()) {
        if (const jet::scene::Node *node = m_deco->GetModel()->FindNodeByName(jet::String(k_bombShootNode))) return node->GetAbsolutePosition();
    }
    return GetPosition();
}

// @0x5a6a54
math::vec3<float> BossMacho::GetShootPosition() const { return GetPosition(); }

// @0x5a5138
const jet::String &BossMacho::GetBossSoundLabel() const { return k_machoSoundLabel; }

// @0x5a5154
int BossMacho::GetStartMinionEvent() const { return k_startMinionEvent; }

// @0x5a515c
int BossMacho::GetEndMinionEvent() const { return k_endMinionEvent; }

// @0x5a5164
int BossMacho::GetBossStartDyingMinionEvent() const { return k_dyingMinionEvent; }

// @0x5a516c
int BossMacho::GetBossExitMinionEvent() const { return k_exitMinionEvent; }

// @0x5a6b28: a crash with an obstacle sends the obstacle's crash engine event to the boss's state machine; without one, the boss's win event
void BossMacho::OnMinionHittedByBossObstacle(ObstacleTemplate *o) {
    if (!o) {
        Boss::OnMinionHittedByBossObstacle(o);
        return;
    }
    SM_OnEngineEvent(o->GetEngineEventOnCrash());
}

// @0x5a6a8c: the boss's outro sound, the score of the defeat and the defeat of a Macho
void BossMacho::OnDie() {
    if (m_bossSoundHandle >= 0) SoundMgr::GetInstance()->SetState(m_bossSoundHandle, k_bossSoundStateOutro);
    Statistics *stats = Statistics::GetInstance();
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_score, Game::GetInstance()->GetScoreValue(Game::Score_BossBeaten));
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_defeatMacho, 1);
}

// @0x5a6780: the lane change is timed by the offset's delayer while it runs, and the position follows
void BossMacho::UpdatePersecution() {
    const int ms = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    if (m_offsetInPathDelayer.IsUpdating()) m_offsetInPathDelayer.Update(ms);
    UpdatePosition();
}

// @0x5a67e4: the boss is on the lanes of the minion's pacesetter, at the lane change's current offset and the path offset
void BossMacho::UpdatePosition() { UpdateLanePosition(m_lane, m_laneDelayer.GetCurrentValue(), m_offsetInPathDelayer.GetCurrentValue()); }
