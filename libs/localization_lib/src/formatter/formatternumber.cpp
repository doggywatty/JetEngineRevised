// libs/localization_lib/src/formatter/formatternumber.cpp
#include "formatter/formatternumber.h"
#include "localization/babel.h"
#include <cmath>
#include <string>

namespace babel {

// @0x5b3688: the fraction of a negative value is truncated, of a positive one rounded
void FormatterNumber::Format(jet::String &out, float value, int digits) const {
    float scale = powf(10.0f, (float)digits);
    if (!(value < 0.0f)) {
        double whole = floor((double)value);
        float frac = floorf((value - (float)whole) * scale + 0.5f);
        Format(out, (long long)whole, (int)frac);
    } else {
        double whole = floor((double)-value);
        Format(out, (long long)whole, (int)((-value - (float)whole) * scale));
        out = jet::String((std::string("-") + out.c_str()).c_str());
    }
}

// @0x5b37a8
void FormatterNumber::Format(jet::String &out, int value) const { Format(out, (long long)value); }

// the 64 bit value: the same, over long long
void FormatterNumber::Format(jet::String &out, long long value) const {
    if (value < 0) {
        Format(out, -value, 0);
        out = jet::String((std::string("-") + out.c_str()).c_str());
    } else {
        Format(out, value, 0);
    }
}

// FormatterNumberComma @0x5b3820, Dot @0x5b3aa0, Dot5 @0x5b3d20, Space @0x5b2ea8, Space5 @0x5b314c
void FormatterNumberGrouped::Format(jet::String &out, long long whole, int fraction) const {
    std::string s;
    if (fraction > 0) {
        s += m_decimal;
        s += Strfmt("%d", fraction).c_str();
    }
    long long rest = whole;
    if (whole == 0) s = "0" + s;
    bool first = true;
    while (rest > 0) {
        int group = (int)(rest % 1000);
        std::string g = Strfmt(rest < 1000 ? "%d" : "%03d", group).c_str();
        if (first) s = g + s;
        else if (m_fiveDigitMin && whole < 10000) s = g + s;
        else s = g + m_group + s;
        first = false;
        rest /= 1000;
    }
    out = s.c_str();
}

}
