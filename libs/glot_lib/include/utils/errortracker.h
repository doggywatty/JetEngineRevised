// libs/glot_lib/include/utils/errortracker.h
#ifndef LIBS_GLOT_LIB_INCLUDE_UTILS_ERRORTRACKER_H
#define LIBS_GLOT_LIB_INCLUDE_UTILS_ERRORTRACKER_H

namespace glot {
class ErrorTracker {  // sizeof 0x68
public:
    static glot::ErrorTracker *GetInstance();
    static void DestroyInstance();
    bool InitializeErrorTracker();
    bool IsInitialized();
    bool IsLogingActive();
    bool IsTCPActive();
    void UpdateTCPConnection();
    bool LogToFileAndTCP(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, glot::TrackingLogLevelType, int, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetLogGlotStarted(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
private:
    void AddDefaultConfigurations(const char*);
    bool LoadConfiguration(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void CheckConfiguration();
    void LogGlotStarted();
    bool AreLogActivatingPresent();
    bool TryOpenLogFile();
    bool TryOpenTCPConnection();
    ErrorTracker();
    ~ErrorTracker();
    static glot::ErrorTracker *s_instance;
    bool m_isInitialized;  // 0x0
    _iobuf *m_glotLog;  // 0x4
    bool m_loggingActive;  // 0x8
    bool m_useTCP;  // 0x9
    bool m_useFile;  // 0xA
    Json::Value m_loggingConfig;  // 0x10
    bool m_IsSetLogGlotStarted;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_strLogGlotStarted;  // 0x24
    int m_typeLogGlotStarted;  // 0x44
    glot::TCPConnection *m_TCPConn;  // 0x48
    std::queue<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::deque<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > logsQueue;  // 0x4C
};
}

#endif
