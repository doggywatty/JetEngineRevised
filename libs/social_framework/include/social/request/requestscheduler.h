// libs/social_framework/include/social/request/requestscheduler.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_REQUESTSCHEDULER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_REQUESTSCHEDULER_H

namespace social::request {
class RequestScheduler {  // sizeof 0x7C
public:
    RequestScheduler(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, int);
    ~RequestScheduler();
    social::request::Error Init(const glwebtools::GlWebTools::CreationSettings&);
private:
    void AddRequest(social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock>);
public:
    social::request::Error AddRequest(const social::request::SocialRequestHandle&);
    void CancelAllRequests();
    void CancelCurrentRequest();
    void SetMaximumRequestRate(int);
    int GetPriority();
    int GetMaximumRequestRate();
    void Update(int);
    bool HasPendingRequest();
    social::request::EPriority GetCurrentRequestPrio();
    void RunOneRequest();
    float GetAverageRequestsPerMinute();
private:
    void CancelRequest(const social::request::SocialRequestHandle&, bool);
    void RegisterRequestTime();
    glwebtools::GlWebTools *m_glwt;  // 0x0
    social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock> m_currentRequest;  // 0x4
    std::list<social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock>,std::allocator<social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock> > > m_requests;  // 0xC
    int m_requestRate;  // 0x24
    int m_timeSinceLastRequest;  // 0x28
    int m_priority;  // 0x2C
    bool m_isInitialized;  // 0x30
    int m_elapsedTime;  // 0x34
    bool m_requestStarted;  // 0x38
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_name;  // 0x3C
    glwebtools::UrlConnection m_connection;  // 0x5C
    std::list<int,std::allocator<int> > m_lastRequestTimes;  // 0x64
    static const int s_maxRequestTimeEntries;
};
}

#endif
