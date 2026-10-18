// libs/localization_lib/include/localization/filter.h
#ifndef LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_FILTER_H
#define LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_FILTER_H

#include <unordered_map>
#include <unordered_set>
#include "core/pstring.h"

namespace babel {
// Code point substitution table from a json array of {"in", "out"} (swapped when reversed).
class Filter {
public:
    Filter(const jet::String &file, bool reverse);
    ~Filter();
    jet::String Apply(const jet::String &text) const;
private:
    unsigned Convert(unsigned cp) const;
    std::unordered_map<unsigned, unsigned> m_conversion;  // 0x0
    mutable std::unordered_set<unsigned> m_result;  // 0x18
};
}

#endif
