// libs/gaia/include/utils/threadmanager.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_THREADMANAGER_H
#define LIBS_GAIA_INCLUDE_UTILS_THREADMANAGER_H

namespace gaia {
class ThreadManager {  // sizeof 0x50
    ThreadManager(const gaia::ThreadManager&);
    ThreadManager();
    gaia::ThreadManager &operator=(const gaia::ThreadManager&);
    virtual ~ThreadManager();
    void ForceCloseAllThreads();
    void CancelRequest(gaia::OpCodes);
    int GetNumerOfRequest();
public:
    static gaia::ThreadManager *GetInstance();
    void Drop();
    void Grab();
    int pushTask(gaia::GaiaRequest*);
    int pushTask(gaia::AsyncRequestImpl*);
    void Update();
private:
    static glwebtools::Mutex s_mutex;
    static gaia::ThreadManager *s_instance;
    int m_refcount;  // 0x4
    int m_nrServices;  // 0x8
    glwebtools::Mutex m_mutex;  // 0xC
    gaia::ThreadManagerService *m_services[15];  // 0x14
};
}

#endif
