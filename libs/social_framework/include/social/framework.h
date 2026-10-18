// libs/social_framework/include/social/framework.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_FRAMEWORK_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_FRAMEWORK_H

namespace social {
class Framework {  // sizeof 0x1
public:
    static void Init(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    static void Init(const social::Framework::InitializationSettings&);
    static void PlatformInit();
    static void Finish();
    static void Update(int);
    static void UpdateConnections();
    static void UpdateError();
    static bool IsInitialized();
    static gaia::Gaia_Osiris *GetOsiris();
    static gaia::Gaia_Seshat *GetSeshat();
    static gaia::Gaia_Hermes *GetHermes();
    static gaia::Gaia *GetGaia();
    static gaia::Gaia_Kairos *GetKairos();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetClientId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPID();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGGI();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetVersion();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetIGPGameCode();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPlatformId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetStoreType();
    static social::cache::CacheManager *GetCacheManager();
    static social::downloadable::DownloadablesManager *GetDownloadableManager();
    static void SetStatus(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool IsServiceOk();
    static bool IsSNSReady(sociallib::ClientSNSEnum);
    static bool IsConnectedToInternet();
    static long GetServerTime();
    static void OnResume();
    static void OnPlatformResume();
    static void OnSuspend();
    static void OnPlatformSuspend();
    static void OnShutDown();
    static bool HandleOpenUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetDebugString();
    static bool IsInView();
    static void SetConnected(bool);
    static void InitGaia();
    static const std::vector<enum sociallib::ClientSNSEnum,std::allocator<enum sociallib::ClientSNSEnum> > &AvailableSNS();
    static bool IsInErrorState();
    static bool IsBeta();
    static void NotifyGaiaInit();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetAppName();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGameName();
    static social::ELang GetLang();
    static void SetLang(social::ELang);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetCachePath();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetPortalUrl();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetResourcesUrl();
private:
    static void OnGaiaInitialized(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void LoginToAnonymous();
    static void InitSNSs();
    static bool OnSNSManagerLoggedIn(void*, bool, sociallib::ClientSNSEnum);
    static bool OnSNSInitialized(void*, bool, sociallib::ClientSNSEnum);
    static void OnServerTimeStamp(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void OnServerTimeStampRefresh(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void CreateDebugString();
    static void AutoLogin();
    static bool sOnSNSLoggedIn(void*, bool, social::OnlineEventData);
    static void LoginToOsirisWithAvailableSNS();
    static bool OnOsirisLoggedInToAnonymous(void*, bool, social::OnlineEventData);
    static bool sOnGameCenterLoggedIn(void*, bool, sociallib::ClientSNSEnum);
    static bool sOnGameCenterLoggedOut(void*, bool, sociallib::ClientSNSEnum);
    static void PerformAutoLogin();
    static void InitCache();
public:
    static const int k_maxItemsPerRequest;
    static social::Framework::SDebug DebugParams;
private:
    static std::map<enum sociallib::ClientSNSEnum,enum social::Framework::ESNSStatus,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,enum social::Framework::ESNSStatus> > > s_availableSns;
    static gaia::Gaia *s_gaia;
    static social::Framework::EInitializationSteps s_initialized;
    static int s_errorCount;
    static const int k_MaxErrors;
    static int s_ellapsedErrorTime;
    static const int k_errorRetryTime;
    static gaia::Gaia_Osiris *s_osiris;
    static gaia::Gaia_Seshat *s_seshat;
    static gaia::Gaia_Hermes *s_hermes;
    static gaia::Gaia_Kairos *s_kairos;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_serviceStatus;
    static bool s_autoLogin;
    static bool s_initializingGameCenter;
    static bool s_connectedToInternet;
    static long s_serverTimeStamp;
    static long s_timeDifference;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_pid;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_ggi;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_version;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_clientId;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_igpGameCode;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_appName;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_gameName;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_platformId;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_storeType;
    static bool s_isBeta;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_debugString;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_debugStatus;
    static social::ELang s_lang;
    static bool s_allSnsInit;
    static bool s_gaiaInitialized;
    static std::vector<enum sociallib::ClientSNSEnum,std::allocator<enum sociallib::ClientSNSEnum> > s_pendingSnsToAutologin;
    static bool s_loggedInToAnonymous;
    static social::cache::CacheManager *s_cacheManager;
    static social::downloadable::DownloadablesManager *s_downloadablesManager;
};
}

#endif
