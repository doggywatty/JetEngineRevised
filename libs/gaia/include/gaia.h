// libs/gaia/include/gaia.h
#ifndef LIBS_GAIA_INCLUDE_GAIA_H
#define LIBS_GAIA_INCLUDE_GAIA_H

namespace gaia {
class Gaia {  // sizeof 0x41C
public:
    gaia::BaseServiceManager *getService(int);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > getVersionNumber(gaia::Gaia::ServiceVersion);
    gaia::Gaia_Janus *GetJanusInstance();
    gaia::Gaia_Pandora *GetPandoraInstance();
    gaia::Gaia_Seshat *GetSeshatInstance();
    gaia::Gaia_Hermes *GetHermesInstance();
    gaia::Gaia_Olympus *GetOlympusInstance();
    gaia::Gaia_Osiris *GetOsirisInstance();
    gaia::Gaia_Iris *GetIrisInstance();
    gaia::Gaia_Hestia *GetHestiaInstance();
    gaia::Gaia_Kairos *GetKairosInstance();
    Gaia();
    ~Gaia();
    int Initialize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    bool IsLoggedIn(gaia::BaseServiceManager::Credentials);
    void Logout(gaia::BaseServiceManager::Credentials);
    int Login(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ResetUserToPerformActionsFor();
    int SetUserToPerformActionsFor(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetServiceUrl(gaia::GaiaRequest&);
    int GetServiceUrl(const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetServiceUrl(gaia::BaseServiceManager::Credentials, const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetVersionUpdateStatus();
    int Authorize(gaia::GaiaRequest);
    int Authorize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    static int StartWorkerThread(gaia::GaiaRequest, const char*);
    int CheckIfAccountExists(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SendAuthentificate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CreateAccount(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ChangeCredentialPassword(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ChangeCredentialContactDetails(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int FindUserByAlias(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddAlias(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SetDeviceInfo(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrievePassword(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddCredential(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddPermission(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int EncryptToken(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetMinimalFedAccountList(std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetCredentialListForAccount(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RefreshAccessToken(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int getServerTimeStamp(long&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    virtual bool CanDelete();
    static gaia::Gaia *GetInstance();
    static bool DestroyInstance();
    static bool IsInitialized();
    unsigned Grab();
    unsigned Drop();
    gaia::GLUID GetGLUID();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetJanusToken(gaia::BaseServiceManager::Credentials);
    int GetCredentialDetails(gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::CredentialDetail, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetInitializationAndLoginStatus(gaia::BaseServiceManager::Credentials);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetClientID();
    int GetNumberOfRequest();
    static void CompleteAsyncAction(gaia::GaiaRequest*);
    static void CompleteAsyncAction(gaia::AsyncRequestImpl*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const int&);
    static void TriggerAllCallbacks();
    static glwebtools::GlWebTools *GetGLWTInstance();
private:
    bool InitGLUID();
    void Update();
    bool Shutdown();
    void DeleteThread();
    void CancelAllRequests();
    glwebtools::Mutex &GetServiceMutex(int);
    int GetJanusStatus();
    int StartAndAuthorizeJanus(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static void UpdateStatic(void*, void*);
    static void callbackRequestCompleted(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void PerformAsyncAction(void*, void*);
    static void PerformAsyncActionDataContainerPtr(void*, void*);
    gaia::BaseServiceManager *m_vpBaseServices[15];  // 0x4
    const char *m_vpBaseServicesName[15];  // 0x40
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_serviceUrl[15];  // 0x7C
    glwebtools::Mutex m_serviceMutex;  // 0x25C
public:
    static glwebtools::Mutex s_CallbackRequestMutex;
    static std::vector<gaia::GaiaRequestCallback *,std::allocator<gaia::GaiaRequestCallback *> > s_CallbackRequest;
private:
    static gaia::Gaia *s_instance;
    int m_refCount;  // 0x264
    static bool s_IsInitialized;
    static bool s_IsInitializing;
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_currentClientID;  // 0x268
    gaia::GLUID m_GLUID;  // 0x288
    gaia::DeviceInfo m_devInfo;  // 0x2B8
    gaia::Gaia::LoginCredentials_struct m_anonymousCredentials;  // 0x358
    std::map<enum gaia::BaseServiceManager::Credentials,gaia::Gaia::LoginCredentials_struct,std::less<enum gaia::BaseServiceManager::Credentials>,std::allocator<std::pair<enum gaia::BaseServiceManager::Credentials const ,gaia::Gaia::LoginCredentials_struct> > > m_IdentifiedCredentials;  // 0x39C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_userToImpersonateUsername;  // 0x3B8
    gaia::BaseServiceManager::Credentials m_userToImpersonateCredential;  // 0x3D8
    glwebtools::Mutex m_mutex;  // 0x3DC
    glwebtools::Mutex m_serviceInitMutex;  // 0x3E4
    glwebtools::Thread::Priority m_updateThreadPriority;  // 0x3EC
    glwebtools::Thread *m_thread;  // 0x3F0
    bool m_isRunning;  // 0x3F4
    bool m_isThreadedUpdate;  // 0x3F5
    static unsigned m_threadedUpdatePeriod;
    static glwebtools::GlWebTools *s_glwtInstance;
    gaia::Gaia_Pandora *m_Pandora;  // 0x3F8
    gaia::Gaia_Janus *m_Janus;  // 0x3FC
    gaia::Gaia_Seshat *m_Seshat;  // 0x400
    gaia::Gaia_Hermes *m_Hermes;  // 0x404
    gaia::Gaia_Olympus *m_Olympus;  // 0x408
    gaia::Gaia_Osiris *m_Osiris;  // 0x40C
    gaia::Gaia_Iris *m_Iris;  // 0x410
    gaia::Gaia_Hestia *m_Hestia;  // 0x414
    gaia::Gaia_Kairos *m_Kairos;  // 0x418
};
}

#endif
