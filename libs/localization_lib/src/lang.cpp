// libs/localization_lib/src/lang.cpp
#include "localization/lang.h"

namespace babel {

// @0x5aa884
void Lang::ToCode(jet::String &out) {
    char code[3] = { (char)(enumVal >> 8), (char)enumVal, 0 };
    out = code;
}

// @0x5aa84c: 0 unless the string is exactly two characters
short Lang::ToShort(const jet::String &s) {
    if (s.size() != 2) return 0;
    const char *c = s.c_str();
    return (short)(unsigned short)(c[1] + (c[0] << 8));
}

}
