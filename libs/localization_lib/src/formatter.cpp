// libs/localization_lib/src/formatter.cpp
#include "localization/formatter.h"
#include "localization/babel.h"
#include "formatter/formatternumber.h"
#include <cstdio>
#include <cstring>

namespace babel {

// the 30 strings of a language without a json entry
static const char *k_defaults[30] = {
    "D/M/Y", "%H:%M", "%H:%M:%S", "%%d %H:%M:%S", "%%d %H:%M:%S", "%s%%", "%d:%d:%d", "%H:%M:%S", "%Ss", "%Mm", "%Hh", "%%dd",
    "%d", " ", "%d", ":", "%d", ":", "%d",
    "%d", "%d", " ", "%d", "%d", ":", "%d", "%d", ":", "%d", "%d" };

// @0x5af19c; the entry for "--" gets other strings (see ParseJsonConfig)
Formatter::TLangInfo::TLangInfo() : currency(0), quantity(0), capital(0) {}

// @0x5af31c: the number formatter table is Comma, Dot, Dot5, Space, Space5
Formatter::Formatter(Babel *babel)
    : m_toUpperFilter(jet::String("capitalization.json"), false), m_toLowerFilter(jet::String("capitalization.json"), true),
      m_noAccentFilter(jet::String("noaccent.json"), false), m_babel(babel) {
    m_formatter.resize(5);
    m_formatter[Comma] = new FormatterNumberComma();
    m_formatter[Dot] = new FormatterNumberDot();
    m_formatter[Space] = new FormatterNumberSpace();
    m_formatter[Space5] = new FormatterNumberSpace5();
    m_formatter[Dot5] = new FormatterNumberDot5();
    ParseJsonConfig();
}

Formatter::~Formatter() {
    for (size_t i = 0; i < m_formatter.size(); i++) delete m_formatter[i];
}

static void ReadString(const Json::Value &v, const char *key, jet::String &out) {
    if (v.isMember(key)) out = v[key].asString().c_str();
}

// @0x5afecc
void Formatter::ParseJsonConfig() {
    Json::Value root(Json::nullValue);
    Json::Reader reader;
    ReadJsonFile(jet::String("langs.json"), reader, root);

    TLangInfo none;
    none.currency = Dot;
    none.quantity = Dot;
    none.capital = CapNone;
    none.strings[0] = "D/M/Y";
    none.strings[1] = "%H:%M";
    none.strings[2] = "%j %H:%M:%S";
    none.strings[5] = "%s%%";
    none.strings[6] = "%d:%d:%d";
    none.strings[7] = "%H:%M:%S";
    m_langInfo.insert(std::make_pair((short)0x2d2d, none));
    if (!root.isArray()) return;
    for (unsigned i = 0; i < root.size(); i++) {
        const Json::Value &e = root[i];
        if (!e.isMember("code")) continue;
        TLangInfo info;
        info.currency = Dot;
        info.quantity = Dot;
        info.capital = CapNone;
        for (int k = 0; k < 30; k++) info.strings[k] = k_defaults[k];
        if (e.isMember("currency")) info.currency = ReadNumberFormatter(e["currency"]);
        if (e.isMember("quantity")) info.quantity = ReadNumberFormatter(e["quantity"]);
        ReadString(e, "date", info.strings[0]);
        ReadString(e, "time", info.strings[1]);
        if (e.isMember("capital")) info.capital = ReadCapitalization(e["capital"]);
        ReadString(e, "interval", info.strings[2]);
        ReadString(e, "interval_day", info.strings[3]);
        ReadString(e, "interval_days", info.strings[4]);
        ReadString(e, "period_seconds", info.strings[8]);
        ReadString(e, "period_minutes", info.strings[9]);
        ReadString(e, "period_hours", info.strings[10]);
        ReadString(e, "period_days", info.strings[11]);
        ReadString(e, "custom_time", info.strings[7]);
        ReadString(e, "percentage", info.strings[5]);
        ReadString(e, "crono", info.strings[6]);
        if (e.isMember("atoms")) {
            const Json::Value atoms = e["atoms"];
            if (atoms.isMember("simple")) {
                const Json::Value s = atoms["simple"];
                ReadString(s, "day", info.strings[12]);
                ReadString(s, "minute", info.strings[16]);
                ReadString(s, "hour", info.strings[14]);
                ReadString(s, "second", info.strings[18]);
                ReadString(s, "day_separator", info.strings[13]);
                ReadString(s, "hour_separator", info.strings[15]);
                ReadString(s, "minute_separator", info.strings[17]);
            }
            if (atoms.isMember("verbose")) {
                const Json::Value v = atoms["verbose"];
                static const char *keys[] = { "day", "days", "day_separator", "hour", "hours", "hour_separator", "minute", "minutes",
                                              "minute_separator", "second", "seconds" };
                for (int k = 0; k < 11; k++) ReadString(v, keys[k], info.strings[19 + k]);
            }
        }
        jet::String code(e["code"].asString().c_str());
        m_langInfo.insert(std::make_pair(Lang::ToShort(code), info));
    }
}


// @0x5b1044
int Formatter::ReadNumberFormatter(const Json::Value &v) {
    jet::String s(v.asString().c_str());
    if (s.Equals("DOT")) return Dot;
    if (s.Equals("COMMA")) return Comma;
    if (s.Equals("SPACE")) return Space;
    if (s.Equals("SPACE5")) return Space5;
    if (s.Equals("DOT5")) return Dot5;
    return Dot;
}

// @0x5b1128
int Formatter::ReadCapitalization(const Json::Value &v) {
    jet::String s(v.asString().c_str());
    if (s.Equals("DEFAULT")) return CapDefault;
    if (s.Equals("NOACCENTS")) return CapNoAccents;
    return CapNone;
}

// @0x5b1610
bool Formatter::UpdateLangInfo() {
    std::unordered_map<short, TLangInfo>::const_iterator it = m_langInfo.find((short)(unsigned short)m_babel->GetSelectedLanguage());
    if (it == m_langInfo.end()) return false;
    m_current = it->second;
    return true;
}

jet::String Formatter::FormatMeasure(float v, int digits) const {
    jet::String out;
    m_formatter[m_current.quantity]->FormatterNumber::Format(out, v, digits);
    return out;
}

jet::String Formatter::FormatMeasure(int v) const {
    jet::String out;
    m_formatter[m_current.quantity]->FormatterNumber::Format(out, v);
    return out;
}

jet::String Formatter::FormatMeasure(long long v) const {
    jet::String out;
    m_formatter[m_current.quantity]->FormatterNumber::Format(out, v);
    return out;
}

jet::String Formatter::FormatCurrency(float v, int digits) const {
    jet::String out;
    m_formatter[m_current.currency]->FormatterNumber::Format(out, v, digits);
    return out;
}

jet::String Formatter::FormatCurrency(int v) const {
    jet::String out;
    m_formatter[m_current.currency]->FormatterNumber::Format(out, v);
    return out;
}

// @0x5b11b0
jet::String Formatter::FormatPercentage(float v, int digits) const {
    char buf[32];
    snprintf(buf, sizeof(buf), m_current.percentage().c_str(), FormatMeasure(v, digits).c_str());
    return jet::String(buf);
}

jet::String Formatter::FormatPercentage(int v) const { return FormatPercentage((float)v, 0); }

// @0x5af7cc
jet::String Formatter::FormatDate(int day, int month, int year) const {
    time_t zero = 0;
    tm t = *localtime(&zero);
    t.tm_mon = month - 1;
    t.tm_year = year - 1900;
    t.tm_mday = day;
    mktime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), m_current.date().c_str(), &t);
    return jet::String(buf);
}

