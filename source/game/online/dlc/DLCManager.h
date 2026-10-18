// source/game/online/dlc/DLCManager.h
#ifndef SOURCE_GAME_ONLINE_DLC_DLCMANAGER_H
#define SOURCE_GAME_ONLINE_DLC_DLCMANAGER_H

class DLCManager : public Singleton<DLCManager> {  // sizeof 0x360
public:
    DLCManager(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~DLCManager();
    void Update();
    void NotifyGaiaInitialization();
    bool RequestAssetOnDemand(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::vector<manhattan::dlc::AssetFeedback,std::allocator<manhattan::dlc::AssetFeedback> > GetFeedback();
    bool IsAllMandatoryReady();
    bool IsAllIndexedNonMandatoryReady();
    bool IsAllOnDemandReady();
    const std::vector<DLCManager::TroubleDescription,std::allocator<DLCManager::TroubleDescription> > GetTroublesomeOperations();
    static bool IsInstantiated();
    double GetProgress();
    double GetProgress(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    double GetProgress(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    manhattan::dlc::Downloader m_downloaderBackGround;  // 0x4
    manhattan::dlc::Downloader m_downloaderOnDemand;  // 0x2C
    manhattan::dlc::AssetMgr2 m_assetMgr;  // 0x58
    bool m_gaiaInitialized;  // 0x358
};

#endif
