// source/game/entities/Challenge.h
#ifndef SOURCE_GAME_ENTITIES_CHALLENGE_H
#define SOURCE_GAME_ENTITIES_CHALLENGE_H

class Challenge : public GameEntity {  // sizeof 0xC8
public:
    Challenge(const clara::Template*);
    virtual ~Challenge();
    virtual void Init();
    virtual void PostInit();
    void OnConditionCompleted();
    void OnStarted(int);
    bool IsCompleted();
    const ChallengePrize &GetChallengePrize(int);
    int GetChallengeValue();
    const jet::String &GetChallengeType();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetChallengeText();
    MissionId GetMissionId();
    static jet::String k_tmplName;
private:
    MissionId m_id;  // 0x90
    ustl::vector<MissionCondition *> m_conditions;  // 0x94
    jet::String m_description;  // 0xA4
    int m_challengeValue;  // 0xA8
    ustl::vector<ChallengePrize *> m_challengePrizes;  // 0xAC
    int m_bananasPrize;  // 0xBC
    bool m_completed;  // 0xC0
    jet::String m_challengeType;  // 0xC4
};

#endif
