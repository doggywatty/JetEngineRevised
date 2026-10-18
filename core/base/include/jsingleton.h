// core/base/include/jsingleton.h
#ifndef CORE_BASE_INCLUDE_JSINGLETON_H
#define CORE_BASE_INCLUDE_JSINGLETON_H

#include <cstddef>

// The object registers itself on construction and unregisters on destruction.
template <class T>
class Singleton {
public:
    static T* GetInstance() { return s_instance; }
    static T& Instance() { return *s_instance; }
    static bool HasInstance() { return s_instance != nullptr; }

protected:
    Singleton() { s_instance = static_cast<T*>(this); }
    virtual ~Singleton() { s_instance = nullptr; }

private:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static T* s_instance;
};

template <class T>
T* Singleton<T>::s_instance = nullptr;

#endif
