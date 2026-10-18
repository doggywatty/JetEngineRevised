// source/game/online/tracking/GameTrackingMgr.h
#ifndef SOURCE_GAME_ONLINE_TRACKING_GAMETRACKINGMGR_H
#define SOURCE_GAME_ONLINE_TRACKING_GAMETRACKINGMGR_H

class GameTrackingMgr : public Singleton<GameTrackingMgr> {  // sizeof 0x18
public:
    GameTrackingMgr();
    virtual ~GameTrackingMgr();
    void Update();
    void Init();
    void OnResume();
    void OnPause();
    void SendFirstTimeLaunchEvent();
    void SendLaunchGameEvent();
    void SendResumeGameEvent();
    void SendLaunchedGameFromPNEvent(TRACKING_PN_TYPE);
    void SendResumedGameFromPNEvent(TRACKING_PN_TYPE);
    void SendIAPItemPurchasedEvent(const jet::String&, const jet::String&, double, TRACKING_SCREEN, TRACKING_SCREEN, TRACKING_PURCHASE_TYPE);
    void SendClickOnIAPPacksEvent(TRACKING_BLOCKED_REASON, TRACKING_IAP_RESULT, const jet::String&, unsigned, const jet::String&, TRACKING_PURCHASE_TYPE, TRACKING_SCREEN, TRACKING_SCREEN);
    void SendConnectToFederationEvent(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SendConnectToSocialNetworkEvent(TRACKING_CONNECTION_STATE, TRACKING_FAIL_REASON, TRACKING_SOCIAL_NETWORK);
    void SendContentDownloadedEvent(unsigned, unsigned, TRACKING_DOWNLOAD_STATE);
    void SendCurrencySpentEvent(TRACKING_CURRENCY, TRACKING_ITEM_CATEGORY, unsigned, TRACKING_ITEM_NAME, unsigned, unsigned, unsigned, unsigned, TRACKING_SCREEN, TRACKING_SCREEN, TRACKING_SPEND_ACTION);
    void SendExternalDeviceControllerInteractionEvent(TRACKING_CONNECTION_STATE, TRACKING_CONTROLLER, unsigned);
    void SendFriendInteractionEvent(TRACKING_FRIEND_ACTION, TRACKING_FRIEND_TYPE, TRACKING_SOCIAL_NETWORK);
    void SendFriendListChangeEvent(unsigned, unsigned, unsigned, TRACKING_SOCIAL_NETWORK);
    void SendGetFreeCashEvent(TRACKING_PROVIDER);
    void SendGiftReceivedThroughCustomerCareToolEvent(TRACKING_CC_GIFT_TYPE, TRACKING_SOCIAL_NETWORK);
    void SendIAPPackPurchasedEvent(unsigned, const jet::String&, double, TRACKING_SCREEN, TRACKING_PURCHASE_TYPE, TRACKING_SCREEN);
    void SendInventoryStatusEvent(TRACKING_SOCIAL_NETWORK);
    void SendInviteToDownloadTheApplicationEvent(unsigned, TRACKING_SOCIAL_NETWORK);
    void SendLeaderboardStatusEvent();
    void SendUserDevice();
    void SendInternetLostEvent();
    void SendVisitScreenEvent(TRACKING_SCREEN, TRACKING_SCREEN);
    void SendMissionInteractionsEvent(MissionId, unsigned, TRACKING_MISSION_ACTION, TRACKING_MISSION_TYPE, int, int, int);
    void SendAchievementsUnlockedThroughGamePlayEvent(AchievementId, TRACKING_ACHIEVEMENT_ACTION, unsigned);
    void SendPickupItemEvent(unsigned);
    void SendMinigameInteractionEvent(TRACKING_EVENT_STATE, TRACKING_ITEM_NAME, float, int, int);
    void SendResetGameEvent();
    void SendSinglePlayerRaceReviveEvent();
    void SendTutorialInteractionEvent(TRACKING_TUTORIAL_ACTION, const TutorialId&);
    void SendLoadingTimesEvent(TRACKING_ACTION_NAME, unsigned);
    void SendSinglePlayerRacePlayedEvent(TRACKING_RESULT);
    static TRACKING_ITEM_NAME GetVehicleItemName(GameItemType);
    static TRACKING_ITEM_NAME GetMissionItemName(MissionId);
private:
    TRACKING_ITEM_NAME GetCurrentCostumeTrackingItemId();
    void SendLaunchEvent(TRACKING_LAUNCH_TYPE, TRACKING_PN_TYPE);
    void SendIAPEvent(TRACKING_EVENTS, const jet::String&, TRACKING_SCREEN, TRACKING_SCREEN);
    static TRACKING_ACHIEVEMENTS AchievementIdToTrackingId(AchievementId);
    static TRACKING_TUTORIAL_PROGRESS TutorialIdToTrackingId(const TutorialId&);
    bool InitTrackingManager();
    glot::TrackingManager *m_trackingManager;  // 0x4
    bool m_initialized;  // 0x8
    int m_timeBetweenSessions;  // 0xC
    int m_lastSessionTime;  // 0x10
    bool m_hasPendingResume;  // 0x14
};

#endif
