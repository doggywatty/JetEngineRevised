// core/base/include/jrtti.h
#ifndef CORE_BASE_INCLUDE_JRTTI_H
#define CORE_BASE_INCLUDE_JRTTI_H

// Jet's own RTTI: every class gets a unique id (a function-local static) and a short name.
namespace jet {
namespace rtti {
int NewClassId();
}

template <class T>
T* rtti_cast(const void* p);
}

#define JET_RTTI_COMMON(Class)                                                                     \
public:                                                                                            \
    static int RttiGetClassId()                                                                    \
    {                                                                                              \
        static const int id = ::jet::rtti::NewClassId();                                           \
        return id;                                                                                 \
    }                                                                                              \
    static const char* RttiGetClassIdName() { return #Class; }                                     \
    virtual const char* RttiGetInstanceClassIdName() const { return RttiGetClassIdName(); }        \
    virtual int RttiGetInstanceClassId() const { return RttiGetClassId(); }

#define JET_RTTI_ROOT(Class)                                                                       \
    JET_RTTI_COMMON(Class)                                                                         \
    virtual bool RttiIsOfType(int id) const { return id == RttiGetClassId(); }                     \
    virtual void* RttiCastTo(int id) const { return id == RttiGetClassId() ? (void*)this : nullptr; }

#define JET_RTTI(Class, Base)                                                                      \
    JET_RTTI_COMMON(Class)                                                                         \
    virtual bool RttiIsOfType(int id) const { return id == RttiGetClassId() || Base::RttiIsOfType(id); } \
    virtual void* RttiCastTo(int id) const { return id == RttiGetClassId() ? (void*)this : Base::RttiCastTo(id); }

#endif
