// core/base/include/jassert.h
#ifndef CORE_BASE_INCLUDE_JASSERT_H
#define CORE_BASE_INCLUDE_JASSERT_H

#include "jenum.h"

namespace assertLib {

struct AssertTypeDef {
    enum type { BREAK, CRASH };
};
typedef safe_enum<AssertTypeDef> AssertType;

typedef AssertType (*AssertHandler)(const char* expr, const char* msg, const char* file, int line);

void SetAssertHandler(AssertHandler h);
void TestCondition(const char* expr, const char* file, int line, const char* fmt, ...);

}

#if defined(NDEBUG) && !defined(JET_ASSERTS)
#define JET_ASSERT(c) ((void)0)
#define JET_ASSERT_MSG(c, ...) ((void)0)
#else
#define JET_ASSERT(c) ((c) ? (void)0 : ::assertLib::TestCondition(#c, __FILE__, __LINE__, nullptr))
#define JET_ASSERT_MSG(c, ...) ((c) ? (void)0 : ::assertLib::TestCondition(#c, __FILE__, __LINE__, __VA_ARGS__))
#endif

#endif