// @0x5af8ac
jet::String Formatter::FormatDate(long v) const {
    time_t tt = (time_t)v;
    tm t = *localtime(&tt);
    char buf[16];
    strftime(buf, sizeof(buf), m_current.date().c_str(), &t);
    return jet::String(buf);
}

// @0x5af95c
jet::String Formatter::FormatTime(int hour, int minute) const {
    time_t zero = 0;
    tm t = *gmtime(&zero);
    t.tm_hour = hour;
    t.tm_min = minute;
    mktime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), m_current.time().c_str(), &t);
    return jet::String(buf);
}

// @0x5afa24
jet::String Formatter::FormatTime(long v) const {
    time_t tt = (time_t)v;
    tm t = *gmtime(&tt);
    char buf[16];
    strftime(buf, sizeof(buf), m_current.time().c_str(), &t);
    return jet::String(buf);
}

// @0x5b165c
jet::String Formatter::FormatTimeCustom(int hour, int minute, int second) const {
    time_t zero = 0;
    tm t = *gmtime(&zero);
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    mktime(&t);
    char buf[256];
    strftime(buf, sizeof(buf), m_current.customTime().c_str(), &t);
    return jet::String(buf);
}

// @0x5b172c
jet::String Formatter::FormatTimeCustom(long v) const {
    time_t tt = (time_t)v;
    tm t = *gmtime(&tt);
    char buf[256];
    strftime(buf, sizeof(buf), m_current.customTime().c_str(), &t);
    return jet::String(buf);
}

// the day count goes through the "%%d" of the format
static jet::String FormatWithDays(const jet::String &fmt, const tm &t) {
    char buf[32], out[32];
    strftime(buf, sizeof(buf), fmt.c_str(), &t);
    if (t.tm_yday > 0) {
        snprintf(out, sizeof(out), buf, t.tm_yday);
        return jet::String(out);
    }
    return jet::String(buf);
}

