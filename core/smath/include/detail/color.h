// core/smath/include/detail/color.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_COLOR_H
#define CORE_SMATH_INCLUDE_DETAIL_COLOR_H

#include <cstdint>
#include "vec4.h"

namespace math {
namespace color {

// u32 colors are 0xAARRGGBB.
inline uint32_t swapRB(uint32_t c) { return (c & 0xffffff) >> 16 | (c & 0xff) << 16 | (c & 0xff00ff00); }

// t in [0, 256].
inline uint32_t lerp(uint32_t a, uint32_t b, uint32_t t)
{
    return (((0x100 - t) * ((a & 0xff00ffff) >> 8) + t * ((b & 0xff00ffff) >> 8)) & 0xff00ff00) |
           ((((a & 0xff00ff) * (0x100 - t) + (b & 0xff00ff) * t) & 0xff00ffff) >> 8);
}

template <class T>
vec4<T> u32ToRgbaVec4(uint32_t c)
{
    const T k = T(0.003921568859368563);
    return vec4<T>(T(int((c & 0xffffff) >> 16)) * k, T(int((c & 0xffff) >> 8)) * k, T(int(c & 0xff)) * k, T(c >> 24) * k);
}

template <class T>
uint32_t rgbaVec4ToU32(const vec4<T>& v)
{
    auto b = [](T x) { x = x < T(0) ? T(0) : x > T(1) ? T(1) : x; return uint32_t(x * T(255) + T(0.5)); };
    return b(v.w) << 24 | b(v.x) << 16 | b(v.y) << 8 | b(v.z);
}

}
}

#endif
