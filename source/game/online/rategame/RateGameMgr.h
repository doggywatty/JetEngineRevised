// source/game/online/rategame/RateGameMgr.h
#ifndef SOURCE_GAME_ONLINE_RATEGAME_RATEGAMEMGR_H
#define SOURCE_GAME_ONLINE_RATEGAME_RATEGAMEMGR_H

class RateGameMgr : public Singleton<RateGameMgr> {  // sizeof 0x34
public:
    RateGameMgr();
    virtual ~RateGameMgr();
    void Update();
    void ShowPopupNow();
    void Save(jet::stream::MemoryStream&);
    void Load(jet::stream::MemLinkStream&);
    void SetImportantEventHappened();
    void EnablePopup();
    void DisablePopup();
    void OnPopupLater();
    void OnPopupRateNow();
private:
    bool CanShowPopup();
    void ShowRateGamePopup();
    static const int k_saveGameVersion;
    bool m_popupEnabled;  // 0x4
    bool m_importantEventHappened;  // 0x5
    _RateGameSaveData m_data;  // 0x8
    game::common::TrackingSession *m_runSession;  // 0x30
    static const int k_firstShowWaitTime;
    static const int k_retryIntervalSeconds;
    static const int k_newVersionWaitTimeSeconds;
    static const double k_newGameWaitTimeMs;
    static const int k_maxRateRetries;
};

#endif
