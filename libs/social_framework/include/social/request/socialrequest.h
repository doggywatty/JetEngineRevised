// libs/social_framework/include/social/request/socialrequest.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_SOCIALREQUEST_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_REQUEST_SOCIALREQUEST_H

namespace social::request {
class SocialRequest : public social::RefCounted {  // sizeof 0x90
public:
    SocialRequest(const social::request::SocialRequest::CreationSettings&);
    virtual ~SocialRequest();
    void Init(glwebtools::GlWebTools*);
    glwebtools::UrlRequest &GetRequest();
    void UnregisterCallback();
    bool HasCallback();
    bool CallCallbackIfSet(const glwebtools::UrlResponse&, bool);
private:
    void Cancel();
    void PrepareRequest();
    bool IsAddedToScheduler();
    void SetScheduler(social::request::RequestScheduler*);
    bool IsInitialized();
    bool IsCancelled();
    social::request::EPriority GetPriority();
    bool m_cancelled;  // 0x8
    glwebtools::UrlRequest m_request;  // 0xC
    social::request::SocialRequest::CreationSettings m_settings;  // 0x14
    bool m_isInitialized;  // 0x88
    social::request::RequestScheduler *m_scheduler;  // 0x8C
};
}

namespace social::request {
class SocialRequestHandle {  // sizeof 0x8
    SocialRequestHandle(const social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock>&);
public:
    SocialRequestHandle();
    bool IsValid();
    social::request::Error Cancel(bool);
    social::request::Error Unregister();
    int GetPriority();
    bool IsCancelled();
    bool IsAddedToScheduler();
private:
    social::IntrusivePointer<social::request::SocialRequest,social::IntrusivePointerNoLock> m_request;  // 0x0
};
}

#endif
