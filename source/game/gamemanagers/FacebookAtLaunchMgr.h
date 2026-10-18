// source/game/gamemanagers/FacebookAtLaunchMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_FACEBOOKATLAUNCHMGR_H
#define SOURCE_GAME_GAMEMANAGERS_FACEBOOKATLAUNCHMGR_H

class FacebookAtLaunchMgr : public Singleton<FacebookAtLaunchMgr>, public GameSaveable<FacebookAtLaunchMgrSaveData> {  // sizeof 0x58
public:
    FacebookAtLaunchMgr();
    virtual ~FacebookAtLaunchMgr();
    virtual void ResetData(FacebookAtLaunchMgrSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, FacebookAtLaunchMgrSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, FacebookAtLaunchMgrSaveData&);
    void HasBeenLaunched();
    bool ForceToRecalculateIfCanBeDisplayed();
    bool HasToBeDisplayed();
    void NeverDisplayAgain();
    void GetReward();
private:
    void SerializeV1(jet::stream::IStreamW&, FacebookAtLaunchMgrSaveData&);
    bool DeserializeV1(jet::stream::IStream&, FacebookAtLaunchMgrSaveData&);
    void SetNextTimeToDisplay();
public:
    static const jet::String k_saveRecordName;
    static const unsigned k_maxTimesDisplayed;
    static const unsigned k_rewardFacebook;
    static const unsigned k_oneDay;
    static const unsigned k_everyXDays;
private:
    static const int k_saveGameVersion;
    static const int k_signatureV1;
};

#endif
