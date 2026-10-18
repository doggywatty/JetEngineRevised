// libs/glf/include/glf/core/rtti.h
#ifndef LIBS_GLF_INCLUDE_GLF_CORE_RTTI_H
#define LIBS_GLF_INCLUDE_GLF_CORE_RTTI_H

namespace glf {
struct Rtti : private glf::NonCopyable {  // sizeof 0xC
    Rtti(const glf::Rtti*, const char*);
    bool IsRtti();
    bool IsDerivedFrom(const glf::Rtti&);
    static const int IS_RTTI;
    const int mIsRtti;  // 0x0
    const glf::Rtti *mParentRtti;  // 0x4
    const char *mName;  // 0x8
};
}

#endif
