// libs/localization_lib/src/formatter/formatternumber.h
#ifndef LIBS_LOCALIZATION_LIB_SRC_FORMATTER_FORMATTERNUMBER_H
#define LIBS_LOCALIZATION_LIB_SRC_FORMATTER_FORMATTERNUMBER_H

#include "core/pstring.h"

namespace babel {

class FormatterNumber {
public:
    virtual ~FormatterNumber() {}
    // the whole part is 64 bits: scores go past the 32 bits of an int (long long is 64 bits on the 32 bit devices too)
    virtual void Format(jet::String &out, long long whole, int fraction) const = 0;
    void Format(jet::String &out, float value, int digits) const;
    void Format(jet::String &out, int value) const;
    void Format(jet::String &out, long long value) const;
};

// Integer part in groups of three, the fraction (as an integer) after the decimal mark
class FormatterNumberGrouped : public FormatterNumber {
public:
    FormatterNumberGrouped(char decimal, char group, bool fiveDigitMin) : m_decimal(decimal), m_group(group), m_fiveDigitMin(fiveDigitMin) {}
    virtual void Format(jet::String &out, long long whole, int fraction) const;
    using FormatterNumber::Format;
private:
    char m_decimal, m_group;
    bool m_fiveDigitMin;  // 4-digit numbers are left ungrouped (the "5" variants)
};

struct FormatterNumberComma : FormatterNumberGrouped { FormatterNumberComma() : FormatterNumberGrouped('.', ',', false) {} };
struct FormatterNumberDot : FormatterNumberGrouped { FormatterNumberDot() : FormatterNumberGrouped(',', '.', false) {} };
struct FormatterNumberDot5 : FormatterNumberGrouped { FormatterNumberDot5() : FormatterNumberGrouped(',', '.', true) {} };
struct FormatterNumberSpace : FormatterNumberGrouped { FormatterNumberSpace() : FormatterNumberGrouped(',', ' ', false) {} };
struct FormatterNumberSpace5 : FormatterNumberGrouped { FormatterNumberSpace5() : FormatterNumberGrouped(',', ' ', true) {} };

}

#endif
