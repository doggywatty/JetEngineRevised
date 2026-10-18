// libs/localization_lib/include/localization/lang.h
#ifndef LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_LANG_H
#define LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_LANG_H

#include "core/pstring.h"

namespace babel {
// a language is its two ISO letters packed big-endian
struct Lang {  // sizeof 0x4
    enum type { NONE = 11565, EN = 25966, FR = 26226, DE = 25701, IT = 26996, ES = 25971, JA = 27233,
                PT = 28788, BR = 25202, KO = 27503, ZH = 31336, RU = 29301, TR = 29810, AR = 24946 };
    Lang() : enumVal(NONE) {}
    Lang(unsigned v) : enumVal(v) {}
    Lang(type v) : enumVal(v) {}
    operator type() const { return (type)enumVal; }
    type value() const { return (type)enumVal; }
    void ToCode(jet::String&);
    static short ToShort(const jet::String&);
private:
    unsigned enumVal;  // 0x0
};
}

#endif
