// libs/glot_lib/include/trackingconnection.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TRACKINGCONNECTION_H
#define LIBS_GLOT_LIB_INCLUDE_TRACKINGCONNECTION_H

namespace glot {
class TrackingConnection {  // sizeof 0x38
public:
    TrackingConnection();
    ~TrackingConnection();
    bool Init(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void (const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void*)*, void*);
    bool IsInitialized();
    void Update();
    bool sendData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    bool tryGetGLWebToolsInstance();
    void CompleteRequest(const glwebtools::UrlResponse&);
    glwebtools::GlWebTools *m_glwebtools;  // 0x0
    glwebtools::UrlConnection m_connection;  // 0x4
private:
    void* m_caller;  // 0xC
    void (*m_callback)(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void*);  // 0x10
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_server_url;  // 0x14
    bool m_isInitialized;  // 0x34
};
}

#endif
