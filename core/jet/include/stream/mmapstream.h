// core/jet/include/stream/mmapstream.h
#ifndef CORE_JET_INCLUDE_STREAM_MMAPSTREAM_H
#define CORE_JET_INCLUDE_STREAM_MMAPSTREAM_H

#include "core/pstring.h"
#include "stream/istream.h"

namespace jet {
namespace stream {

// A memory-mapped window [offset, offset + size) of a file; size 0 means to the end.
// The map is created on first Read/Map after Start() and released by the last Finish().
class MMapStream : public IStreamW {
    JET_RTTI(MMapStream, IStreamW)
public:
    explicit MMapStream(const String& path, u64 offset = 0, unsigned size = 0);
    virtual ~MMapStream();

    virtual void Start();
    virtual void Finish();
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual unsigned Read(void* dst, unsigned size) const;
    virtual unsigned Read1(void* dst) const { return Read(dst, 1); }
    virtual unsigned Read2(void* dst) const { return Read(dst, 2); }
    virtual unsigned Read4(void* dst) const { return Read(dst, 4); }
    virtual void SeekBeg(unsigned pos) { m_offset = pos < m_size ? pos : m_size; }
    virtual void SeekCur(int delta);
    virtual unsigned Tell() const { return m_offset; }
    virtual void* Map(unsigned size);
    virtual void Unmap() { m_isMapped = false; }
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const { return m_size; }
    virtual unsigned Write(const void* src, unsigned size);
    virtual bool IsChanged() const;
    virtual bool Touch();

private:
    struct Impl;
    bool SystemMOpen();
    void SystemMClose();
    u8* SystemMMap() const;
    void SystemMUnmap();

    String m_path;
    u64 m_requestedOffset;
    unsigned m_requestedSize;
    unsigned m_size;
    mutable unsigned m_offset;
    mutable u8* m_view;
    bool m_isMapped;
    bool m_isOpen;
    unsigned m_startCount;
    Impl* m_impl;
};

}
}

#endif
