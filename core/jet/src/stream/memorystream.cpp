// core/jet/src/stream/memorystream.cpp
#include "stream/memorystream.h"
#include <cstring>

namespace jet {
namespace stream {

MemoryStream::MemoryStream() : m_link(nullptr), m_linkSize(0), m_offset(0), m_isMapped(false), m_startCount(0) {}

unsigned MemoryStream::Read(void* dst, unsigned size) const
{
    unsigned n = Size() - m_offset;
    if (size < n)
        n = size;
    if (n) {
        memcpy(dst, Data() + m_offset, n);
        m_offset += n;
    }
    return n;
}

unsigned MemoryStream::Read1(void* dst) const
{
    if (m_offset + 1 > Size())
        return 0;
    *static_cast<u8*>(dst) = Data()[m_offset++];
    return 1;
}

unsigned MemoryStream::Read2(void* dst) const
{
    if (m_offset + 2 > Size())
        return 0;
    memcpy(dst, Data() + m_offset, 2);
    m_offset += 2;
    return 2;
}

// As in the original: a short read yields 0, reports 4 and does not advance.
unsigned MemoryStream::Read4(void* dst) const
{
    if (!Size())
        return 0;
    if (m_offset + 3 < Size()) {
        memcpy(dst, Data() + m_offset, 4);
        m_offset += 4;
    } else {
        memset(dst, 0, 4);
    }
    return 4;
}

void MemoryStream::SeekBeg(unsigned pos) { m_offset = pos < Size() ? pos : Size(); }

void MemoryStream::SeekCur(int delta)
{
    s64 p = s64(m_offset) + delta;
    m_offset = unsigned(p < 0 ? 0 : p > Size() ? Size() : p);
}

void* MemoryStream::Map(unsigned)
{
    m_isMapped = true;
    return Size() ? Data() + m_offset : nullptr;
}

unsigned MemoryStream::Write(const void* src, unsigned size)
{
    if (!size)
        return 0;
    unsigned end = m_offset + size;
    if (end > Size()) {
        GetInternalArray();
        if (end >= m_buffer.capacity())
            m_buffer.reserve(end * 2);
        m_buffer.resize(end);
    }
    memcpy(Data() + m_offset, src, size);
    m_offset += size;
    return size;
}

std::vector<u8>& MemoryStream::GetInternalArray()
{
    if (m_link) {
        m_buffer.assign(m_link, m_link + m_linkSize);
        m_link = nullptr;
        m_linkSize = 0;
    }
    return m_buffer;
}

void MemoryStream::Clear()
{
    m_buffer.clear();
    m_link = nullptr;
    m_linkSize = 0;
    m_offset = 0;
}

void MemoryStream::Link(const u8* p, unsigned size)
{
    m_buffer.clear();
    m_link = const_cast<u8*>(p);
    m_linkSize = size;
    m_offset = 0;
}

}
}
