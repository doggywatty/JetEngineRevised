// source/game/entities/WeeklyMission.h
#ifndef SOURCE_GAME_ENTITIES_WEEKLYMISSION_H
#define SOURCE_GAME_ENTITIES_WEEKLYMISSION_H

class WeeklyMission : public GameEntity {  // sizeof 0xC0
public:
    WeeklyMission(const clara::Template*);
    virtual ~WeeklyMission();
    virtual void Init();
    virtual void PostInit();
    int GetTrackedStatValue();
    jet::String GetDescription();
    jet::String GetConsolationDescription();
    jet::String GetConsolationPopupText();
    bool IsAccumulativeStatistic();
    void OnCancel();
    void SetConsolationMissionConditions();
    ustl::vector<WeeklyMissionConsolation *> &GetConsolationPrizeArray();
    MissionId GetMissionId();
private:
    void UnregisterObservers();
public:
    static jet::String k_tmplName;
private:
    MissionId m_id;  // 0x90
    jet::String m_description;  // 0x94
    jet::String m_consolationDescription;  // 0x98
    bool m_completed;  // 0x9C
    bool m_accumulativeStatistic;  // 0x9D
    ustl::vector<MissionCondition *> m_conditions;  // 0xA0
    ustl::vector<WeeklyMissionConsolation *> m_consolationArray;  // 0xB0
};

#endif
