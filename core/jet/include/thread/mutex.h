// core/jet/include/thread/mutex.h
#ifndef CORE_JET_INCLUDE_THREAD_MUTEX_H
#define CORE_JET_INCLUDE_THREAD_MUTEX_H

#include <mutex>

namespace jet {
namespace thread {

class Mutex {
public:
    void Lock() { m.lock(); }
    void Unlock() { m.unlock(); }
    bool TryLock() { return m.try_lock(); }

private:
    std::mutex m;
};

class RecursiveMutex {
public:
    void Lock() { m.lock(); ++m_count; }
    void Unlock() { --m_count; m.unlock(); }
    bool TryLock()
    {
        if (!m.try_lock())
            return false;
        ++m_count;
        return true;
    }
    int GetLockCount() const { return m_count; }

private:
    std::recursive_mutex m;
    int m_count = 0;
};

class ScopedMutex {
public:
    explicit ScopedMutex(Mutex& m) : m_locked(true), m_mutex(&m), m_recursive(nullptr) { m.Lock(); }
    explicit ScopedMutex(RecursiveMutex& m) : m_locked(true), m_mutex(nullptr), m_recursive(&m) { m.Lock(); }
    ~ScopedMutex() { Unlock(); }
    ScopedMutex(const ScopedMutex&) = delete;
    ScopedMutex& operator=(const ScopedMutex&) = delete;

    void Unlock()
    {
        if (!m_locked)
            return;
        if (m_mutex)
            m_mutex->Unlock();
        if (m_recursive)
            m_recursive->Unlock();
        m_locked = false;
    }

private:
    bool m_locked;
    Mutex* m_mutex;
    RecursiveMutex* m_recursive;
};

}
}

#endif
