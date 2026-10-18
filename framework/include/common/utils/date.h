// framework/include/common/utils/date.h
#ifndef FRAMEWORK_INCLUDE_COMMON_UTILS_DATE_H
#define FRAMEWORK_INCLUDE_COMMON_UTILS_DATE_H

#include <string>

// A calendar date and time of day, saved as its seven fields (0x1C bytes, uninitialized until set: every field -1)
struct Date {  // sizeof 0x1C
    Date() : m_day(-1), m_month(-1), m_year(-1), m_hours(-1), m_minutes(-1), m_seconds(-1), m_isInitialized(false) {}
    explicit Date(const std::string &s) { FromString(s); }
    int GetDay() const { return m_day; }
    int GetMonth() const { return m_month; }
    int GetYear() const { return m_year; }
    int GetHours() const { return m_hours; }
    int GetMinutes() const { return m_minutes; }
    int GetSeconds() const { return m_seconds; }
    bool IsInitialized() const { return m_isInitialized; }
    // from a time in seconds since the epoch, as a UTC time (SetLocalTime, @0x38e0a0) or in the local zone (SetTime, @0x454ed0)
    void SetTime(long long t);
    void SetLocalTime(long long t);
    void SetToNow();
    // seconds from the epoch to this date taken as a UTC one (0 while it is not set)
    int ToTimeSinceEpoch() const;
    bool operator==(const Date &o) const;
    bool operator!=(const Date &o) const { return !(*this == o); }
    bool operator<(const Date &o) const;
    bool operator>(const Date &o) const { return !(*this == o) && !(*this < o); }
    bool operator<=(const Date &o) const { return !(*this > o); }
    bool operator>=(const Date &o) const { return *this == o || *this > o; }
private:
    // "2013-05-02T10:20:30+0200" or "2013-05-02 10:20:30"
    void FromString(const std::string &s);
    int m_day, m_month, m_year, m_hours, m_minutes, m_seconds;  // 0x0 .. 0x14
    bool m_isInitialized;  // 0x18
};

#endif
