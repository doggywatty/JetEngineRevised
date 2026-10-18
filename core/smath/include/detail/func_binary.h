// core/smath/include/detail/func_binary.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_BINARY_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_BINARY_H

#include <cstdint>

namespace math {

inline unsigned countBits(uint32_t v)
{
    unsigned n = 0;
    for (; v; v &= v - 1)
        ++n;
    return n;
}

inline int lowestBit(uint32_t v)
{
    if (!v)
        return -1;
    int i = 0;
    while (!(v & 1)) {
        v >>= 1;
        ++i;
    }
    return i;
}

}

#endif
