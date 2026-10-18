// core/jet/src/stream/mmapstream.cpp
#include "stream/mmapstream.h"
#include <cstring>
#include "stream/streamutil.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace jet {
namespace stream {

struct MMapStream::Impl {
#ifdef _WIN32
    HANDLE file = INVALID_HANDLE_VALUE;
    HANDLE mapping = nullptr;
#else
    int fd = -1;
#endif
    u64 alignedOffset = 0;  // map start, page aligned
    unsigned delta = 0;     // requested offset - aligned offset
    size_t length = 0;      // mapped bytes
    void* base = nullptr;
};

static unsigned PageAlignment()
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwAllocationGranularity;
#else
    return unsigned(sysconf(_SC_PAGESIZE));
#endif
}

MMapStream::MMapStream(const String& path, u64 offset, unsigned size)
    : m_path(GetFixedCasePath(path)), m_requestedOffset(offset), m_requestedSize(size), m_size(0), m_offset(0), m_view(nullptr),
      m_isMapped(false), m_isOpen(false), m_startCount(0), m_impl(new Impl)
{
    Touch();
}

MMapStream::~MMapStream()
{
    SystemMUnmap();
    SystemMClose();
    delete m_impl;
}

bool MMapStream::SystemMOpen()
{
    u64 fileSize = 0;
#ifdef _WIN32
    m_impl->file = CreateFileA(m_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_impl->file == INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER li;
    GetFileSizeEx(m_impl->file, &li);
    fileSize = u64(li.QuadPart);
#else
    m_impl->fd = open(m_path.c_str(), O_RDONLY);
    if (m_impl->fd < 0)
        return false;
    struct stat st;
    if (fstat(m_impl->fd, &st) != 0) {
        SystemMClose();
        return false;
    }
    fileSize = u64(st.st_size);
#endif
    unsigned size = m_requestedSize ? m_requestedSize : unsigned(fileSize - m_requestedOffset);
    if (m_requestedOffset + size > fileSize) {
        SystemMClose();
        return false;
    }
    m_impl->alignedOffset = m_requestedOffset - m_requestedOffset % PageAlignment();
    m_impl->delta = unsigned(m_requestedOffset - m_impl->alignedOffset);
    m_impl->length = size + m_impl->delta;
    m_size = size;
    return true;
}

void MMapStream::SystemMClose()
{
#ifdef _WIN32
    if (m_impl->mapping)
        CloseHandle(m_impl->mapping);
    if (m_impl->file != INVALID_HANDLE_VALUE)
        CloseHandle(m_impl->file);
    m_impl->mapping = nullptr;
    m_impl->file = INVALID_HANDLE_VALUE;
#else
    if (m_impl->fd >= 0)
        close(m_impl->fd);
    m_impl->fd = -1;
#endif
}

u8* MMapStream::SystemMMap() const
{
    if (!m_impl->length)
        return nullptr;
#ifdef _WIN32
    if (!m_impl->mapping)
        m_impl->mapping = CreateFileMappingA(m_impl->file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!m_impl->mapping)
        return nullptr;
    m_impl->base = MapViewOfFile(m_impl->mapping, FILE_MAP_READ, DWORD(m_impl->alignedOffset >> 32), DWORD(m_impl->alignedOffset),
                                 m_impl->length);
#else
    if (m_impl->fd < 0)
        return nullptr;
    void* p = mmap(nullptr, m_impl->length, PROT_READ, MAP_SHARED, m_impl->fd, off_t(m_impl->alignedOffset));
    m_impl->base = p == MAP_FAILED ? nullptr : p;
#endif
    return m_impl->base ? static_cast<u8*>(m_impl->base) + m_impl->delta : nullptr;
}

void MMapStream::SystemMUnmap()
{
    if (!m_impl->base)
        return;
#ifdef _WIN32
    UnmapViewOfFile(m_impl->base);
#else
    munmap(m_impl->base, m_impl->length);
#endif
    m_impl->base = nullptr;
    m_view = nullptr;
}

void MMapStream::Start()
{
    if (!m_isOpen)
        m_isOpen = SystemMOpen();
    if (m_isOpen)
        ++m_startCount;
}

void MMapStream::Finish()
{
    if (--m_startCount)
        return;
    SystemMUnmap();
    if (m_isOpen)
        SystemMClose();
    m_isOpen = false;
}

unsigned MMapStream::Read(void* dst, unsigned size) const
{
    if (!m_view && !(m_view = SystemMMap()))
        return 0;
    unsigned n = m_size - m_offset;
    if (size < n)
        n = size;
    if (n) {
        memcpy(dst, m_view + m_offset, n);
        m_offset += n;
    }
    return n;
}

void MMapStream::SeekCur(int delta)
{
    s64 p = s64(m_offset) + delta;
    m_offset = unsigned(p < 0 ? 0 : p > m_size ? m_size : p);
}

void* MMapStream::Map(unsigned)
{
    if (!m_view && !(m_view = SystemMMap()))
        return nullptr;
    m_isMapped = true;
    return m_view + m_offset;
}

unsigned MMapStream::Write(const void*, unsigned) { return 0; }

bool MMapStream::IsChanged() const
{
    long t = 0;
    return !GetFileTime(m_path, t) || GetTimestamp() != t;
}

bool MMapStream::Touch()
{
    long t = 0;
    GetFileTime(m_path, t);
    SetTimestamp(t);
    return true;
}

}
}
