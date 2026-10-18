// source/game/entities/MissionCondition.h
#ifndef SOURCE_GAME_ENTITIES_MISSIONCONDITION_H
#define SOURCE_GAME_ENTITIES_MISSIONCONDITION_H

#include "entity.h"
#include "core/pstring.h"
#include "game/Statistics.h"

// One condition of a mission, an achievement or a challenge ("MissionCondition" data entity, 0x60 bytes in the .so): a
// statistic's scope, a comparison and the value it is compared with. It watches the statistic (StatisticsObserver) and
// tells its owner when it holds. The conditions only count integer values (IsFloatValue is false in the .so).
class MissionCondition : public clara::DataEntity, public StatisticsObserver {
public:
    explicit MissionCondition(const clara::Template*);
    explicit MissionCondition(MissionCondition *original);  // @0x57d928: a copy of the data of another
    virtual ~MissionCondition();
    virtual void Init();
    virtual void PostInit();
    // @0x57dddc: the statistic changed
    virtual void OnStatisticChanged(int location, const jet::String &scope, const GameplayStatisticsValue &value);
    void ForceUpdate();                           // @0x57dd70
    void SetComplete(bool);                       // @0x57e128
    void Reset();                                 // @0x57e21c
    void ResetGameSpecificValues();               // @0x57e230
    void RegisterObserver();                      // @0x57e280
    void UnregisterObserver();                    // @0x57e2bc
    void SetRepeatCountValues(int);               // @0x57e30c
    bool IsScopeCodeImplemented();                // @0x57e3d0
    void MissionLinkTo(clara::DataEntity *owner) { m_owner = owner; }  // @0x57e420
    void OnForceCompletion();                     // @0x57e428
    bool IsCompleted() const { return m_completed; }
    float GetCompletedPercentage() const { return m_completed ? 1.0f : m_percentage; }  // @0x57e2f8
    bool IsFloatValue() const { return false; }   // @0x57e204
    float GetFloatValue() const { return m_floatValue; }
    int GetIntValue() const { return m_intValue; }
    const jet::String &GetConditionStatScope() const { return m_scope; }
    void SetLocation(int location) { m_location = location; }
    void SetStatType(int type) { m_type = type; }
    void SetNeedsOnlineServices(bool b) { m_needsOnline = b; }
    static const jet::String k_tmplName;
    // how the statistic is compared with the value
    enum ConditionOperand { Less = 0, LessOrEqual = 1, Equal = 2, GreaterOrEqual = 3, Greater = 4 };
private:
    void SetConditionValue(int);                  // @0x57e3f4
    jet::String m_scope;       // 0x30: the scope of the statistic
    int m_operand;             // 0x34
    bool m_completed;          // 0x38
    float m_percentage;        // 0x3c: how far it is, for the operands that count up
    bool m_observerRegistered; // 0x40
    int m_intValue;            // 0x44: the value to reach
    float m_floatValue;        // 0x48
    float m_repeatMissionMultiplier;  // 0x4c: what the value grows by when a mission is repeated
    int m_claraIntValue;       // 0x50: the value of the data
    float m_claraFloatValue;   // 0x54
    bool m_needsOnline;        // 0x58: only counts with the online services up
    clara::DataEntity *m_owner;  // 0x5c: the mission, achievement or challenge it belongs to
};

#endif
