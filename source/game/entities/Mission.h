// source/game/entities/Mission.h
#ifndef SOURCE_GAME_ENTITIES_MISSION_H
#define SOURCE_GAME_ENTITIES_MISSION_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"

class MissionCondition;
class Price;

// A mission of the player ("Mission" data entity, 0x94 bytes in the .so): a text, the conditions that all have to hold, a
// giver (Gru, Nefario or the girls), the points it adds to the score multiplier and the price to skip it. While it is one
// of the active missions of MissionMgr its conditions watch the statistics.
// Not ported: the in-game notification of a completed mission (IGNotification_Mission, GS_Gameplay's interface), the
// tracking events and the GUI objects that explain a mission the first time (GuiObject).
class Mission : public clara::DataEntity {
public:
    explicit Mission(const clara::Template*);
    virtual ~Mission();
    virtual void Init() {}
    virtual void PostInit();
    void OnConditionCompleted();                          // @0x57ced4
    void ShowIngameNotification();                        // @0x57cfc0
    void OnMarkAsPreviouslyCompleted();                   // @0x57d09c
    void OnForceCompletion();                             // @0x57d0b4
    void OnAddedToActiveMission(int slot, int repeatCount, bool reset);  // @0x57d180
    const jet::String &GetMissionGiver() const { return m_missionGiver; }  // @0x57d294
    Price *GetSkipPrice() const { return m_skipPrice; }   // @0x57d330
    float GetCompletedPercentage() const;                 // @0x57d338
    bool CanBeSeenByUser();                               // @0x57d3a8
    int GetStatType() const;                              // @0x57d460
    void ForceConditionUpdate();                          // @0x57d4c0
    int GetFirstConditionNumber() const;                  // @0x57d500
    const jet::String &GetDescription();                  // @0x57d520
    void DisplayGuiAssociated(int dt);                    // @0x57d6e0
    void OnReset() { m_completed = false; }               // @0x57d80c
    void UnregisterObservers();                           // @0x57d818
    bool WasMissionDependencyCompleted() const;           // @0x57d840
    bool IsCompleted() const { return m_completed; }
    bool IsActive() const { return m_isActive; }
    bool IsMandatory() const { return m_isMandatory; }
    unsigned GetMissionMultiplierPoints() const { return m_missionMultiplierPoints; }
    bool IsInfiniteMissionLooperSet() const { return false; }
    unsigned GetVersion() const { return m_version; }
    int GetMissionId() const { return m_missionId; }
    const jet::String &GetMissionIdString() const { return m_missionIdString; }
    int GetActiveMissionSlot() const { return m_activeMissionSlot; }
    int GetLocation() const { return m_location; }
    const jet::String &GetScope() const { return m_scope; }
    const std::vector<MissionCondition*> &GetConditions() const { return m_conditions; }
    static const jet::String k_tmplName, k_missionScopeCurrentRun, k_missionScopeCareer, k_missionScopeAccumulative;
    static const jet::String k_missionGiver_Gru, k_missionGiver_Nefario, k_missionGiver_Girls;
private:
    int m_missionId;                           // 0x20
    jet::String m_missionIdString;             // 0x24
    jet::String m_description;                 // 0x28
    unsigned m_version;                        // 0x2c
    int m_location;                            // 0x30: where it counts, -1 for anywhere
    jet::String m_scope;                       // 0x34
    int m_activeMissionSlot;                   // 0x38
    std::vector<MissionCondition*> m_conditions;  // 0x3c
    bool m_completed;                          // 0x50
    bool m_isActive;                           // 0x51: IsMissionActive of the data
    jet::String m_missionGiver;                // 0x4c
    Price *m_skipPrice;                        // 0x54
    unsigned m_missionMultiplierPoints;        // 0x58
    bool m_hasPopup;                           // 0x6c
    bool m_hasDependency;                      // 0x88
    jet::String m_dependencyMissionId;         // 0x8c
    bool m_isMandatory;                        // 0x90
    static const jet::String k_guiAssociated, k_hasPopup;
};

#endif
