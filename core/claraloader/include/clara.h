// core/claraloader/include/clara.h
#ifndef CORE_CLARALOADER_INCLUDE_CLARA_H
#define CORE_CLARALOADER_INCLUDE_CLARA_H

#include <cstring>
#include <vector>
#include "core/pstring.h"
#include "stream/istream.h"

// Clara: Gameloft's level/data editor format (.bclara levels, .blibclara libraries).
namespace clara {

class Project;
class ClaraFile;
class DataEntity;
class Entity;
class Folder;
class Group;
class Movie;
class MultiLayer;
class Param;
class Template;
class Type;

// "a/b/c" or "/a/b/c" (absolute when a '/' comes before any non-space character).
class Path {
public:
    Path() : m_pathDirty(true), m_hash(0), m_isAbsolute(false) {}
    Path(const char* s) : Path() { Parse(s, s ? unsigned(strlen(s)) : 0); }
    Path(const jet::String& s) : Path() { Parse(s.c_str(), s.size()); }
    Path(const Path& a, const Path& b) : Path() { Set(a, b); }

    Path& operator=(const char* s) { Parse(s, s ? unsigned(strlen(s)) : 0); return *this; }
    Path& operator=(const jet::String& s) { Parse(s.c_str(), s.size()); return *this; }
    const jet::String& operator[](int i) const { return m_elements[i]; }
    Path operator+(const Path& p) const { return Path(*this, p); }
    Path operator+(const char* s) const { return Path(*this, Path(s)); }
    Path operator+(const jet::String& s) const { return Path(*this, Path(s)); }
    Path& operator+=(const Path& p) { return *this = *this + p; }
    Path& operator+=(const char* s) { return *this = *this + s; }
    Path& operator+=(const jet::String& s) { return *this = *this + s; }
    bool operator==(const Path& p) const;
    bool operator!=(const Path& p) const { return !(*this == p); }

    void Clear();
    void Shrink(unsigned n);
    void Set(int i, const jet::String& s) { m_elements[i] = s; m_pathDirty = true; }
    void Set(const Path& a, const Path& b);
    const jet::String& GetAsString() const;
    bool IsAbsolute() const { return m_isAbsolute; }
    unsigned GetCount() const { return unsigned(m_elements.size()); }
    const jet::String& Get(int i) const { return m_elements[i]; }
    void GetSubPath(Path& out, unsigned start, int count) const;
    Path GetSubPath(unsigned start, int count) const { Path p; GetSubPath(p, start, count); return p; }
    bool IsEmpty() const { return m_elements.empty(); }

    void Parse(const char* s, unsigned n);

private:
    std::vector<jet::String> m_elements;
    mutable jet::String m_path;
    mutable bool m_pathDirty;
    unsigned m_hash;
    bool m_isAbsolute;
};

const jet::stream::IStream& operator>>(const jet::stream::IStream& s, Path& p);

}

#endif
