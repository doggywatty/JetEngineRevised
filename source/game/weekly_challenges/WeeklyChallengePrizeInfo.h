// source/game/weekly_challenges/WeeklyChallengePrizeInfo.h
#ifndef SOURCE_GAME_WEEKLY_CHALLENGES_WEEKLYCHALLENGEPRIZEINFO_H
#define SOURCE_GAME_WEEKLY_CHALLENGES_WEEKLYCHALLENGEPRIZEINFO_H

class WeeklyChallengePrizeInfo {  // sizeof 0x4C
public:
    WeeklyChallengePrizeInfo();
    ~WeeklyChallengePrizeInfo();
    static void SerializeV1(jet::stream::IStreamW&, const WeeklyChallengePrizeInfo&);
    static bool DeserializeV1(jet::stream::IStream&, WeeklyChallengePrizeInfo&);
    bool operator==(const WeeklyChallengePrizeInfo&);
    jet::String m_missionDescription;  // 0x0
    int m_minRange;  // 0x4
    int m_maxRange;  // 0x8
    jet::String m_consolationPopupText;  // 0xC
    safe_enum<EPriceTypeDef,enum EPriceTypeDef::type> m_prizeType;  // 0x10
    jet::core::ProtectedStorage<int> m_prizeAmount;  // 0x14
    MissionId m_tracking_missionId;  // 0x40
    int m_tracking_missionNumber;  // 0x44
    TRACKING_MISSION_TYPE m_tracking_missionType;  // 0x48
};

#endif
