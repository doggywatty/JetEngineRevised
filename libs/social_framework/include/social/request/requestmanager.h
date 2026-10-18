// libs/social_framework/include/social/request/requestmanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_REQUESTMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_REQUESTMANAGER_H

namespace social::request {
class RequestManager : public social::SSingleton<social::request::RequestManager> {  // sizeof 0x28
public:
    RequestManager(const social::request::RequestManager::CreationSettings&);
    virtual ~RequestManager();
    static social::request::SocialRequestHandle CreateRequest(const social::request::SocialRequest::CreationSettings&);
    void Init();
    void Update(int);
    bool HasScheduler(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::request::RequestScheduler *GetScheduler(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::request::Error CreateScheduler(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, int, const glwebtools::GlWebTools::CreationSettings&);
    void AddRequestToScheduler(social::request::SocialRequestHandle&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void AddRequestToScheduler(social::request::SocialRequestHandle&, social::request::RequestScheduler&);
    void SetTargetRequestsPerMinute(float);
private:
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::request::RequestScheduler *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::request::RequestScheduler *> > > m_schedulers;  // 0x4
    float m_currentShare;  // 0x20
    social::request::RequestManager::CreationSettings m_settings;  // 0x24
};
}

#endif
