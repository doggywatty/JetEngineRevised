// libs/glot_lib/include/errormanager.h
#ifndef LIBS_GLOT_LIB_INCLUDE_ERRORMANAGER_H
#define LIBS_GLOT_LIB_INCLUDE_ERRORMANAGER_H

namespace glot {
class ErrorManager {  // sizeof 0x108
public:
    static glot::ErrorManager *GetInstance();
    static void DeleteInstance();
    bool LogToFileAndTCP(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, glot::TrackingLogLevelType, int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, );
    bool AddErrorEvent(glot::GlotErrorEventIdCodes, glot::GlotErrorLogLevel, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_fstream<char,std::char_traits<char> >&);
    void AddErrorEventNoReason(glot::GlotErrorEventIdCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    glot::TrackingErrorEvent *GetNextErrorEvent();
    bool UpdateErrorFile();
    void DeleteErrorEvent();
    bool SwitchFullEventErrorFileWith(const char*);
    void UpdateTCPConnection();
    void SetLogGlotStarted(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool HasErrorsInQueue();
    bool IsInitialize();
    void ResetNrOfAddedEvents();
private:
    bool OpenAndAllocateFile(bool);
    void InitializeErrorManager();
    bool AddBatchErrorEvent(glot::GlotErrorEventIdCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    ErrorManager();
    ~ErrorManager();
    static glot::ErrorManager *s_Instance;
    static bool s_IsInitialized;
    static unsigned s_numberOfEventsAdded;
    glot::ErrorTracker *m_ErrorTracker;  // 0x0
    glot::GlotEventWrapper *m_GEW;  // 0x4
    glot::TrackingManager *m_TM;  // 0x8
    std::list<glot::TrackingErrorEvent *,std::allocator<glot::TrackingErrorEvent *> > m_collectorErrorQueue;  // 0xC
    glwebtools::Mutex m_mutexErrorQueue;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_ErrorEventsFilename;  // 0x2C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_ErrorEventsFullFilename;  // 0x4C
    glwebtools::Mutex m_mutexErrorEventsFile;  // 0x6C
    _iobuf *m_ErrorEventsFile;  // 0x74
    std::basic_fstream<char,std::char_traits<char> > m_ErrorEventsFullFile;  // 0x78
};
}

#endif
