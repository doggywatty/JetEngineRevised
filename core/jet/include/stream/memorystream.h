// core/jet/include/stream/memorystream.h
#ifndef CORE_JET_INCLUDE_STREAM_MEMORYSTREAM_H
#define CORE_JET_INCLUDE_STREAM_MEMORYSTREAM_H

#include <vector>
#include "stream/istream.h"

namespace jet {
namespace stream {

// Growable in-memory stream over m_buffer, or over linked external memory (see MemLinkStream).
class MemoryStream : public IStreamW {
    JET_RTTI(MemoryStream, IStreamW)
public:
    MemoryStream();
    virtual ~MemoryStream() {}

    virtual void Start() { ++m_startCount; }
    virtual void Finish() { --m_startCount; }
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual unsigned Read(void* dst, unsigned size) const;
    virtual unsigned Read1(void* dst) const;
    virtual unsigned Read2(void* dst) const;
    virtual unsigned Read4(void* dst) const;
    virtual void SeekBeg(unsigned pos);
    virtual void SeekCur(int delta);
    virtual unsigned Tell() const { return m_offset; }
    virtual void* Map(unsigned size);
    virtual void Unmap() { m_isMapped = false; }
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const { return m_link ? m_linkSize : unsigned(m_buffer.size()); }
    virtual bool IsChanged() const { return false; }
    virtual bool Touch() { return true; }
    virtual unsigned Write(const void* src, unsigned size);

    std::vector<u8>& GetInternalArray();
    void Clear();

protected:
    void Link(const u8* p, unsigned size);

    u8* Data() const { return m_link ? m_link : const_cast<u8*>(m_buffer.data()); }

    std::vector<u8> m_buffer;
    u8* m_link;
    unsigned m_linkSize;
    mutable unsigned m_offset;
    bool m_isMapped;
    unsigned m_startCount;
};

}
}

#endif
