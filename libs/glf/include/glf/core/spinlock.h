// libs/glf/include/glf/core/spinlock.h
#ifndef LIBS_GLF_INCLUDE_GLF_CORE_SPINLOCK_H
#define LIBS_GLF_INCLUDE_GLF_CORE_SPINLOCK_H

namespace boost::detail {
class spinlock {  // sizeof 0x4
public:
    bool try_lock();
    void lock();
    void unlock();
    long v_;  // 0x0
};
}

namespace glf {
struct SpinLock : private glf::NonCopyable {  // sizeof 0x4
    SpinLock();
    ~SpinLock();
    void Lock();
    bool TryLock();
    void Unlock();
    glf::SpinLock::Impl &GetImpl();
private:
    glf::SpinLock::Impl &m;  // 0x0
};
}

#endif
