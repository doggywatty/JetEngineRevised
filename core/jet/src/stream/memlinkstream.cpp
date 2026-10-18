// core/jet/src/stream/memlinkstream.cpp
#include "stream/memlinkstream.h"
#include <cstring>

namespace jet {
namespace stream {

unsigned MemLinkStream::Write(const void* src, unsigned size)
{
    if (!m_canWrite)
        return 0;
    if (m_offset + size > Size())
        size = Size() - m_offset;
    if (size) {
        memcpy(Data() + m_offset, src, size);
        m_offset += size;
    }
    return size;
}

}
}
