// core/jet/src/stream/filestream.cpp
#include "stream/filestream.h"
#include <atomic>
#include "stream/streamutil.h"

#ifdef _WIN32
#define jet_fseek _fseeki64
#define jet_ftell _ftelli64
#else
#define jet_fseek fseeko
#define jet_ftell ftello
#endif

namespace jet {
namespace stream {

static std::atomic<int> s_openCount(0);

int FileStream::GetOpenCount() { return s_openCount; }

FileStream::FileStream(const String& path, OpenMode mode, bool useLowercase)
    : m_useLowercase(useLowercase), m_fullPath(useLowercase ? GetFixedCasePath(path) : path), m_isMapped(false), m_isOpen(false),
      m_mappedMemory(nullptr), m_startCount(0), m_offset(0), m_openMode(mode), m_file(nullptr), m_fileSize(-1)
{
}

FileStream::~FileStream()
{
    Unmap();
    if (IsOpen())
        Close();
}

void FileStream::Start()
{
    if (!IsOpen())
        Open(m_fullPath, m_openMode, m_useLowercase);
    if (IsOpen())
        ++m_startCount;
}

void FileStream::Finish()
{
    if (--m_startCount)
        return;
    if (IsOpen())
        Close();
}

unsigned FileStream::Read(void* dst, unsigned size) const
{
    if (!m_file)
        return 0;
    unsigned n = unsigned(fread(dst, 1, size, m_file));
    m_offset += n;
    return n;
}

void FileStream::SeekBeg(unsigned pos)
{
    if (m_offset != pos && m_file) {
        jet_fseek(m_file, pos, SEEK_SET);
        m_offset = pos;
    }
}

void FileStream::SeekCur(int delta)
{
    if (!delta || !m_file)
        return;
    s64 p = s64(m_offset) + delta;
    s64 size = Size();
    p = p < 0 ? 0 : p > size ? size : p;
    jet_fseek(m_file, p, SEEK_SET);
    m_offset = unsigned(p);
}

void* FileStream::Map(unsigned size)
{
    Unmap();
    m_mappedMemory = new u8[size ? size : 1];
    Read(m_mappedMemory, size);
    m_isMapped = true;
    return m_mappedMemory;
}

void FileStream::Unmap()
{
    delete[] m_mappedMemory;
    m_mappedMemory = nullptr;
    m_isMapped = false;
}

unsigned FileStream::Size() const
{
    if (m_fileSize < 0 && m_file) {
        s64 cur = jet_ftell(m_file);
        jet_fseek(m_file, 0, SEEK_END);
        m_fileSize = jet_ftell(m_file);
        jet_fseek(m_file, cur, SEEK_SET);
    }
    return m_fileSize < 0 ? 0 : unsigned(m_fileSize);
}

bool FileStream::Open(const String& path, OpenMode mode, bool useLowercase)
{
    m_useLowercase = useLowercase;
    m_fullPath = useLowercase ? GetFixedCasePath(path) : path;
    if (IsOpen())
        Close();
    m_openMode = mode;
    const char* fm = "rb";
    unsigned m = mode;
    if ((m & k_in) == m)
        fm = "rb";
    else if ((m & k_out) == m)
        fm = "wb";
    else if ((m & k_app) == m || (m & (k_out | k_app)) == m)
        fm = "ab";
    else if ((m & (k_in | k_out)) == m)
        fm = "r+b";
    else if (m & (k_trunc | k_create))
        fm = (m & k_trunc) ? "wb" : "ab";
    m_offset = 0;
    m_file = fopen(m_fullPath.c_str(), fm);
    if (!m_file && fm[0] == 'a' && (m & k_create))
        m_file = fopen(m_fullPath.c_str(), "wb");
    m_fileSize = -1;
    m_isOpen = m_file != nullptr;
    Touch();
    if (m_isOpen)
        ++s_openCount;
    return m_isOpen;
}

void FileStream::Close()
{
    if (m_isOpen) {
        fclose(m_file);
        m_file = nullptr;
        m_fileSize = -1;
        m_offset = 0;
        --s_openCount;
    }
    m_isOpen = false;
}

unsigned FileStream::Write(const void* src, unsigned size)
{
    if (!m_file)
        return 0;
    unsigned n = unsigned(fwrite(src, 1, size, m_file));
    m_offset += n;
    if (s64(m_offset) > m_fileSize)
        m_fileSize = -1;
    return n;
}

bool FileStream::IsChanged() const
{
    long t = 0;
    if (!GetFileTime(m_fullPath, t))
        return true;
    return GetTimestamp() != t;
}

bool FileStream::Touch()
{
    long t = 0;
    GetFileTime(m_fullPath, t);
    SetTimestamp(t);
    return true;
}

}
}
