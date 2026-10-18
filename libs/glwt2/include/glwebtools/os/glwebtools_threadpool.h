// libs/glwt2/include/glwebtools/os/glwebtools_threadpool.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_OS_GLWEBTOOLS_THREADPOOL_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_OS_GLWEBTOOLS_THREADPOOL_H

namespace glwebtools {
struct Job {  // sizeof 0xC
    Job();
    void Reset();
    bool IsSet();
    void operator()();
    void (*m_userUpdateFunc)(void*, void*);  // 0x0
    void* m_userCaller;  // 0x4
    void* m_userParam;  // 0x8
};
}

namespace glwebtools {
class JobRunner {  // sizeof 0x40
public:
    JobRunner(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    ~JobRunner();
    int Start(glwebtools::Thread::Priority);
    int Join();
    int Signal(const glwebtools::Job&);
    bool IsIdle();
    bool IsWaiting();
    bool IsRunning();
    bool IsEnding();
    const char *GetName();
    int Update(unsigned __int64);
    bool IsIdleForMoreThan(unsigned __int64);
private:
    static void RunStatic(void*, void*);
    void Run();
    glwebtools::Job m_localJob;  // 0x0
    glwebtools::Thread m_thread;  // 0xC
    volatile glwebtools::JobRunner::State m_state;  // 0x24
    glwebtools::Mutex m_mutex;  // 0x28
    glwebtools::Condition m_condition;  // 0x30
    unsigned __int64 m_idle_time_ms;  // 0x38
};
}

namespace glwebtools {
class ThreadPool {  // sizeof 0x40
public:
    ThreadPool();
    ~ThreadPool();
    int Initialize(const glwebtools::ThreadPool::CreationSettings&);
    void Terminate();
    bool IsInitialized();
    int Signal(const glwebtools::Job&);
    unsigned AvailableThreadCount();
    unsigned WaitingThreadCount();
    unsigned StartedThreadCount();
    unsigned Size();
    bool Idle();
    int Update(unsigned __int64, bool);
private:
    int AddThread();
    std::list<glwebtools::JobRunner *,std::allocator<glwebtools::JobRunner *> > m_pool;  // 0x0
    glwebtools::ThreadPool::CreationSettings m_creationSettings;  // 0x18
    unsigned __int64 m_accumulator_ms;  // 0x30
    unsigned m_usedThreads;  // 0x38
};
}

#endif
