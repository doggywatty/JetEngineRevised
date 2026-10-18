// libs/localization_lib/include/localization/formatter.h
#ifndef LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_FORMATTER_H
#define LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_FORMATTER_H

#include <ctime>
#include <unordered_map>
#include <vector>
#include "core/pstring.h"
#include "json/json.h"
#include "localization/filter.h"

namespace babel {

class Babel;
class FormatterNumber;

// Per language number, date and time formats from langs.json
class Formatter {  // sizeof 0x13C
public:
    enum FormatterNumberEnum { Comma = 0, Dot = 1, Dot5 = 2, Space = 3, Space5 = 4 };
    enum CapitalizationEnum { CapDefault = 0, CapNoAccents = 1, CapNone = 2 };

    struct TLangInfo {
        TLangInfo();
        int currency;  // 0x0: FormatterNumberEnum
        int quantity;  // 0x4
        int capital;  // 0x8: CapitalizationEnum
        jet::String strings[30];  // 0xC..0x80: date, time, interval, interval_day, interval_days, percentage, crono,
                                  // custom_time, period_{seconds,minutes,hours,days}, then the 7 "simple" and 11 "verbose" atoms
        const jet::String &date() const { return strings[0]; }
        const jet::String &time() const { return strings[1]; }
        const jet::String &interval() const { return strings[2]; }
        const jet::String &intervalDay() const { return strings[3]; }
        const jet::String &intervalDays() const { return strings[4]; }
        const jet::String &percentage() const { return strings[5]; }
        const jet::String &crono() const { return strings[6]; }
        const jet::String &customTime() const { return strings[7]; }
        const jet::String &periodSeconds() const { return strings[8]; }
        const jet::String &periodMinutes() const { return strings[9]; }
        const jet::String &periodHours() const { return strings[10]; }
        const jet::String &periodDays() const { return strings[11]; }
    };

    Formatter(Babel *babel);
    ~Formatter();
    jet::String FormatMeasure(float v, int digits) const;
    jet::String FormatMeasure(int v) const;
    jet::String FormatMeasure(long long v) const;
    jet::String FormatCurrency(float v, int digits) const;
    jet::String FormatCurrency(int v) const;
    jet::String FormatPercentage(float v, int digits) const;
    jet::String FormatPercentage(int v) const;
    jet::String FormatDate(int day, int month, int year) const;
    jet::String FormatDate(long t) const;
    jet::String FormatTime(int hour, int minute) const;
    jet::String FormatTime(long t) const;
    jet::String FormatTimeCustom(int hour, int minute, int second) const;
    jet::String FormatTimeCustom(long t) const;
    jet::String FormatInterval(long t) const;
    jet::String FormatPeriod(long t) const;
    jet::String FormatCrono(int minutes, int seconds, int millis) const;
    jet::String FormatCrono(unsigned long long ms) const;
    void FormatCrono(char *out, int minutes, int seconds, int millis) const;
    void FormatCrono(char *out, unsigned long long ms) const;
    jet::String FormatByAtoms(unsigned seconds, unsigned count) const;
    jet::String FormatByAtomsVerbose(unsigned seconds, unsigned count) const;
    jet::String ToUpper(const jet::String &s) const;
    jet::String ToLower(const jet::String &s) const;
    const TLangInfo &GetLangInfo() const { return m_current; }
    bool UpdateLangInfo();
private:
    void ParseJsonConfig();
    int ReadNumberFormatter(const Json::Value &v);
    int ReadCapitalization(const Json::Value &v);
    std::vector<FormatterNumber*> m_formatter;  // 0x0
    std::unordered_map<short, TLangInfo> m_langInfo;  // 0xC
    TLangInfo m_current;  // 0x24
    Filter m_toUpperFilter;  // 0xA8
    Filter m_toLowerFilter;  // 0xD8
    Filter m_noAccentFilter;  // 0x108
    Babel *m_babel;  // 0x138
};

}

#endif
