// framework/include/connection/connectionpoller.h
#ifndef FRAMEWORK_INCLUDE_CONNECTION_CONNECTIONPOLLER_H
#define FRAMEWORK_INCLUDE_CONNECTION_CONNECTIONPOLLER_H

class ConnectionPoller {  // sizeof 0x10
public:
    ConnectionPoller();
    ~ConnectionPoller();
    static void TinyThreadEntryPoint(void*);
    void ThreadRun();
    void Stop();
    bool HasInternetConnection();
    void forceCheck();
private:
    ConnectionPoller::SConnection CheckConnection();
    bool m_stop;  // 0x0
    ConnectionPoller::SConnection m_connection;  // 0x1
    tthread::fast_mutex m_mutex;  // 0x4
    tthread::thread *m_thread;  // 0x8
    bool m_forceCheck;  // 0xC
};

#endif
