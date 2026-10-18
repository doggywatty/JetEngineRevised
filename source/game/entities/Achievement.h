// source/game/entities/Achievement.h
#ifndef SOURCE_GAME_ENTITIES_ACHIEVEMENT_H
#define SOURCE_GAME_ENTITIES_ACHIEVEMENT_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"

class MissionCondition;
class AchievementSet;
class Price;

// An achievement ("Achievement" data entity, 0x5c bytes in the .so): conditions that all have to hold (counted from the
// start of the career), a prize and, when social features are on, the achievements it depends on.
// Not ported: the in-game notification (IGNotification_Achievement), the tracking event and the Game Center / Facebook
// publication (OnlinePlayerData::PublishAchievementActivity).
class Achievement : public clara::DataEntity {
public:
    explicit Achievement(const clara::Template*);
    virtual ~Achievement() {}
    virtual void Init();                                  // @0x59dee8
    virtual void PostInit();                              // @0x59df04
    void OnConditionCompleted();                          // @0x59e1dc
    void OnStarted();                                     // @0x59e288
    void OnForceCompletion();                             // @0x59e2ec
    const jet::String &GetAchievementDescription();       // @0x59e328
    const jet::String &GetAchievementName();              // @0x59e434
    void MarkAsPreviouslyCompleted();                     // @0x59e4c0
    void UnregisterObservers();                           // @0x59e4cc
    float GetCompletedPercentage();                       // @0x59e4f0
    void GetCompletedInt(int &done, int &total);          // @0x59e564
    void ForceConditionUpdate();                          // @0x59e5f0
    bool CanBeSeenByUser();                               // @0x59e628
    void ShowIngameNotification();                        // @0x59e690
    bool IsCompleted() const { return m_completed; }
    const std::vector<MissionCondition*> &GetConditions() const { return m_conditions; }
    const jet::String &GetConditionsScope() const { return m_conditionsScope; }
    int GetAchievementId() const { return m_id; }
    Price *GetPrize() const { return m_prize; }
    const jet::String &GetGameCenterId() const { return m_gameCenterId; }
    bool ShouldShareInFacebook() const { return m_shareInFacebook; }
    const jet::String &GetNameCopy() const { return m_nameCopy; }
    void SetSet(AchievementSet *s) { m_set = s; }
    static const jet::String k_tmplName;
private:
    jet::String m_conditionsScope;               // 0x20: "Career" or the like
    int m_location;                              // 0x24
    std::vector<MissionCondition*> m_conditions; // 0x28
    int m_id;                                    // 0x34
    jet::String m_name;                          // 0x38
    jet::String m_description;                   // 0x3c
    Achievement *m_dependency;                   // 0x40
    bool m_completed;                            // 0x44
    jet::String m_gameCenterId;                  // 0x48
    Price *m_prize;                              // 0x4c
    AchievementSet *m_set;                       // 0x50
    jet::String m_nameCopy;                      // 0x54
    bool m_shareInFacebook;                      // 0x58
};

#endif
