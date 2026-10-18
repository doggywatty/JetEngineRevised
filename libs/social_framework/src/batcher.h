// libs/social_framework/src/batcher.h
#ifndef LIBS_SOCIAL_FRAMEWORK_SRC_BATCHER_H
#define LIBS_SOCIAL_FRAMEWORK_SRC_BATCHER_H

namespace social {
class Batcher {  // sizeof 0x68
public:
    Batcher();
    virtual ~Batcher();
    void Update();
    virtual bool CanBeSaveBatched(social::Storable*) = 0;
    virtual bool CanBeMerged(const social::BatchLoadRequest&, social::Loadable*) = 0;
    virtual bool CanBeMerged(const social::BatchSaveRequest&, social::Storable*) = 0;
    void BatchStorable(social::Storable*);
    virtual bool CanBeBatched(social::Loadable*) = 0;
    void BatchLoadable(social::Loadable*);
protected:
    virtual void PerformRequest(const social::BatchSaveRequest&) = 0;
    virtual void PerformRequest(const social::BatchLoadRequest&) = 0;
    void OnRequestFinished();
    const social::BatchLoadRequest &GetCurrentRequest();
    const social::BatchSaveRequest &GetCurrentSaveRequest();
private:
    std::deque<social::BatchLoadRequest,std::allocator<social::BatchLoadRequest> > m_loadRequests;  // 0x4
    std::deque<social::BatchSaveRequest,std::allocator<social::BatchSaveRequest> > m_saveRequests;  // 0x20
    social::BatchLoadRequest m_currentRequest;  // 0x3C
    social::BatchSaveRequest m_currentSaveRequest;  // 0x50
    bool m_executingRequest;  // 0x64
};
}

#endif
