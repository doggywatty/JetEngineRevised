// libs/glot_lib/include/protocolbuffers/gloteventswrapper.h
#ifndef LIBS_GLOT_LIB_INCLUDE_PROTOCOLBUFFERS_GLOTEVENTSWRAPPER_H
#define LIBS_GLOT_LIB_INCLUDE_PROTOCOLBUFFERS_GLOTEVENTSWRAPPER_H

namespace glot {
class GlotEventWrapper {  // sizeof 0x48
public:
    static glot::GlotEventWrapper *GetInstance();
    static void DestroyInstance();
    bool SerializePBErrorEvent(glot::GlotErrorEventIdCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_fstream<char,std::char_traits<char> >&, unsigned long);
    bool SerializePBEvent(int, Json::Value*, std::basic_fstream<char,std::char_traits<char> >&);
    bool SerializePBEvent(int, int, glot::EventValue*, std::basic_fstream<char,std::char_traits<char> >&);
    bool DeserializePBEvent(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_fstream<char,std::char_traits<char> >&);
    void SetCurrentFileOffset(int);
    int GetCurrentFileOffset();
    void SetFileReadError(bool);
    bool GetFileReadError();
    void SetReadEOFMarker(bool);
    bool GetReadEOFMarker();
    Json::Value &GetJsonEvent();
    void SetLastToken(int);
private:
    GlotEventWrapper();
    virtual ~GlotEventWrapper();
    void SetJsonAsErrorEvent(glot::GlotErrorEventIdCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, );
    static glot::GlotEventWrapper *s_instance;
    glot::TrackingManager *m_tracking;  // 0x8
    glwebtools::Mutex m_MutexAddEvent;  // 0xC
    glwebtools::Mutex m_MutexJsonEvent;  // 0x14
    glwebtools::Mutex m_MutexDeserializeEvent;  // 0x1C
    int m_FileOffset;  // 0x24
    bool m_FileReadError;  // 0x28
    bool m_FileReadMarkEOF;  // 0x29
    Json::Value m_JsonEvent;  // 0x30
    int m_lastTokenRead;  // 0x40
};
}

#endif
