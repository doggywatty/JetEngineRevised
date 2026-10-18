// libs/social_framework/include/social/cache/cacherequest.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEREQUEST_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEREQUEST_H

namespace social::cache {
class CacheRequest : public social::RefCounted {  // sizeof 0x78
public:
    void Start();
    void Cancel();
    bool IsCancelled();
    const social::cache::CacheRequestResult &GetResult();
    social::cache::CacheRequestOp GetOp();
    const social::cache::CacheObjectData &GetData();
    bool operator==(const social::cache::CacheRequest&);
    virtual unsigned Grab();
    virtual unsigned Drop();
    CacheRequest(social::cache::CacheRequestManager&, unsigned, social::cache::CacheDepot&, const social::cache::CacheRequest::CreationSettings&);
    virtual ~CacheRequest();
    static int s_requestCount;
private:
    social::cache::CacheRequestManager &m_manager;  // 0x8
    social::cache::CacheRequestOp m_op;  // 0xC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_objectKey;  // 0x10
    unsigned m_id;  // 0x30
    social::cache::CacheRequestState m_state;  // 0x34
    social::cache::CacheDepot &m_depot;  // 0x38
    social::cache::CacheRequestResult m_result;  // 0x3C
    social::cache::CacheObjectData m_data;  // 0x40
    glwebtools::Mutex m_mutex;  // 0x6C
    bool m_isCancelled;  // 0x74
};
}

namespace social::cache {
class CacheRequestHandle {  // sizeof 0xC
    CacheRequestHandle(const social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock>&);
public:
    CacheRequestHandle(const social::cache::CacheRequestHandle&);
    CacheRequestHandle();
    social::cache::CacheRequestHandle &operator=(const social::cache::CacheRequestHandle&);
    ~CacheRequestHandle();
    bool IsValid();
    social::cache::CacheRequestState GetState();
    social::cache::CacheRequestResult GetResult();
    social::cache::CacheRequestOp GetOp();
    bool operator==(const social::cache::CacheRequestHandle&);
    bool operator==(const social::cache::CacheRequest&);
    bool operator!=(const social::cache::CacheRequestHandle&);
private:
    void Cancel();
    social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock> m_request;  // 0x0
};
}

#endif
