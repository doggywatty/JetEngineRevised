// core/jet/src/core/util.cpp
#include "core/util.h"
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "core/pstring.h"
#include "stream/istream.h"
#include "stream/streammgr.h"

namespace jet {
namespace core {

s64 s_rndx = 0, s_rndy = 0, s_rndz = 0, s_rndw = 0;

void Srand(s64 x, s64 y, s64 z, s64 w)
{
    s_rndx = x;
    s_rndy = y;
    s_rndz = z;
    s_rndw = w;
}

static s64 Step()
{
    s64 t = s_rndx ^ (s64(u64(s_rndx) << 11));
    s_rndx = s_rndy;
    s_rndy = s_rndz;
    s_rndz = s_rndw;
    s_rndw = s_rndw ^ (s_rndw >> 19) ^ t ^ (t >> 8);
    return s_rndw;
}

// 31-bit result; 0x7fffffff is never returned.
u32 Rand()
{
    s64 w;
    do
        w = Step();
    while ((w & 0x7fffffff) == 0x7fffffff);
    return u32(w) & 0x7fffffff;
}

int Rand(int a, int b)
{
    if (b < a) {
        int t = a;
        a = b;
        b = t;
    }
    int r = int(Rand());
    int range = b - a;
    if (!range)
        return a;
    return a + int(s64(r >> 2) % s64(range));
}

int Rand2(int a, int b)
{
    u32 w = u32(Step());
    return a + int((s64(b - a) * ((w & 0x3ffff) >> 2)) >> 16);
}

float Randf(float a, float b)
{
    float lo = a, hi = b;
    if (!(a < b) && a != b) {
        lo = b;
        hi = a;
    }
    return lo + (hi - lo) * float(Rand() & 0xffff) * 1.52587890625e-05f;
}

float Randf2(u32& seed)
{
    seed *= 0x41a7;
    u32 bits = (seed & 0x7fffff) | 0x40000000;
    float f;
    memcpy(&f, &bits, 4);
    return f - 3.0f;
}

// @0x682370: 0.5 + 0.5 * Randf2
float RandPosf2(u32& seed)
{
    float r = Randf2(seed);
    return 0.5f + r * 0.5f;
}

float Randf2(u32& seed, float a, float b)
{
    float h = (b - a) * 0.5f;
    return h + Randf2(seed) * h + a;
}

const char* Strfmt(const char* fmt, ...)
{
    static char buf[0x800];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return buf;
}

const char* Stristr(const char* s, const char* find)
{
    for (; *s; ++s) {
        const char *a = s, *b = find;
        while (*a && *b && toupper(u8(*a)) == toupper(u8(*b))) {
            ++a;
            ++b;
        }
        if (!*b)
            return s;
    }
    return nullptr;
}

void StrideCopy(u8* dst, unsigned dstStride, const u8* src, unsigned srcStride, unsigned elemSize, unsigned count)
{
    if (!srcStride)
        srcStride = elemSize;
    if (!dstStride)
        dstStride = elemSize;
    for (unsigned i = 0; i < count; ++i, dst += dstStride, src += srcStride)
        memcpy(dst, src, elemSize);
}

bool ResolveIncludes(const std::string& src, std::string& out, std::set<std::string>& seen)
{
    if (src.empty())
        return false;
    out = src;
    size_t pos = out.find("#include");
    while (pos != std::string::npos) {
        size_t start = pos + 8;
        size_t eol = out.find('\n', start);
        if (eol == std::string::npos)
            eol = out.size();
        std::string name = out.substr(start, eol - start);
        size_t q0 = name.find('"'), q1 = name.rfind('"');
        if (q0 != q1)
            name = name.substr(q0 + 1, q1 - q0 - 1);
        stream::StreamMgr& m = stream::StreamMgr::GetInstance();
        stream::IStream* s = m.CreateStream(String(name));
        if (!s)
            s = m.CreateStreamByName(String(name));
        if (!s) {
            printf("ResolveIncludes: cannot open '%s'\n", name.c_str());
            return false;
        }
        seen.insert(name);
        std::string text;
        s->Start();
        unsigned n = s->Size();
        if (n) {
            text.resize(n);
            s->Read(&text[0], n);
            text += '\n';
        }
        s->Finish();
        delete s;
        std::string expanded;
        if (!ResolveIncludes(text, expanded, seen))
            return false;
        std::string r = out.substr(0, pos) + expanded;
        if (eol < out.size())
            r += out.substr(eol + 1);
        out = r;
        pos = out.find("#include", pos + expanded.size());
    }
    return true;
}

std::string ResolveIncludes(const std::string& src)
{
    std::string out;
    std::set<std::string> seen;
    return ResolveIncludes(src, out, seen) ? out : std::string();
}

}
}
