// core/jet/include/thread/localstorage.h
#ifndef CORE_JET_INCLUDE_THREAD_LOCALSTORAGE_H
#define CORE_JET_INCLUDE_THREAD_LOCALSTORAGE_H

#include <map>
#include <mutex>
#include <thread>

namespace jet {
namespace thread {

// Per-thread value, created on first access from a copy of the default.
template <class T>
class LocalStorage {
public:
    explicit LocalStorage(const T& def = T()) : m_default(def) {}
    LocalStorage(const LocalStorage&) = delete;
    LocalStorage& operator=(const LocalStorage&) = delete;

    T& Value()
    {
        std::lock_guard<std::mutex> l(m_lock);
        auto it = m_values.find(std::this_thread::get_id());
        if (it == m_values.end())
            it = m_values.emplace(std::this_thread::get_id(), m_default).first;
        return it->second;
    }
    T& operator*() { return Value(); }
    T* operator->() { return &Value(); }

private:
    T m_default;
    std::mutex m_lock;
    std::map<std::thread::id, T> m_values;
};

}
}

#endif
