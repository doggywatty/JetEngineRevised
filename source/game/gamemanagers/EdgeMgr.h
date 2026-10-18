// source/game/gamemanagers/EdgeMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_EDGEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_EDGEMGR_H

#include "core/pstring.h"

class LevelSequenceParser;

// What decides whether an edge of the level sequence graph may be taken: a condition (a location is available) or a probability that
// grows with the rooms run. The graph asks every edge of a node, then takes one by weights.
class EdgeMgr {  // sizeof 0x4
public:
    virtual ~EdgeMgr() {}
    // reads what the edge's label says
    virtual void Create(const jet::String &label, LevelSequenceParser &parser) = 0;
    virtual bool Evaluate() = 0;
    virtual void Reset() {}
    // after the edge was taken
    virtual void ResetData() {}
    virtual void OnAdvanceLevelTemplateList() {}
    // the name the edge's probability is kept under (empty for none)
    virtual jet::String GetEdgeId() const { return jet::String::null; }
    virtual bool IsEdgeOnDemand() const { return false; }
    virtual unsigned GetProbabilityMultiplier() const { return 1; }
};

// "laboRoomAvailable" and the like: the edge leads to a location the player has unlocked
class EdgeConditionMgr : public EdgeMgr {  // sizeof 0xC
public:
    static const jet::String k_laboAvailableConditionStr, k_resiAvailableConditionStr, k_machoAvailableConditionStr, k_minionsBeachAvailableConditionStr,
        k_resiNightAvailableConditionStr, k_mallAvailableConditionStr;
    EdgeConditionMgr() : m_condition(-1) {}
    virtual void Create(const jet::String &label, LevelSequenceParser &parser);
    virtual bool Evaluate();
    virtual jet::String GetEdgeId() const { return m_conditionStr; }
    virtual bool IsEdgeOnDemand() const;
    virtual unsigned GetProbabilityMultiplier() const;
private:
    int ConditionStringToId(const jet::String&) const;
    int m_condition;             // 0x4: a location, -1 for none
    jet::String m_conditionStr;  // 0x8
};

#endif
