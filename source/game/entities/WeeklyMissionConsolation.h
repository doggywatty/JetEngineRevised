// source/game/entities/WeeklyMissionConsolation.h
#ifndef SOURCE_GAME_ENTITIES_WEEKLYMISSIONCONSOLATION_H
#define SOURCE_GAME_ENTITIES_WEEKLYMISSIONCONSOLATION_H

class WeeklyMissionConsolation : public GameEntity {  // sizeof 0xB0
public:
    WeeklyMissionConsolation(const clara::Template*);
    virtual ~WeeklyMissionConsolation();
    virtual void Init();
    virtual void PostInit();
    jet::String GetConsolationDescription();
    const Price &GetConsolationPopupPrize();
    jet::String GetConsolationPopupText();
    void OnStarted();
    void OnConditionCompleted();
    void OnCancel();
    void SetCompleted(bool);
    bool WasCompleted();
    int GetConditionValue();
    WeeklyMission *GetWeeklyMission();
    void SetWeeklyMission(WeeklyMission*);
    void SetMissionConditions(const ustl::vector<MissionCondition *>&);
private:
    void ClearMissionConditions();
    void UnregisterObservers();
public:
    static jet::String k_tmplName;
private:
    WeeklyMission *m_weeklyMission;  // 0x90
    Price *m_consolationPopupPrize;  // 0x94
    bool m_completed;  // 0x98
    ustl::vector<MissionCondition *> m_conditions;  // 0x9C
    int m_conditionValue;  // 0xAC
};

#endif
