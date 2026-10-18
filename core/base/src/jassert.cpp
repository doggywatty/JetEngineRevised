// core/base/src/jassert.cpp
#include "jassert.h"
#include <cstdarg>
#include <cstdio>

namespace {

assertLib::AssertType DefaultHandler(const char* expr, const char* msg, const char* file, int line)
{
    printf("%s(%d): Assert Failure: ", file, line);
    if (expr)
        printf("'%s' ", expr);
    if (msg)
        printf("%s", msg);
    putchar('\n');
    return assertLib::AssertTypeDef::CRASH;  // the result is not acted on, as in the original
}

assertLib::AssertHandler g_handler = DefaultHandler;

}

namespace assertLib {

void SetAssertHandler(AssertHandler h) { g_handler = h ? h : DefaultHandler; }

void TestCondition(const char* expr, const char* file, int line, const char* fmt, ...)
{
    char buf[8192];
    const char* msg = nullptr;
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        msg = buf;
    }
    g_handler(expr, msg, file, line);
}

}
