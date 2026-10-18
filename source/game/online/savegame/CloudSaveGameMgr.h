// source/game/online/savegame/CloudSaveGameMgr.h
#ifndef SOURCE_GAME_ONLINE_SAVEGAME_CLOUDSAVEGAMEMGR_H
#define SOURCE_GAME_ONLINE_SAVEGAME_CLOUDSAVEGAMEMGR_H

class CloudSaveGameMgr : public Singleton<CloudSaveGameMgr> {  // sizeof 0xC0
public:
    CloudSaveGameMgr();
    virtual ~CloudSaveGameMgr();
    bool Init();
    void Update(int);
    bool IsReady();
    CloudSaveGameMgr::ECloudSaveStatus GetStatus();
    void SetCredentialTypeUsedForBackup(gaia::BaseServiceManager::Credentials);
    void SaveGame(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    bool LoadGames();
    safe_enum<CloudSaveGameMgr::ReturnValue,enum CloudSaveGameMgr::ReturnValue::type> GetRestoredGamesStatus();
    std::list<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> > GetAvailableSaves();
    void RestoreGame(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    safe_enum<CloudSaveGameMgr::ReturnValue,enum CloudSaveGameMgr::ReturnValue::type> IsDataLoadedFromCloud();
    bool ApplyCloudSaveToGame();
    bool IsSaveFromOtherDevice();
    bool IsSaveAvailable();
    void SetSaveAvailable(bool);
    void Save(jet::stream::MemoryStream&);
    void Load(jet::stream::MemLinkStream&);
    void ReScheduleNextAutosave();
private:
    bool LoadGameFromCloud();
    bool LoadGameFromFile();
    void ScheduleNextAutoSave();
    static void sGLRequestCompleted(savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*);
    int m_timeSinceLastSave;  // 0x4
    int m_timeSinceLastCCCheck;  // 0x8
    savemanager::SaveGameManager *m_saveManager;  // 0xC
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_messageResponse;  // 0x10
    std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> > m_restoredSaves;  // 0x24
    safe_enum<CloudSaveGameMgr::ReturnValue,enum CloudSaveGameMgr::ReturnValue::type> m_restoredSavesAvailable;  // 0x38
    safe_enum<CloudSaveGameMgr::ReturnValue,enum CloudSaveGameMgr::ReturnValue::type> m_isDataReady;  // 0x3C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_saveToRestore;  // 0x40
    bool m_otherDeviceSaveFound;  // 0x60
    bool m_saveGameAfterRestore;  // 0x61
    savemanager::CloudSave *m_otherDeviceSaveCandidate;  // 0x64
    CloudSaveGameMgr::ECloudSaveStatus m_cloudStatus;  // 0x68
    CloudSaveGameMgr::ECloudSaveStatus m_previousCloudStatus;  // 0x6C
    int m_lastCloudSaveTimestamp;  // 0x70
    bool m_saveAvailable;  // 0x74
    Date m_next24hSave;  // 0x78
    bool m_hasPendingSave;  // 0x94
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_pendingDescription;  // 0x98
    int m_numRetries;  // 0xB8
    static const int k_maxRetries;
    static const int k_minimumTimeBetweenSaves;
    static const int k_minimumTimeBetweenCCChecks;
    static const int k_autoSaveTimeSecs;
    static const int k_autoSaveTimeCheatSecs;
    static const jet::String k_saveRecordName;
    bool m_hasPendingRestore;  // 0xBC
};

#endif
