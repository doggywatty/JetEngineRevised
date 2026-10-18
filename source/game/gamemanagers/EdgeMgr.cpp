// source/game/gamemanagers/EdgeMgr.cpp
#include "EdgeMgr.h"
#include "BappleMgr.h"
#include "LocationsMgr.h"
#include "../GameLevel.h"
#include "../Player.h"
#include "../entities/LocationInfoEntity.h"
#include "../leveltemplate/LevelSequenceGraph.h"
#include "../leveltemplate/LevelSequenceParser.h"
#include "param.h"

const jet::String EdgeConditionMgr::k_laboAvailableConditionStr("laboRoomAvailable"), EdgeConditionMgr::k_resiAvailableConditionStr("residentialAvailable"),
    EdgeConditionMgr::k_machoAvailableConditionStr("machosLabAvailable"), EdgeConditionMgr::k_minionsBeachAvailableConditionStr("minionsBeachAvailable"),
    EdgeConditionMgr::k_resiNightAvailableConditionStr("resiNightAvailable"), EdgeConditionMgr::k_mallAvailableConditionStr("mallAvailable");

// @0x4c7060
int EdgeConditionMgr::ConditionStringToId(const jet::String &s) const {
    if (s.Equals(k_laboAvailableConditionStr)) return 0;
    if (s.Equals(k_resiAvailableConditionStr)) return 1;
    if (s.Equals(k_machoAvailableConditionStr)) return 2;
    if (s.Equals(k_minionsBeachAvailableConditionStr)) return 3;
    if (s.Equals(k_resiNightAvailableConditionStr)) return 4;
    if (s.Equals(k_mallAvailableConditionStr)) return 5;
    return -1;
}

// @0x4c6e08
void EdgeConditionMgr::Create(const jet::String &label, LevelSequenceParser &parser) {
    jet::String out;
    parser.ProcessConditionalEdge(label, &out);
    m_conditionStr = out;
    m_condition = ConditionStringToId(m_conditionStr);
}

// @0x4c6e64: the location exists, is not hidden, has a graph and was unlocked
bool EdgeConditionMgr::Evaluate() {
    if (m_condition < 0 || m_condition > 5) return false;
    LocationInfoEntity *info = LocationsMgr::GetInstance()->GetLocationInfo(m_condition);
    if (!info || info->IsHidden()) return false;
    if (!GameLevel::s_levelSequenceGraphMgr->HasLevelSequenceGraph(m_condition)) return false;
    return Player::GetInstance()->IsLocationUnlocked(m_condition);
}

// @0x4c6fa8
bool EdgeConditionMgr::IsEdgeOnDemand() const {
    BappleMgr *b = BappleMgr::GetInstance();
    return b && b->AreActiveObjectivesInSpot(EdgeConditionToBappleSpot(m_condition));
}

// @0x4c6ff4: the edge to a location with bapple objectives is wanted more
unsigned EdgeConditionMgr::GetProbabilityMultiplier() const {
    BappleMgr *b = BappleMgr::GetInstance();
    if (!b || !b->AreActiveObjectivesInSpot(EdgeConditionToBappleSpot(m_condition))) return 1;
    int mult = 1;
    GameLevel::GetInstance()->GetGameplayDef()->GetParam(jet::String("BappleLocationProbabilityMultiplier"), mult);
    return (unsigned)mult;
}
