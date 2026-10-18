// libs/glsociallib/include/glwtmanager/glwtmanager.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_GLWTMANAGER_GLWTMANAGER_H
#define LIBS_GLSOCIALLIB_INCLUDE_GLWTMANAGER_GLWTMANAGER_H

namespace sociallib {
class GLWTManager {  // sizeof 0x48
public:
    GLWTManager();
    ~GLWTManager();
    void Update();
    bool StartRequest(sociallib::GLWTManager::ServiceRequest*);
    void UpdateRequestQueue();
    void CompleteRequest(char*, int);
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned&, bool);
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const int&, bool);
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    sociallib::GLWTManager::State GetState();
    bool IsInProgress();
    void CancelRequest();
    int GetUpdateResponse();
    int GetCallFunctionId();
    int GetCallSNSType();
    void SendRequest(int, sociallib::VKWebComponent*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    void SendRequest(int, void (int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool)*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    bool m_send_by_post;  // 0x0
    bool m_enabled;  // 0x1
    int m_updateResponse;  // 0x4
    long m_callStartTime;  // 0x8
    int m_callFunctionID;  // 0xC
    int m_callSNSType;  // 0x10
protected:
    sociallib::GLWTManager::State m_state;  // 0x14
    std::queue<sociallib::GLWTManager::ServiceRequest *,std::deque<sociallib::GLWTManager::ServiceRequest *,std::allocator<sociallib::GLWTManager::ServiceRequest *> > > m_requests;  // 0x18
    glwebtools::UrlConnection m_connection;  // 0x34
    glwebtools::GlWebTools *m_glwtInstance;  // 0x3C
    glwebtools::Mutex m_mutex;  // 0x40
};
}

#endif
