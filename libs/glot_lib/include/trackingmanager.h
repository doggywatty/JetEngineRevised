// libs/glot_lib/include/trackingmanager.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TRACKINGMANAGER_H
#define LIBS_GLOT_LIB_INCLUDE_TRACKINGMANAGER_H

namespace glot {
class TrackingManager {  // sizeof 0x258
public:
    static glot::TrackingManager *GetInstance();
    static void FreeInstance();
    int Initialize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, char*, unsigned);
    bool IsInitialized();
    glot::GlotAddEventReturn AddEvent(int, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue, glot::EventValue);
    int Update();
    void OnResume();
    void OnPause();
    int BlockEventSending();
    int UnblockEventSending();
    int TriggerImmediateSendingOfEvents();
    unsigned long GetTotalGameTime();
    const bool GetOfflineFlag();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetMAC();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetUDID();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetGLDID();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetGameVersion();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetGLOTRevision();
    long GetServerTimeSeconds();
    long GetServerTimeMillis();
    long GetSessionsLengthSeconds();
    long GetTimeBetweenSessionsMinutes();
    __int64 GetTotalDeviceUpTimeDetected();
    int GetCurrentSessionLaunchType();
    long GetDiffOfServerTimesSeconds();
    bool LogToFileAndTCP(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, glot::TrackingLogLevelType, int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, );
private:
    ~TrackingManager();
    TrackingManager();
    static bool CrmManagerCallback(void*, bool, Json::Value);
    static bool CrmManagerLogCallback(void*, bool, Json::Value);
    static void CallbackGAIAFinishRequest(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void CallbackFinishSendEvents(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void*);
    static int HandleErrorCodes(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool SaveTimeData();
    static bool LoadTimeData();
    static bool InitTimeData();
    static int UpdateTimeData(bool);
    static bool TryUpdateServerTime();
    void AddAutomaticEventsAndParamsIfMissing();
    int AttemptToAddEventToMessageFromCollectedEvents(glot::TrackingMessage*);
    int CrossCheckMessageIDWithServerID(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool CheckPrerequisitesForSending(bool);
    bool DetectCrashAndFirstLaunch(int);
    bool GenerateCustomBIAutomaticEvents(int, glot::TrackingEvent::eventType, int, int);
    const Json::Value GetEventInfo(const char*);
    unsigned long GetNextEventID();
    bool GetReadEventFileOpend();
    bool GetWriteEventFileOpend();
    void GlotLogToFileAndTCP(glot::GlotLogEventType, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, );
    int InitializeDeviceIdentifiers();
    int InitializeHTTPConnection();
    int InitializeLocalVariables();
    int InternalInitialization();
    int FlushBatchedEvents();
    int LoadCollectedBatchableEvents();
    int LoadOldLegacyEvents();
    void LogGlotStarted();
    int PrepareEventParameters(const char*, unsigned);
    glot::GlotAddEventReturn PushBatchedEvent(glot::TrackingEvent*, bool);
    int ReadOldLegacyEventData(_iobuf*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    int ReadStateMarkers();
    bool RequestPkgID();
    int SaveCollectedBatchableEvents();
    void SendErrorNotification(glot::GlotErrorEventIdCodes, glot::GlotErrorLogLevel, const char*, );
    bool SendPackage();
    void SetCurrentAutomaticParametersForBatchedEv(Json::Value&);
    bool SwitchDataFiles();
    int ValidateParameterConfiguration();
    int WriteStateMarkers();
    static glwebtools::Mutex m_mutexGetInstance;
    static glot::TrackingManager *s_instance;
    static glot::GlotState s_state;
    static unsigned s_retries;
    static long s_lastServerTime;
    static long s_lastSessionBreakTime;
    static long s_lastSessionLength;
    static long s_STSDiffSec;
    static long s_sessionStartServerTime;
    static long s_sessionSyncServerTime;
    static long s_miliSec;
    static __int64 s_lastUpTime;
    static __int64 s_lastSessionStart;
    static __int64 s_totalDeviceUpTimeDetected;
    static int s_launchType;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_cachedDeviceSavePath;
    bool m_isInitialized;  // 0x0
    int m_GGI;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_UDID;  // 0x8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_GLDID;  // 0x28
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_MAC;  // 0x48
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_gameversion;  // 0x68
    glot::TrackingConnection *m_connection;  // 0x88
    glot::GlotEventWrapper *m_eventWrapper;  // 0x8C
    std::basic_fstream<char,std::char_traits<char> > m_writeEventsFile;  // 0x90
    std::basic_fstream<char,std::char_traits<char> > m_readEventsFile;  // 0x120
    _iobuf *m_markersWriteFile;  // 0x1B0
    unsigned long m_lastEventId;  // 0x1B4
    bool m_pauseMarker;  // 0x1B8
    unsigned long m_totalGameTime;  // 0x1BC
    unsigned m_package_id;  // 0x1C0
    unsigned m_last_read_offset;  // 0x1C4
    unsigned m_last_sent_offset;  // 0x1C8
    bool m_blockSending;  // 0x1CC
    bool m_forceRecoverFiles;  // 0x1CD
    long m_internalTimer;  // 0x1D0
    bool m_isOfflineMode;  // 0x1D4
    Json::Value m_parameterMap;  // 0x1D8
    std::map<Json::Value,Json::Value,std::less<Json::Value>,std::allocator<std::pair<Json::Value const ,Json::Value> > > m_batchedEvents;  // 0x1E8
    glwebtools::Mutex m_mutexSwitchDataFiles;  // 0x204
    glwebtools::Mutex m_mutexNextEventID;  // 0x20C
    glwebtools::Mutex m_mutexMarkers;  // 0x214
    glwebtools::Mutex m_mutexBatchedEvents;  // 0x21C
    glwebtools::Mutex m_mutexSerializeFunctions;  // 0x224
    glot::TrackingMessage *m_message;  // 0x22C
    glot::ErrorManager *m_ErrorManager;  // 0x230
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_clientId;  // 0x234
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_eveTrackingLink;
    gaia::CrmManager *m_CrmManager;  // 0x254
    static glwebtools::Mutex m_mutexGAIAState;
    static int s_gaiaSTSRequestState;
};
}

#endif
