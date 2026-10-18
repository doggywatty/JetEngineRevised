// libs/social_framework/include/social/cache/cacherequestmanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEREQUESTMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEREQUESTMANAGER_H

namespace social::cache {
class CacheRequestManager {  // sizeof 0x28
public:
    CacheRequestManager(const social::cache::CacheRequestRunner::CreationSettings&);
    CacheRequestManager();
    ~CacheRequestManager();
    void Init();
    social::cache::CacheRequestHandle PushRequest(social::cache::CacheDepot&, social::cache::CacheRequest::CreationSettings&);
    void Update();
private:
    void CompleteRequest(social::cache::CacheRequest&);
    void RemoveRequest(social::cache::CacheRequest&);
    std::map<unsigned int,social::cache::CacheRequest *,std::less<unsigned int>,std::allocator<std::pair<unsigned int const ,social::cache::CacheRequest *> > > m_requests;  // 0x0
    unsigned m_lastRequestId;  // 0x1C
    social::cache::CacheRequestRunner *m_runner;  // 0x20
    bool m_isInitialized;  // 0x24
};
}

namespace social::cache {
class CacheRequestRunner {  // sizeof 0x68
public:
    CacheRequestRunner(const social::cache::CacheRequestRunner::CreationSettings&);
    CacheRequestRunner();
    ~CacheRequestRunner();
    void Start();
    void Stop();
    void AddRequest(const social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock>&);
    bool GetCompletedRequest(social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock>&);
private:
    void RunRequests();
    void RunRequest(social::cache::CacheRequest&);
    static void RunStatic(void*, void*);
    static void StoreData(social::cache::CacheRequest&);
    static void LoadData(social::cache::CacheRequest&);
    void Erase(social::cache::CacheRequest&);
    social::SyncQueue<social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock> > m_syncQueueToRun;  // 0x0
    social::SyncQueue<social::IntrusivePointer<social::cache::CacheRequest,social::IntrusivePointerMutexLock> > m_syncQueueDone;  // 0x2C
    glwebtools::Thread *m_thread;  // 0x58
    social::cache::CacheRequestRunner::CreationSettings m_creationSettings;  // 0x5C
    bool m_isRunning;  // 0x64
};
}

#endif
