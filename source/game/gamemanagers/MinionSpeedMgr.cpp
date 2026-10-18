// source/game/gamemanagers/MinionSpeedMgr.cpp
#include "MinionSpeedMgr.h"
#include "MissionMgr.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../entities/Minion.h"
#include "../gamestates/GS_Gameplay.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "param.h"
#include "project.h"
#include <cmath>
#include <climits>

const float MinionSpeedMgr::k_roomSize = 180.0f;  // @0xc8ae34
const float MinionSpeedMgr::k_minimumSpeedForIncreaseFactor = 10.0f;  // @0xc8ae38
const jet::String MinionSpeedMgr::k_speedIntervalsParam("SpeedIntervals");
const jet::String MinionSpeedMgr::k_roomAmountParam("RoomAmount");
const jet::String MinionSpeedMgr::k_speedParam("Speed");
const jet::String MinionSpeedMgr::k_speedIntervalOnReviveParam("SpeedIntervalOnRevive");

// @0x4f5d9c
MinionSpeedMgr::MinionSpeedMgr()
    : m_speedDelayer(0.0f, 0.0f, 1000), m_distance(0), m_roomsRunned(0), m_currentInterval(0), m_speedIntervalOnRevive(0), m_forceSpeedByScript(false), m_speedByScript(0) {}

// @0x4f5e38
MinionSpeedMgr::~MinionSpeedMgr() {
    for (SpeedInterval *i : m_speedIntervals) delete i;
}

// @0x4f5ed8: the intervals of the minion's speed definition (the "Slow" ones while the multiplier of the missions is low), and the one
// to go on with after a revive; the run starts at the first
void MinionSpeedMgr::PostInit() {
    if (Minion *minion = GameLevel::GetInstance()->GetMinion()) {
        clara::DataEntity *def = minion->GetSpeedDefinition();
        const clara::Param *intervals = def->FindParamByName(k_speedIntervalsParam);
        if ((int)MissionMgr::GetInstance()->GetMultiplierLevel() + 1 <= 2) {
            jet::String slow = k_speedIntervalsParam;
            slow.append("Slow");
            intervals = def->FindParamByName(slow);
        }
        if (intervals) {
            for (unsigned i = 0; i < intervals->GetComponentCount(); i++) {
                clara::DataEntity *e = intervals->GetAsEntity(i);
                SpeedInterval *s = new SpeedInterval;
                s->roomAmount = 0;
                s->speed = 0;
                e->GetParam(k_roomAmountParam, s->roomAmount);
                e->GetParam(k_speedParam, s->speed);
                m_speedIntervals.push_back(s);
            }
        }
        def->GetParam(k_speedIntervalOnReviveParam, m_speedIntervalOnRevive);
    }
    if (!m_speedIntervals.empty()) {
        m_roomsRunned = 0;
        m_currentInterval = 0;
        m_distance = 0;
        m_speedDelayer.ResetValues(m_speedIntervals[0]->speed);
        SetSpeedDelayer();
    }
}

// @0x4f6104: the speed eases while the run is on (not while a script forces it or the template is a special node's)
void MinionSpeedMgr::Update(unsigned dt) {
    if (!Game::GetInstance()->IsInActionPhase()) return;
    if (GameLevel::GetInstance()->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance()->IsGeneratedBySpecialNode()) return;
    if (m_forceSpeedByScript) return;
    m_speedDelayer.Update((int)dt);
}

// @0x4f61dc: the speed a script forces or the eased one. The large minion's own speed and the rocket's are not ported with those modes.
float MinionSpeedMgr::GetCurrentSpeed() const { return m_forceSpeedByScript ? m_speedByScript : m_speedDelayer.GetCurrentValue(); }

// @0x4f6254: a level template of a normal run was passed: its length adds to the distance, and every room that is full goes on to the next
// room of the interval or to the next interval
void MinionSpeedMgr::OnAdvanceLevelTemplateList() {
    GameLevel *level = GameLevel::GetInstance();
    GameplayTemplateInstance *gti = level->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance();
    if (gti->GetBifurcation() != Bifurcation_Center) return;
    GS_Gameplay *g = GS_Gameplay::GetInstance();
    if (!g || g->GetCurrentMode() == GS_Gameplay::Mode_Boss) return;
    if (gti->IsGeneratedBySpecialNode()) return;
    if (level->GetMinion()->IsInFastForward()) return;
    if (m_forceSpeedByScript) return;
    m_distance += (int)gti->GetDefinition()->GetSizeY();
    while ((float)m_distance >= k_roomSize) {
        m_distance -= (int)k_roomSize;
        m_roomsRunned++;
        if (m_currentInterval < (int)m_speedIntervals.size() && m_roomsRunned >= (unsigned)m_speedIntervals[(unsigned)m_currentInterval]->roomAmount) {
            m_currentInterval++;
            m_roomsRunned = 0;
            SetSpeedDelayer();
        }
    }
}

// the float to int conversion of the ARM (vcvt): NaN is 0 and what does not fit saturates
static int ArmFloatToInt(float f) {
    if (f != f) return 0;
    if (f >= 2147483648.0f) return INT_MAX;
    if (f <= -2147483648.0f) return INT_MIN;
    return (int)f;
}

// @0x4f638c: the speed goes from the interval's to the next one's over the time the constant acceleration that does it in the rooms of the
// interval takes: v1^2 - v0^2 = 2 a d, with d = rooms * k_roomSize
void MinionSpeedMgr::SetSpeedDelayer() {
    if (m_currentInterval >= (int)m_speedIntervals.size() - 1) return;
    const SpeedInterval *from = m_speedIntervals[(unsigned)m_currentInterval], *to = m_speedIntervals[(unsigned)m_currentInterval + 1];
    const float v0 = from->speed, v1 = to->speed;
    const float acceleration = (v1 * v1 - v0 * v0) / ((float)from->roomAmount * 360.0f);
    const int ms = ArmFloatToInt((v1 - v0) / acceleration * 1000.0f);
    m_speedDelayer.SetDelay(ms);
    m_speedDelayer.SetTargetValue(v1);
}

// the revive: the rooms of the interval begin again, and the speed is the one of the interval of the data when the run was farther on
void MinionSpeedMgr::OnMinionRevive() {
    m_roomsRunned = 0;
    if (m_currentInterval > m_speedIntervalOnRevive) {
        m_currentInterval = m_speedIntervalOnRevive;
        m_speedDelayer.SetDelay(0);
        m_speedDelayer.ResetValues(m_speedIntervals[(unsigned)m_currentInterval]->speed);
    }
}
