// core/jet/src/encoder.cpp
#include "encoder.h"

namespace jet {
namespace encoder {

static const char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

u8 GetValueFromBase64(u8 c)
{
    if (u8(c - 'a') < 26) return u8(c - 71);
    if (u8(c - '0') < 10) return u8(c + 4);
    if (u8(c - 'A') < 26) return u8(c - 'A');
    return c == '+' ? 62 : c == '/' ? 63 : 0;
}

bool EncodeBase64(unsigned size, const u8* data, std::vector<u8>& out)
{
    out.clear();
    out.reserve(size * 4 / 3 + 4);
    for (unsigned i = 0; i < size; i += 3) {
        u32 v = u32(data[i]) << 16;
        if (i + 1 < size) v += u32(data[i + 1]) << 8;
        if (i + 2 < size) v += data[i + 2];
        for (int s = 18; s >= 0; s -= 6)
            out.push_back(u8(kAlphabet[(v >> s) & 63]));
    }
    if (size % 3 == 1)
        out[out.size() - 2] = out[out.size() - 1] = '=';
    else if (size % 3 == 2)
        out[out.size() - 1] = '=';
    return true;
}

bool DecodeBase64(unsigned size, const u8* data, std::vector<u8>& out)
{
    out.clear();
    out.reserve(size);
    for (unsigned i = 0; i < size; i += 4) {
        u32 v = u32(GetValueFromBase64(data[i])) << 18 | u32(GetValueFromBase64(data[i + 1])) << 12 |
                u32(GetValueFromBase64(data[i + 2])) << 6 | GetValueFromBase64(data[i + 3]);
        out.push_back(u8(v >> 16));
        out.push_back(u8(v >> 8));
        out.push_back(u8(v));
    }
    if (size < 3 && size != 2)
        return true;
    if (size >= 3 && data[size - 2] == '=') {
        out.resize(out.size() - 2);
        return true;
    }
    if (data[size >= 3 ? size - 1 : 1] == '=')
        out.resize(out.size() - 1);
    return true;
}

}
}
