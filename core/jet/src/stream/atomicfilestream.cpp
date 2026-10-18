// core/jet/src/stream/atomicfilestream.cpp
#include "stream/atomicfilestream.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include "stream/streamutil.h"

#ifdef _WIN32
#include <io.h>
#define jet_fsync(f) _commit(_fileno(f))
#else
#include <unistd.h>
#define jet_fsync(f) fsync(fileno(f))
#endif

namespace jet {
namespace stream {

AtomicFileStream::AtomicFileStream(const String& path, OpenMode mode)
    : m_fullPath(GetFixedCasePath(path)), m_isMapped(false), m_isOpen(false), m_startCount(0), m_openMode(mode), m_offset(0)
{
}

AtomicFileStream::~AtomicFileStream()
{
    if (IsOpen())
        Close();
}

void AtomicFileStream::Start()
{
    if (!IsOpen())
        Open(m_fullPath, m_openMode);
    if (IsOpen())
        ++m_startCount;
}

void AtomicFileStream::Finish()
{
    if (--m_startCount)
        return;
    if (IsOpen())
        Close();
}

unsigned AtomicFileStream::Read(void* dst, unsigned size) const
{
    if (m_data.empty())
        return 0;
    unsigned n = Size() - m_offset;
    if (size < n)
        n = size;
    if (n) {
        memcpy(dst, m_data.data() + m_offset, n);
        m_offset += n;
    }
    return n;
}

void AtomicFileStream::SeekCur(int delta)
{
    s64 p = s64(m_offset) + delta;
    m_offset = unsigned(p < 0 ? 0 : p > Size() ? Size() : p);
}

void* AtomicFileStream::Map(unsigned)
{
    m_isMapped = true;
    return m_data.empty() ? nullptr : m_data.data() + m_offset;
}

bool AtomicFileStream::Open(const String& path, OpenMode mode)
{
    m_openMode = mode;
    m_fullPath = GetFixedCasePath(path);
    m_offset = 0;
    m_data.clear();
    if (mode == OpenModeDef::READ) {
        FILE* f = fopen(m_fullPath.c_str(), "rb");
        if (!f)
            return false;
        m_data.reserve(0x2000);
        u8 buf[0x400];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
            m_data.insert(m_data.end(), buf, buf + n);
        fclose(f);
    }
    m_isOpen = true;
    Touch();
    return true;
}

void AtomicFileStream::Close()
{
    m_isOpen = false;
    if (m_openMode == OpenModeDef::WRITE) {
        String tmp = m_fullPath + "_tmp";
        FILE* f = fopen(tmp.c_str(), "wb");
        bool ok = f != nullptr;
        if (f) {
            ok = m_data.empty() || fwrite(m_data.data(), 1, m_data.size(), f) == m_data.size();
            fflush(f);
            jet_fsync(f);
            fclose(f);
        }
        m_data.clear();
        m_offset = 0;
        if (!ok) {
            printf("AtomicFileStream: cannot write '%s': %s\n", m_fullPath.c_str(), strerror(errno));
            return;
        }
#ifdef _WIN32
        remove(m_fullPath.c_str());
#endif
        if (rename(tmp.c_str(), m_fullPath.c_str()) != 0)
            printf("AtomicFileStream: cannot rename '%s' to '%s': %s\n", tmp.c_str(), m_fullPath.c_str(), strerror(errno));
        return;
    }
    m_data.clear();
    m_offset = 0;
}

unsigned AtomicFileStream::Write(const void* src, unsigned size)
{
    if (!IsOpen() || m_openMode != OpenModeDef::WRITE)
        return 0;
    if (!size)
        return 0;
    unsigned end = m_offset + size;
    if (end > Size()) {
        if (end >= m_data.capacity())
            m_data.reserve(end > 999999 ? end * 300 / 256 : end * 2);
        m_data.resize(end);
    }
    memcpy(m_data.data() + m_offset, src, size);
    m_offset += size;
    return size;
}

}
}
