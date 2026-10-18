// libs/vox1.1/include/vox_mutex.h
// vox::Mutex (vox_mutex_pthread.cpp in the .so: a recursive pthread mutex) and vox::VoxThread (vox_thread_pthread.cpp: a pthread
// that runs a function(user data, user data) until it is asked to stop). Both sit on the C++11 threads, which are pthreads on the
// platforms the .so runs on.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MUTEX_H
#define LIBS_VOX1_1_INCLUDE_VOX_MUTEX_H

#include <atomic>
#include <mutex>
#include <thread>

namespace vox {

class Mutex {
public:
    Mutex() {}
    void Lock() { m_mutex.lock(); }
    void Unlock() { m_mutex.unlock(); }
    bool TryLock() { return m_mutex.try_lock(); }
private:
    Mutex(const Mutex&);
    Mutex &operator=(const Mutex&);
    std::recursive_mutex m_mutex;
};

class ScopedLock {
public:
    explicit ScopedLock(Mutex &m) : m_mutex(m) { m_mutex.Lock(); }
    ~ScopedLock() { m_mutex.Unlock(); }
private:
    ScopedLock(const ScopedLock&);
    ScopedLock &operator=(const ScopedLock&);
    Mutex &m_mutex;
};

class VoxThread {
public:
    typedef void (*Function)(void*, void*);
    // starts at once: calls fn(arg0, arg1) every `periodMs` milliseconds until Stop
    VoxThread(Function fn, void *arg0, void *arg1, int periodMs, const char *name);
    ~VoxThread();
    void Stop();
    const char *GetName() const { return m_name; }
private:
    void Run();
    Function m_function;
    void *m_arg0;
    void *m_arg1;
    int m_periodMs;
    const char *m_name;
    std::atomic<bool> m_running;
    std::thread m_thread;
};

}  // namespace vox

#endif
