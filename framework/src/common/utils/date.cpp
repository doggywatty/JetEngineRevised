// framework/src/common/utils/date.cpp
#include "common/utils/date.h"
#include <cstdlib>
#include <ctime>

static void Fill(Date &d, const tm *t, int &day, int &month, int &year, int &h, int &m, int &s) {
    year = t->tm_year + 1900;
    month = t->tm_mon + 1;
    day = t->tm_mday;
    h = t->tm_hour;
    m = t->tm_min;
    s = t->tm_sec;
}

// @0x38e0a0
void Date::SetLocalTime(long long t) {
    m_isInitialized = true;
    time_t tt = (time_t)t;
    tm *p = gmtime(&tt);
    if (!p) {
        tt = time(0);
        p = gmtime(&tt);
    }
    Fill(*this, p, m_day, m_month, m_year, m_hours, m_minutes, m_seconds);
}

// @0x454ed0
void Date::SetTime(long long t) {
    m_isInitialized = true;
    time_t tt = (time_t)t;
    tm *p = localtime(&tt);
    if (!p) {
        tt = time(0);
        p = localtime(&tt);
    }
    Fill(*this, p, m_day, m_month, m_year, m_hours, m_minutes, m_seconds);
}

// @0x38e120
void Date::SetToNow() {
    m_isInitialized = true;
    time_t tt = time(0);
    tm *p = localtime(&tt);
    Fill(*this, p, m_day, m_month, m_year, m_hours, m_minutes, m_seconds);
}

// days from 1970-01-01 to a civil date
static long long DaysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long long)doe - 719468;
}

// @0x38e18c: the date is clamped to what boost accepts (year 1400 to 10000, month 1 to 12, day 1 to 31)
int Date::ToTimeSinceEpoch() const {
    if (!m_isInitialized) return 0;
    int y = (m_year < 1400 || m_year > 10000) ? 1400 : m_year;
    int mo = (m_month < 1 || m_month > 12) ? 1 : m_month;
    int d = (m_day < 1 || m_day > 31) ? 1 : m_day;
    return (int)(DaysFromCivil(y, (unsigned)mo, (unsigned)d) * 86400 + (long long)m_hours * 3600 + (long long)m_minutes * 60 + m_seconds);
}

// @0x593dbc
bool Date::operator==(const Date &o) const {
    return m_year == o.m_year && m_month == o.m_month && m_day == o.m_day && m_hours == o.m_hours && m_minutes == o.m_minutes && m_seconds == o.m_seconds;
}

// @0x38e444
bool Date::operator<(const Date &o) const {
    if (m_year != o.m_year) return m_year < o.m_year;
    if (m_month != o.m_month) return m_month < o.m_month;
    if (m_day != o.m_day) return m_day < o.m_day;
    if (m_hours != o.m_hours) return m_hours < o.m_hours;
    if (m_minutes != o.m_minutes) return m_minutes < o.m_minutes;
    return m_seconds < o.m_seconds;
}

// @0x38e4d0
void Date::FromString(const std::string &s) {
    m_isInitialized = false;
    if (s.empty()) {
        m_day = m_month = m_year = m_hours = m_minutes = m_seconds = 0;
        return;
    }
    size_t cut = s.find('T');
    if (cut == std::string::npos) cut = s.find(' ');
    std::string date = s.substr(0, cut), time = s.substr(cut + 1);
    size_t plus = s.find('+');
    if (plus != std::string::npos) time = s.substr(cut + 1, plus - (cut + 1));
    int f[3] = { 0, 0, 0 };
    size_t pos = 0;
    for (int i = 0; i < 3 && pos <= date.size(); i++) {
        size_t e = date.find('-', pos);
        f[i] = atoi(date.substr(pos, e == std::string::npos ? e : e - pos).c_str());
        if (e == std::string::npos) break;
        pos = e + 1;
    }
    m_year = f[0], m_month = f[1], m_day = f[2];
    pos = 0;
    int g[3] = { 0, 0, 0 };
    for (int i = 0; i < 3 && pos <= time.size(); i++) {
        size_t e = time.find(':', pos);
        g[i] = atoi(time.substr(pos, e == std::string::npos ? e : e - pos).c_str());
        if (e == std::string::npos) break;
        pos = e + 1;
    }
    m_hours = g[0], m_minutes = g[1], m_seconds = g[2];
}
