// core/jet/include/core/util.h
#ifndef CORE_JET_INCLUDE_CORE_UTIL_H
#define CORE_JET_INCLUDE_CORE_UTIL_H

#include <set>
#include <string>
#include <vector>
#include "jtypes.h"

namespace jet {
namespace core {

// xorshift128 state (64-bit words). All zero until the game seeds it.
extern s64 s_rndx, s_rndy, s_rndz, s_rndw;
void Srand(s64 x, s64 y, s64 z, s64 w);

u32 Rand();  // 31 bits
int Rand(int a, int b);   // in [min, max)
int Rand2(int a, int b);  // single xorshift step, 16-bit fraction of (b - a)
float Randf(float a, float b);
float Randf2(u32& seed);  // LCG, in [-1, 1)
float RandPosf2(u32& seed);  // the same LCG, in [0, 1) (@0x682370)
float Randf2(u32& seed, float a, float b);

const char* Strfmt(const char* fmt, ...);  // formats into a static 2 KB buffer
const char* Stristr(const char* s, const char* find);
void StrideCopy(u8* dst, unsigned dstStride, const u8* src, unsigned srcStride, unsigned elemSize, unsigned count);

// Expands #include "file" lines from the stream manager, recursively. Returns "" on failure.
std::string ResolveIncludes(const std::string& src);
bool ResolveIncludes(const std::string& src, std::string& out, std::set<std::string>& seen);

template <class S, class C>
std::vector<S> Tokenize(const S& s, const C& sep)
{
    std::vector<S> r;
    unsigned b = 0;
    for (unsigned i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == sep) {
            if (i > b)
                r.push_back(S(s.c_str() + b, i - b));
            b = i + 1;
        }
    }
    return r;
}

}
}

#endif