// @0x5afad4
jet::String Formatter::FormatInterval(long v) const {
    time_t tt = (time_t)v;
    tm t = *gmtime(&tt);
    const jet::String &fmt = t.tm_yday == 1 ? m_current.intervalDay() : t.tm_yday < 2 ? m_current.interval() : m_current.intervalDays();
    return FormatWithDays(fmt, t);
}

// @0x5b17dc
jet::String Formatter::FormatPeriod(long v) const {
    time_t tt = (time_t)v;
    tm t = *gmtime(&tt);
    const jet::String &fmt = t.tm_yday >= 1 ? m_current.periodDays() : t.tm_hour >= 1 ? m_current.periodHours()
                             : t.tm_min >= 1 ? m_current.periodMinutes() : m_current.periodSeconds();
    return FormatWithDays(fmt, t);
}

// @0x5b1294 / @0x5b1304 / @0x5b1394 / @0x5b13d0
jet::String Formatter::FormatCrono(int minutes, int seconds, int millis) const {
    char buf[32];
    FormatCrono(buf, minutes, seconds, millis);
    return jet::String(buf);
}

jet::String Formatter::FormatCrono(unsigned long long ms) const {
    return FormatCrono((int)(ms / 60000), (int)(ms % 60000 / 1000), (int)(ms % 1000));
}

void Formatter::FormatCrono(char *out, int minutes, int seconds, int millis) const {
    sprintf(out, m_current.crono().c_str(), minutes, seconds, millis);
}

void Formatter::FormatCrono(char *out, unsigned long long ms) const {
    FormatCrono(out, (int)(ms / 60000), (int)(ms % 60000 / 1000), (int)(ms % 1000));
}

// pieces day, separator, hour, separator, minute, separator, second from the first non-zero unit, count units
static jet::String JoinAtoms(const jet::String (&atoms)[7], unsigned days, unsigned hours, unsigned minutes, unsigned count) {
    unsigned first = days ? 0 : hours ? 2 : minutes ? 4 : 6;
    std::string out;
    for (unsigned i = first; i < 7 && i + 1 < first + count * 2; i++) out += atoms[i].c_str();
    return jet::String(out.c_str());
}

// @0x5b1914
jet::String Formatter::FormatByAtoms(unsigned seconds, unsigned count) const {
    unsigned days = seconds / 86400, hoursTotal = seconds / 3600, minutesTotal = seconds / 60;
    unsigned hours = hoursTotal - days * 24, minutes = minutesTotal - hoursTotal * 60, secs = seconds - minutesTotal * 60;
    const TLangInfo &li = m_current;
    jet::String atoms[7] = { Strfmt(li.strings[12].c_str(), days), Strfmt(li.strings[13].c_str()), Strfmt(li.strings[14].c_str(), hours),
                             Strfmt(li.strings[15].c_str()), Strfmt(li.strings[16].c_str(), minutes), Strfmt(li.strings[17].c_str()),
                             Strfmt(li.strings[18].c_str(), secs) };
    return JoinAtoms(atoms, days, hours, minutes, count);
}

// @0x5b1bcc: singular/plural forms
jet::String Formatter::FormatByAtomsVerbose(unsigned seconds, unsigned count) const {
    unsigned days = seconds / 86400, hoursTotal = seconds / 3600, minutesTotal = seconds / 60;
    unsigned hours = hoursTotal - days * 24, minutes = minutesTotal - hoursTotal * 60, secs = seconds - minutesTotal * 60;
    const TLangInfo &li = m_current;
    jet::String atoms[7] = { Strfmt(li.strings[days == 1 ? 19 : 20].c_str(), days), Strfmt(li.strings[21].c_str()),
                             Strfmt(li.strings[hours == 1 ? 22 : 23].c_str(), hours), Strfmt(li.strings[24].c_str()),
                             Strfmt(li.strings[minutes == 1 ? 25 : 26].c_str(), minutes), Strfmt(li.strings[27].c_str()),
                             Strfmt(li.strings[secs == 1 ? 28 : 29].c_str(), secs) };
    return JoinAtoms(atoms, days, hours, minutes, count);
}

// @0x5afbe8: DEFAULT upper-cases, NOACCENTS also strips accents first, otherwise unchanged
jet::String Formatter::ToUpper(const jet::String &s) const {
    jet::String out(s);
    if (m_current.capital == CapDefault) return m_toUpperFilter.Apply(out);
    if (m_current.capital == CapNoAccents) return m_toUpperFilter.Apply(m_noAccentFilter.Apply(out));
    return out;
}

// @0x5afc90
jet::String Formatter::ToLower(const jet::String &s) const {
    if (m_current.capital == CapDefault || m_current.capital == CapNoAccents) return m_toLowerFilter.Apply(s);
    return s;
}

}
