// core/jet/include/text2/utf8/utf8/unchecked.h
#ifndef CORE_JET_INCLUDE_TEXT2_UTF8_UTF8_UNCHECKED_H
#define CORE_JET_INCLUDE_TEXT2_UTF8_UTF8_UNCHECKED_H

#include <cstdint>

namespace utf8::unchecked {
template<typename It> uint32_t next(It &it) {
    uint32_t cp = (uint8_t)*it;
    if (cp < 0x80) {
    } else if ((cp >> 5) == 6) {
        ++it; cp = ((cp << 6) & 0x7ff) + ((uint8_t)*it & 0x3f);
    } else if ((cp >> 4) == 14) {
        ++it; cp = ((cp << 12) & 0xffff) + (((uint8_t)*it << 6) & 0xfff);
        ++it; cp += (uint8_t)*it & 0x3f;
    } else if ((cp >> 3) == 30) {
        ++it; cp = ((cp << 18) & 0x1fffff) + (((uint8_t)*it << 12) & 0x3ffff);
        ++it; cp += ((uint8_t)*it << 6) & 0xfff;
        ++it; cp += (uint8_t)*it & 0x3f;
    }
    ++it;
    return cp;
}
}

#endif
