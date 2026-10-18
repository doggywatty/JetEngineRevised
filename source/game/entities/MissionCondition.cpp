// source/game/entities/MissionCondition.cpp
//
// Not ported: the completion of a Challenge or a WeeklyMissionConsolation (their owners are not ported).
#include "MissionCondition.h"
#include "Achievement.h"
#include "Mission.h"
#include "game/Game.h"
#include <cfloat>
#include <cmath>

const jet::String MissionCondition::k_tmplName("MissionCondition");  // @0xc4093f

// @0x57d8cc
MissionCondition::MissionCondition(const clara::Template *t)
    : clara::DataEntity(t), m_operand(0), m_completed(false), m_percentage(0), m_observerRegistered(false), m_intValue(0), m_floatValue(0),
      m_repeatMissionMultiplier(0), m_claraIntValue(0), m_claraFloatValue(0), m_needsOnline(false), m_owner(0) {}

// @0x57d928
MissionCondition::MissionCondition(MissionCondition *original)
    : clara::DataEntity((const clara::Template*)0), m_operand(0), m_completed(false), m_percentage(0), m_observerRegistered(false), m_intValue(0), m_floatValue(0),
      m_repeatMissionMultiplier(0), m_claraIntValue(0), m_claraFloatValue(0), m_needsOnline(false), m_owner(0) {
    ConstructFrom(original);
}

MissionCondition::~MissionCondition() { UnregisterObserver(); }

// @0x57da74
void MissionCondition::Init() {
    clara::DataEntity::Init();
    m_percentage = 0;
}

// @0x57da8c: the data; the name of the value to reach in the data depends on the scope
void MissionCondition::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("RepeatMissionMultiplier"), m_repeatMissionMultiplier);
    int operand = 0;
    GetParam(jet::String("ConditionOperand"), operand);
    m_operand = operand;
    GetParam(jet::String("ConditionScope"), m_scope);
    jet::String valueName;
    typedef StatisticsSaveData S;
    if (m_scope.Equals(S::k_statScope_distance) || m_scope.Equals(S::k_statScope_costumeRunDistance) || m_scope.Equals(S::k_statScope_runDistanceNoBananas) ||
        m_scope.Equals(S::k_statScope_runDistanceNoPickups) || m_scope.Equals(S::k_statScope_distanceWithoutDespicable))
        valueName = jet::String("DistanceMeters");
    else if (m_scope.Equals(S::k_statScope_bananas) || m_scope.Equals(S::k_statScope_bananaMultiplierBananas) || m_scope.Equals(S::k_statScope_shopSpentBananas))
        valueName = jet::String("BananasCount");
    else if (m_scope.Equals(S::k_statScope_tokens))
        valueName = jet::String("TokensCount");
    else if (m_scope.Equals(S::k_statScope_timeElapsed))
        valueName = jet::String("TimeElapsed");
    else if (m_scope.Equals(S::k_statScope_bestScore))
        valueName = jet::String("BestScore");
    else
        valueName = jet::String("count");
    if (IsFloatValue()) {
        GetParam(valueName, m_floatValue);
        m_claraFloatValue = m_floatValue;
    } else {
        GetParam(valueName, m_intValue);
        m_claraIntValue = m_intValue;
    }
    ResetGameSpecificValues();
}

// @0x57dd70: the value as it is now
void MissionCondition::ForceUpdate() {
    if (!m_registered) return;
    OnStatisticChanged(m_location, m_scope, Statistics::GetInstance()->GetGameplayStatValue(m_location, m_scope, m_type));
}

// @0x57dddc: the statistic of the condition changed: it holds, or counts up to it
void MissionCondition::OnStatisticChanged(int location, const jet::String &scope, const GameplayStatisticsValue &value) {
    if (m_completed) return;
    if (scope != m_scope) return;
    if (location != m_location) return;
    if (m_needsOnline && !Game::GetInstance()->AreOnlineServicesReady()) return;
    // (the .so reads the value twice, one of them decoded, and quits the game when they differ: AntiCheat_GetValue)
    // The .so compares the int of the value (its ProtectedStorage<int>, @0x587c90), not the high word: a 32 bit compare.
    const int v = value.intValue.GetValue();
    bool complete = false;
    switch (m_operand) {
        case Less: complete = v < m_intValue; break;
        case LessOrEqual: complete = v <= m_intValue; break;
        case Equal: complete = v == m_intValue; break;
        case GreaterOrEqual:
            complete = v >= m_intValue;
            if (m_intValue > 0) m_percentage = (float)v / (float)m_intValue;
            break;
        case Greater:
            complete = v > m_intValue;
            if (m_intValue >= 0) m_percentage = (float)v / (float)(m_intValue + 1);
            break;
        default: break;
    }
    SetComplete(complete);
}

// @0x57e128: the owner is told when it holds the first time
void MissionCondition::SetComplete(bool complete) {
    const bool was = m_completed;
    m_completed = complete;
    if (was || !complete || !m_owner) return;
    const jet::String &n = m_owner->GetTemplateName();
    if (n.Equals(Mission::k_tmplName)) {
        static_cast<Mission*>(m_owner)->OnConditionCompleted();
    } else if (n.Equals(Achievement::k_tmplName)) {
        static_cast<Achievement*>(m_owner)->OnConditionCompleted();
    }
    // Challenge::OnConditionCompleted and WeeklyMissionConsolation::OnConditionCompleted: not ported
}

// @0x57e21c
void MissionCondition::Reset() {
    m_completed = false;
    m_percentage = 0;
    ResetGameSpecificValues();
}

// @0x57e230: the distance to beat is the best the player ran (a value the statistics keep)
void MissionCondition::ResetGameSpecificValues() {
    if (!m_scope.Equals(StatisticsSaveData::k_statScope_beatBestOwnDistance)) return;
    m_intValue = Statistics::GetInstance()->GetData().u18;
    m_claraIntValue = m_intValue;
}

// @0x57e280
void MissionCondition::RegisterObserver() {
    if (m_observerRegistered) return;
    m_observerRegistered = true;
    Statistics::GetInstance()->RegisterObserver(this);
}

// @0x57e2bc
void MissionCondition::UnregisterObserver() {
    if (!m_observerRegistered) return;
    m_observerRegistered = false;
    Statistics::GetInstance()->UnregisterObserver(this);
}

// @0x57e30c: a repeated mission asks for more by the multiplier of its condition, as many times as it was repeated
void MissionCondition::SetRepeatCountValues(int repeats) {
    if (repeats > 0 && m_repeatMissionMultiplier > 1.0f && std::fabs(m_repeatMissionMultiplier - 1.0f) > FLT_EPSILON)
        m_intValue = (int)((float)repeats * m_repeatMissionMultiplier * (float)m_claraIntValue);
    else
        m_intValue = m_claraIntValue;
}

// @0x57e3d0
bool MissionCondition::IsScopeCodeImplemented() { return Statistics::IsScopeCodeImplemented(m_scope); }

// @0x57e3f4
void MissionCondition::SetConditionValue(int v) { m_intValue = v; }

// @0x57e428
void MissionCondition::OnForceCompletion() {
    if (!m_completed) SetComplete(true);
}
