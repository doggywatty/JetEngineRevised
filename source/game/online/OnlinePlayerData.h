// source/game/online/OnlinePlayerData.h
#ifndef SOURCE_GAME_ONLINE_ONLINEPLAYERDATA_H
#define SOURCE_GAME_ONLINE_ONLINEPLAYERDATA_H

class OnlinePlayerData : public Singleton<OnlinePlayerData>, public GameSaveable<OnlinePlayerSaveData> {  // sizeof 0xA4
public:
    OnlinePlayerData();
    virtual ~OnlinePlayerData();
    void Init();
    void Update(int);
    social::UserOsiris *GetSocialUser();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetUserId();
    SocialUtils *GetSocialUtils();
    bool IsLoggedIn();
    bool IsLoggedInToAnonymous();
    bool IsLoggedInFacebook();
    bool IsLoggedInGameCenter();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFacebookLogin();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetGameCenterLogin();
    void LoginToFacebook();
    void LogoutFromFacebook();
    void LoginToGameCenter();
    void LogoutFromGameCenter();
    bool IsPlayer(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    void SetNeverLoggedIn(bool);
    bool GetNeverLoggedIn();
    void RefreshOnlineData();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGuestName();
    void OnLoginSuccess();
    void CheckCloudDataConflicts(bool);
    virtual void ResetData(OnlinePlayerSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, OnlinePlayerSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, OnlinePlayerSaveData&);
    OnlineUser *GetOnlineUser();
    bool CanLogin();
    void PublishAchievementActivity(const jet::String&, const jet::String&, bool);
    void PublishBuyMinionCostumActivity(const jet::String&);
    void PublishBuyGoldBonusActivity(const jet::String&);
    void PublishChallengeActivity(const jet::String&);
    void PublishWeeklyMissionActivity();
    void PublishToFacebook(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetCheckingCloudConflict(bool);
    void OnSnsProfileLoaded(sociallib::ClientSNSEnum);
    bool IsCheckingCloudData();
    void LogOnlineStatus();
    jet::String TrimName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    void SyncAchievements();
    static void sOnGameCenterAchievementPublished(bool, void*);
    static bool sOnPublishToFacebook(void*, bool, social::OnlineEventData);
    static void sOnGameCenterAchievementReset(bool, void*);
    static bool OnSocialLoggedIn(void*, bool, social::OnlineEventData);
    void SerializeV1(jet::stream::IStreamW&, OnlinePlayerSaveData&);
    bool DeserializeV1(jet::stream::IStream&, OnlinePlayerSaveData&);
    static const int k_saveGameVersion;
    static const int k_nameMaxCharacters;
    social::UserOsiris *m_socialUser;  // 0x90
    SocialUtils *m_socialUtils;  // 0x94
    bool m_checkingCloudData;  // 0x98
    bool m_checkingCloudDataShowWaitingScreen;  // 0x99
    OnlineUser *m_onlineUser;  // 0x9C
    bool m_waitingForCloudConflictPopup;  // 0xA0
    bool m_isImportingProfile;  // 0xA1
    bool m_isImportingFriends;  // 0xA2
    static const int k_signatureV1;
};

#endif
