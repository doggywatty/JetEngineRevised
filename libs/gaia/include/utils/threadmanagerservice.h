// libs/gaia/include/utils/threadmanagerservice.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_THREADMANAGERSERVICE_H
#define LIBS_GAIA_INCLUDE_UTILS_THREADMANAGERSERVICE_H

namespace gaia {
class ThreadManagerService {  // sizeof 0x30
public:
    ThreadManagerService(int);
    void AddRequest(gaia::GaiaRequest*);
    void AddRequest(gaia::AsyncRequestImpl*);
    bool CanStartRequest();
    void StartRequest();
    void Update();
    int GetNumberOfRunningThread();
    void CheckThreadsStatus();
    void CancelAllIdleRequests();
    int GetNumberOfRequest();
    void DiscardAllPendingRequests();
    virtual ~ThreadManagerService();
private:
    int m_NrMaxThreads;  // 0x4
    int m_NrRunningThreads;  // 0x8
    std::vector<gaia::ThreadManagerRequest *,std::allocator<gaia::ThreadManagerRequest *> > m_PendingRequests;  // 0xC
    glwebtools::Mutex m_mutex;  // 0x20
    glwebtools::Thread **m_runningThreads;  // 0x28
    gaia::ThreadManagerRequest **m_runningRequests;  // 0x2C
};
}

#endif
