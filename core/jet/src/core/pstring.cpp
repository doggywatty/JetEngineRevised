// core/jet/src/core/pstring.cpp
#include "core/pstring.h"
#include "core/pstringdb.h"
#include "stream/istream.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

namespace jet {

namespace detail {

int tolowerascii(int c) { return unsigned(c - 'A') < 26 ? c + 32 : c; }
int toupperascii(int c) { return unsigned(c - 'a') < 26 ? c - 32 : c; }

const char* memichr(const char* s, char c, unsigned n)
{
    int lc = tolowerascii(c);
    for (unsigned i = 0; i < n; ++i)
        if (tolowerascii(s[i]) == lc)
            return s + i;
    return nullptr;
}

const char* stristr(const char* s, const char* find)
{
    size_t n = strlen(find);
    for (; *s; ++s) {
        size_t i = 0;
        while (i < n && tolowerascii(s[i]) == tolowerascii(find[i]))
            ++i;
        if (i == n)
            return s;
    }
    return nullptr;
}

}

const String String::null;

String::StringData::StringData(const StringData& o) : cell(o.cell)
{
    if (cell && cell->counter)
        cell->counter->fetch_add(1);
}

String::StringData::~StringData()
{
    if (cell && cell->counter)
        cell->counter->fetch_sub(1);
}

String::StringData& String::StringData::operator=(const StringData& o)
{
    StringData t(o);
    swap(t);
    return *this;
}

void String::StringData::reset()
{
    StringData t;
    swap(t);
}

String::String(const char* s) { *this = s; }

String::String(const char* b, const char* e) { StringDB::GetInstance().InternalizeString(b, e, mData); }

String::String(const char* s, unsigned n) { StringDB::GetInstance().InternalizeString(s, s + n, mData); }

String::String(const std::string& s) { *this = s; }

String::String(const String& s, unsigned pos, unsigned n) { *this = s.substr(pos, n); }

String& String::operator=(const char* s)
{
    if (!s || !*s)
        mData.reset();
    else
        StringDB::GetInstance().InternalizeString(s, s + strlen(s), mData);
    return *this;
}

String& String::operator=(const std::string& s)
{
    if (s.empty())
        mData.reset();
    else
        StringDB::GetInstance().InternalizeString(s.data(), s.data() + s.size(), mData);
    return *this;
}

String String::Format(const char* fmt, ...)
{
    std::vector<char> buf(512);
    for (;;) {
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(buf.data(), buf.size() - 1, fmt, ap);
        va_end(ap);
        if (n >= 0 && n + 1 < int(buf.size()))
            return String(buf.data());
        if (buf.size() > 0xffff)
            return null;
        buf.resize(buf.size() * 2);
    }
}

const char* String::c_str() const { return mData.cell ? mData.cell->string : ""; }

void String::resize(unsigned n)
{
    if (n <= size()) {
        *this = substr(0, n);
        return;
    }
    std::vector<char> buf(begin(), end());
    buf.resize(n, 0);
    *this = String(buf.data(), buf.data() + n);
}

String String::substr(unsigned pos, unsigned n) const
{
    if (n == npos)
        n = size() - pos;
    if (n == 0)
        return null;
    return String(c_str() + pos, c_str() + pos + n);
}

void String::append(char c)
{
    if (!mData.cell) {
        char s[2] = {c, 0};
        *this = String(s, s + 1);
        return;
    }
    std::vector<char> buf(begin(), end());
    buf.push_back(c);
    *this = String(buf.data(), buf.data() + buf.size());
}

void String::append(const char* s)
{
    if (!s || !*s)
        return;
    append(s, s + strlen(s));
}

void String::append(const char* b, const char* e)
{
    if (!b || !*b)
        return;
    if (!mData.cell) {
        *this = String(b, e);
        return;
    }
    std::vector<char> buf(begin(), end());
    buf.insert(buf.end(), b, e);
    *this = String(buf.data(), buf.data() + buf.size());
}

void String::append(const String& s)
{
    if (s.empty())
        return;
    if (empty()) {
        *this = s;
        return;
    }
    append(s.begin(), s.end());
}

unsigned String::find(const char* s, unsigned pos) const
{
    if (!s || !*s)
        return 0;
    if (pos >= size())
        return npos;
    const char* p = detail::stristr(c_str() + pos, s);
    return p ? unsigned(p - c_str()) : npos;
}

unsigned String::find(char c, unsigned pos) const { return find_first_of(c, pos); }

unsigned String::find_first_of(char c, unsigned pos) const
{
    for (unsigned n = size(); pos < n; ++pos)
        if (c_str()[pos] == c)
            return pos;
    return npos;
}

unsigned String::find_first_of(const char* s, unsigned pos) const
{
    if (!s || !*s)
        return pos;
    unsigned l = unsigned(strlen(s));
    for (unsigned n = size(); pos < n; ++pos)
        if (detail::memichr(s, c_str()[pos], l))
            return pos;
    return npos;
}

unsigned String::find_first_of(const String& s, unsigned pos) const
{
    if (s.empty())
        return pos;
    for (unsigned n = size(); pos < n; ++pos)
        if (detail::memichr(s.c_str(), c_str()[pos], s.size()))
            return pos;
    return npos;
}

unsigned String::find_first_not_of(char c, unsigned pos) const
{
    for (unsigned n = size(); pos < n; ++pos)
        if (c_str()[pos] != c)
            return pos;
    return npos;
}

unsigned String::find_first_not_of(const char* s, unsigned pos) const
{
    if (!s || !*s)
        return pos < size() ? pos : npos;
    unsigned l = unsigned(strlen(s));
    for (unsigned n = size(); pos < n; ++pos)
        if (!detail::memichr(s, c_str()[pos], l))
            return pos;
    return npos;
}

unsigned String::find_first_not_of(const String& s, unsigned pos) const { return find_first_not_of(s.c_str(), pos); }

unsigned String::find_last_of(char c, unsigned pos) const
{
    if (empty())
        return npos;
    if (pos == npos)
        pos = size() - 1;
    else if (pos >= size())
        return npos;
    for (;; --pos) {
        if (c_str()[pos] == c)
            return pos;
        if (pos == 0)
            return npos;
    }
}

unsigned String::find_last_of(const char* s, unsigned pos) const
{
    if (empty())
        return npos;
    if (pos == npos)
        pos = size() - 1;
    else if (pos >= size())
        return npos;
    if (!s || !*s)
        return pos;
    unsigned l = unsigned(strlen(s));
    for (;; --pos) {
        if (detail::memichr(s, c_str()[pos], l))
            return pos;
        if (pos == 0)
            return npos;
    }
}

unsigned String::find_last_of(const String& s, unsigned pos) const { return find_last_of(s.c_str(), pos); }

unsigned String::find_last_not_of(char c, unsigned pos) const
{
    if (empty())
        return npos;
    if (pos == npos || pos >= size())
        pos = size() - 1;
    for (;; --pos) {
        if (c_str()[pos] != c)
            return pos;
        if (pos == 0)
            return npos;
    }
}

unsigned String::find_last_not_of(const char* s, unsigned pos) const
{
    if (empty())
        return npos;
    if (pos == npos || pos >= size())
        pos = size() - 1;
    unsigned l = s ? unsigned(strlen(s)) : 0;
    for (;; --pos) {
        if (!detail::memichr(s, c_str()[pos], l))
            return pos;
        if (pos == 0)
            return npos;
    }
}

unsigned String::find_last_not_of(const String& s, unsigned pos) const { return find_last_not_of(s.c_str(), pos); }

String String::GetLowercase() const
{
    if (empty())
        return null;
    std::vector<char> buf(begin(), end());
    for (char& c : buf)
        c = char(detail::tolowerascii(c));
    return String(buf.data(), buf.data() + buf.size());
}

String String::GetUppercase() const
{
    if (empty())
        return null;
    std::vector<char> buf(begin(), end());
    for (char& c : buf)
        c = char(detail::toupperascii(c));
    return String(buf.data(), buf.data() + buf.size());
}

static int CompareNoCase(const char* a, const char* b)
{
    for (;; ++a, ++b) {
        int x = detail::tolowerascii(*a), y = detail::tolowerascii(*b);
        if (x != y || !x)
            return x - y;
    }
}

bool String::LessThan(const String& s) const { return !Equals(s) && CompareNoCase(c_str(), s.c_str()) < 0; }
bool String::LessThan(const char* s) const { return CompareNoCase(c_str(), s ? s : "") < 0; }
bool String::LessThanCS(const String& s) const { return strcmp(c_str(), s.c_str()) < 0; }
bool String::LessThanCS(const char* s) const { return strcmp(c_str(), s ? s : "") < 0; }

bool String::Equals(const char* s) const
{
    if (!s)
        return empty();
    unsigned n = unsigned(strlen(s));
    if (!mData.cell)
        return n == 0;
    if (size() != n)
        return false;
    for (unsigned i = 0; i < n; ++i)
        if (detail::tolowerascii(s[i]) != detail::tolowerascii(c_str()[i]))
            return false;
    return true;
}

// Matches the original: std::string compares case-sensitively.
bool String::Equals(const std::string& s) const { return size() == s.size() && memcmp(c_str(), s.data(), size()) == 0; }

bool String::EqualsCS(const char* s) const { return strcmp(c_str(), s ? s : "") == 0; }
bool String::EqualsCS(const std::string& s) const { return Equals(s); }

String ReadString(const stream::IStream& s)
{
    u16 n = 0;
    s >> n;
    if (!n)
        return String::null;
    std::vector<char> buf(n);
    s.Read(buf.data(), n);
    return String(buf.data(), buf.data() + n);
}

std::string ReadCString(const stream::IStream& s)
{
    u16 n = 0;
    s >> n;
    std::string r(n, '\0');
    if (n)
        s.Read(&r[0], n);
    return r;
}

void SkipString(stream::IStream& s)
{
    u16 n = 0;
    s >> n;
    if (n)
        s.SeekCur(n);
}

static void WriteRaw(stream::IStreamW& s, const char* p, unsigned n)
{
    u16 len = u16(n);
    s << len;
    if (len)
        s.Write(p, len);
}

void WriteString(stream::IStreamW& s, const String& str) { WriteRaw(s, str.c_str(), str.size()); }
void WriteString(stream::IStreamW& s, const std::string& str) { WriteRaw(s, str.data(), unsigned(str.size())); }
void WriteString(stream::IStreamW& s, const char* str) { WriteRaw(s, str ? str : "", str ? unsigned(strlen(str)) : 0); }

namespace stream {
const IStream& operator>>(const IStream& s, String& v) { v = ReadString(s); return s; }
const IStream& operator>>(const IStream& s, std::string& v) { v = ReadCString(s); return s; }
IStreamW& operator<<(IStreamW& s, const String& v) { WriteString(s, v); return s; }
IStreamW& operator<<(IStreamW& s, const std::string& v) { WriteString(s, v); return s; }
}

}
