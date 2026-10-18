// core/jet/include/core/pstring.h
#ifndef CORE_JET_INCLUDE_CORE_PSTRING_H
#define CORE_JET_INCLUDE_CORE_PSTRING_H

#include <atomic>
#include <functional>
#include <string>
#include "jtypes.h"

namespace jet {

namespace detail {
int tolowerascii(int c);
int toupperascii(int c);
const char* memichr(const char* s, char c, unsigned n);
const char* stristr(const char* s, const char* find);
}

namespace stream {
class IStream;
class IStreamW;
}

// Immutable, interned string. Equality is case-insensitive: every case variant of a string shares one key.
class String {
public:
    struct Cell {
        enum { stringStorageSize = 32 };
        int length;
        unsigned hash;
        const char* key;
        const char* string;
        Cell* next;
        Cell* nextSibling;
        std::atomic<int> counterData;
        std::atomic<int>* counter;
        char stringStorage[stringStorageSize];

        Cell() { Reset(); }
        void Reset();
    };

    class StringData {
    public:
        StringData() : cell(nullptr) {}
        explicit StringData(Cell* c) : cell(c) {}
        StringData(const StringData& o);
        ~StringData();
        StringData& operator=(const StringData& o);
        bool operator==(const StringData& o) const { return cell == o.cell; }
        void swap(StringData& o) { Cell* c = cell; cell = o.cell; o.cell = c; }
        void reset();
        bool isEmpty() const { return cell == nullptr; }
        Cell* getCell() const { return cell; }
        Cell* operator->() const { return cell; }

        Cell* cell;
    };

    static const String null;
    static const unsigned npos = 0xffffffffu;

    static String Format(const char* fmt, ...);

    String() {}
    String(const char* s);
    String(const char* b, const char* e);
    String(const char* s, unsigned n);
    String(const std::string& s);
    String(const String& s, unsigned pos, unsigned n);
    String(const String& s) : mData(s.mData) {}

    String& operator=(const char* s);
    String& operator=(const std::string& s);
    String& operator=(const String& s) { mData = s.mData; return *this; }

    bool operator==(const char* s) const { return Equals(s); }
    bool operator==(const std::string& s) const { return Equals(s); }
    bool operator==(const String& s) const { return Equals(s); }
    bool operator!=(const char* s) const { return !Equals(s); }
    bool operator!=(const std::string& s) const { return !Equals(s); }
    bool operator!=(const String& s) const { return !Equals(s); }
    bool operator<(const String& s) const { return LessThan(s); }

    String operator+(char c) const { String r(*this); r.append(c); return r; }
    String operator+(const char* s) const { String r(*this); r.append(s); return r; }
    String operator+(const String& s) const { String r(*this); r.append(s); return r; }
    String& operator+=(char c) { append(c); return *this; }
    String& operator+=(const char* s) { append(s); return *this; }
    String& operator+=(const String& s) { append(s); return *this; }
    char operator[](unsigned i) const { return c_str()[i]; }

    const char* c_str() const;
    unsigned size() const { return mData.cell ? mData.cell->length : 0; }
    unsigned length() const { return size(); }
    bool empty() const { return size() == 0; }
    void clear() { mData.reset(); }
    void swap(String& s) { mData.swap(s.mData); }
    void reserve(unsigned) {}
    void resize(unsigned n);
    String substr(unsigned pos, unsigned n = npos) const;
    void push_back(char c) { append(c); }
    void append(const String& s);
    void append(const char* b, const char* e);
    void append(const char* s);
    void append(char c);

    unsigned find(const String& s, unsigned pos = 0) const { return find(s.c_str(), pos); }
    unsigned find(const char* s, unsigned pos = 0) const;
    unsigned find(char c, unsigned pos = 0) const;
    unsigned find_first_of(const String& s, unsigned pos = 0) const;
    unsigned find_first_of(const char* s, unsigned pos = 0) const;
    unsigned find_first_of(char c, unsigned pos = 0) const;
    unsigned find_first_not_of(const String& s, unsigned pos = 0) const;
    unsigned find_first_not_of(const char* s, unsigned pos = 0) const;
    unsigned find_first_not_of(char c, unsigned pos = 0) const;
    unsigned find_last_of(const String& s, unsigned pos = npos) const;
    unsigned find_last_of(const char* s, unsigned pos = npos) const;
    unsigned find_last_of(char c, unsigned pos = npos) const;
    unsigned find_last_not_of(const String& s, unsigned pos = npos) const;
    unsigned find_last_not_of(const char* s, unsigned pos = npos) const;
    unsigned find_last_not_of(char c, unsigned pos = npos) const;

    const char* begin() const { return c_str(); }
    const char* end() const { return c_str() + size(); }
    unsigned GetHash() const { return mData.cell ? mData.cell->hash : 0; }
    const char* GetKey() const { return mData.cell ? mData.cell->key : nullptr; }
    String GetLowercase() const;
    String GetUppercase() const;

    bool LessThan(const char* s) const;
    bool LessThan(const String& s) const;
    bool LessThanCS(const char* s) const;
    bool LessThanCS(const String& s) const;
    bool Equals(const char* s) const;
    bool Equals(const std::string& s) const;
    bool Equals(const String& s) const { return GetKey() == s.GetKey(); }
    bool EqualsCS(const char* s) const;
    bool EqualsCS(const std::string& s) const;
    bool EqualsCS(const String& s) const { return mData.cell == s.mData.cell; }

private:
    void reset() { mData.reset(); }

    StringData mData;
};

String ReadString(const stream::IStream& s);
std::string ReadCString(const stream::IStream& s);
void SkipString(stream::IStream& s);
void WriteString(stream::IStreamW& s, const String& str);
void WriteString(stream::IStreamW& s, const std::string& str);
void WriteString(stream::IStreamW& s, const char* str);

// ADL hook so boost::hash<String> (used by the boost::unordered_map fields throughout jet::video)
// picks this up automatically.
inline size_t hash_value(const String& s) { return s.GetHash(); }

}

namespace std {
template <>
struct hash<jet::String> {
    size_t operator()(const jet::String& s) const { return s.GetHash(); }
};
}

#endif
