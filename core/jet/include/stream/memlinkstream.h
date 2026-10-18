// core/jet/include/stream/memlinkstream.h
#ifndef CORE_JET_INCLUDE_STREAM_MEMLINKSTREAM_H
#define CORE_JET_INCLUDE_STREAM_MEMLINKSTREAM_H

#include "stream/memorystream.h"

namespace jet {
namespace stream {

// A MemoryStream over memory it does not own. Writes never grow it.
class MemLinkStream : public MemoryStream {
    JET_RTTI(MemLinkStream, MemoryStream)
public:
    MemLinkStream() : m_canWrite(true) {}
    MemLinkStream(u8* p, unsigned size) : m_canWrite(true) { Link(p, size); }
    MemLinkStream(const u8* p, unsigned size) : m_canWrite(false) { Link(p, size); }
    MemLinkStream(std::vector<u8>& v) : m_canWrite(true) { Link(v.data(), unsigned(v.size())); }
    MemLinkStream(const std::vector<u8>& v) : m_canWrite(false) { Link(v.data(), unsigned(v.size())); }

    virtual unsigned Write(const void* src, unsigned size);
    void Link(const u8* p, unsigned size) { MemoryStream::Link(p, size); m_canWrite = false; }
    void Link(u8* p, unsigned size) { MemoryStream::Link(p, size); m_canWrite = true; }

private:
    bool m_canWrite;
};

}
}

#endif
