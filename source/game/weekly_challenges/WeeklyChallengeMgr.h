// source/game/weekly_challenges/WeeklyChallengeMgr.h
#ifndef SOURCE_GAME_WEEKLY_CHALLENGES_WEEKLYCHALLENGEMGR_H
#define SOURCE_GAME_WEEKLY_CHALLENGES_WEEKLYCHALLENGEMGR_H

class WeeklyChallengeMgr : public Singleton<WeeklyChallengeMgr>, public GameSaveable<WeeklyChallengeMgrSaveData> {  // sizeof 0x8C
public:
    WeeklyChallengeMgr();
    virtual ~WeeklyChallengeMgr();
    void Reset();
    virtual void ResetData(WeeklyChallengeMgrSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, WeeklyChallengeMgrSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, WeeklyChallengeMgrSaveData&);
    void SetWeeklyMissionData(social::Event*);
    const WeeklyChallengeInfo &GetWeeklyChallengeData();
    void UpdateWeeklyChallengeScore();
    void OnCompleted(WeeklyMissionConsolation*);
    void OnLoadingStarted();
    jet::String GetPrizeRangeText(int, int, int);
    void Prizes_AddPrize(GameMessageGift*);
    bool Prizes_HasPopupsToShow();
    void Prizes_TryShowPopups();
    void Prizes_RemovePrizeEntry(const WeeklyChallengePrizeInfo&);
    WeeklyMission *GetCurrentWeeklyMission();
    bool WasCompleted(const WeeklyMissionConsolation*);
    void MarkUserHasSeenTheNewContest();
    bool HasUserSeenTheNewContest();
private:
    void SerializeV1(jet::stream::IStreamW&, WeeklyChallengeMgrSaveData&);
    bool DeserializeV1(jet::stream::IStream&, WeeklyChallengeMgrSaveData&);
    void TryRestartingWeeklyMission();
    TRACKING_MISSION_TYPE GetConsolationPrizeTrackingId(WeeklyMissionConsolation*);
    TRACKING_MISSION_TYPE GetPrizeTrackingId(bool, int);
public:
    static const jet::String k_saveRecordName;
private:
    static const int k_saveGameVersion;
    static const int k_signatureV1;
    bool m_areEntitiesLoaded;  // 0x88
};

#endif
