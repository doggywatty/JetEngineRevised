// libs/glf/include/glf/core/semaphore.h
#ifndef LIBS_GLF_INCLUDE_GLF_CORE_SEMAPHORE_H
#define LIBS_GLF_INCLUDE_GLF_CORE_SEMAPHORE_H

namespace glf {
struct Semaphore : private glf::NonCopyable {  // sizeof 0x4
    Semaphore(unsigned, unsigned);
    ~Semaphore();
    int Wait();
    int TryWait();
    int Post();
    glf::Semaphore::Impl &GetImpl();
private:
    glf::Semaphore::Impl &m;  // 0x0
};
}

namespace jet::thread {
class Semaphore : private boost::noncopyable_::noncopyable {  // sizeof 0x24
public:
    void Wait(jet::thread::RecursiveMutex&);
    void Wait(jet::thread::Mutex&);
    void Notify();
    void NotifyAll();
private:
    tthread::condition_variable m_variable;  // 0x0
};
}

#endif
