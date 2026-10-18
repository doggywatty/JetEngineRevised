// core/claraloader/src/clara.cpp
#include "clara.h"
#include <cctype>
#include <string>

namespace clara {

void Path::Parse(const char* s, unsigned n)
{
    m_elements.clear();
    m_path = jet::String::null;
    m_pathDirty = true;
    m_hash = 0;
    m_isAbsolute = false;
    if (!s || !n || !*s)
        return;
    bool leading = true;
    const char* seg = s;
    const char* e = s + n;
    for (const char* c = s; c != e && *c; ++c) {
        if (*c == '/') {
            if (leading)
                m_isAbsolute = true;
            if (c != seg)
                m_elements.push_back(jet::String(seg, c));
            seg = c + 1;
        }
        if (leading)
            leading = *c != char(0xff) && isspace(u8(*c));
    }
    const char* end = seg;
    while (end != e && *end)
        ++end;
    if (end != seg)
        m_elements.push_back(jet::String(seg, end));
}

void Path::Clear()
{
    m_elements.clear();
    m_pathDirty = true;
    m_isAbsolute = false;
}

void Path::Shrink(unsigned n)
{
    if (n < m_elements.size())
        m_elements.resize(n);
    m_pathDirty = true;
}

void Path::Set(const Path& a, const Path& b)
{
    if (b.m_isAbsolute) {
        *this = b;
        return;
    }
    m_elements = a.m_elements;
    for (const jet::String& s : b.m_elements) {
        if (s == "..") {
            if (!m_elements.empty())
                m_elements.pop_back();
        } else if (s != ".") {
            m_elements.push_back(s);
        }
    }
    m_isAbsolute = a.m_isAbsolute;
    m_pathDirty = true;
}

const jet::String& Path::GetAsString() const
{
    if (m_pathDirty) {
        std::string r;
        if (!m_elements.empty() && m_isAbsolute)
            r = "/";
        for (size_t i = 0; i < m_elements.size(); ++i) {
            if (i)
                r += '/';
            r += m_elements[i].c_str();
        }
        m_path = r;
        m_pathDirty = false;
    }
    return m_path;
}

bool Path::operator==(const Path& p) const
{
    if (m_isAbsolute != p.m_isAbsolute || m_elements.size() != p.m_elements.size())
        return false;
    for (size_t i = 0; i < m_elements.size(); ++i)
        if (m_elements[i] != p.m_elements[i])
            return false;
    return true;
}

void Path::GetSubPath(Path& out, unsigned start, int count) const
{
    out.Clear();
    unsigned end = count <= 0 ? GetCount() : start + unsigned(count);  // 0 = to the end
    for (unsigned i = start; i < end && i < GetCount(); ++i)
        out.m_elements.push_back(m_elements[i]);
    out.m_isAbsolute = m_isAbsolute && start == 0;
}

const jet::stream::IStream& operator>>(const jet::stream::IStream& s, Path& p)
{
    u16 n = 0;
    s >> n;
    std::string buf(n, '\0');
    if (n)
        s.Read(&buf[0], n);
    p.Parse(buf.c_str(), n);
    return s;
}

}
