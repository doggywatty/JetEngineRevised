// core/jet/include/stream/substream.h
#ifndef CORE_JET_INCLUDE_STREAM_SUBSTREAM_H
#define CORE_JET_INCLUDE_STREAM_SUBSTREAM_H

#include "stream/istream.h"

namespace jet {
namespace stream {

// A window [offset, offset + size) onto another stream.
class SubStream : public IStreamW {
    JET_RTTI(SubStream, IStreamW)
public:
    SubStream();
    virtual ~SubStream() {}
    void SetDestination(IStreamW& dst, unsigned offset, unsigned size);
    void SetSource(IStream& src, unsigned offset, unsigned size);

    virtual void Start();
    virtual void Finish();
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual void SeekBeg(unsigned pos);
    virtual void SeekCur(int delta);
    virtual unsigned Tell() const { return m_offset; }
    virtual void* Map(unsigned) { return nullptr; }
    virtual void Unmap() {}
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const { return m_size; }
    virtual unsigned Read(void* dst, unsigned size) const;
    virtual unsigned Read1(void* dst) const { return Read(dst, 1); }
    virtual unsigned Read2(void* dst) const { return Read(dst, 2); }
    virtual unsigned Read4(void* dst) const { return Read(dst, 4); }
    virtual unsigned Write(const void* src, unsigned size);
    virtual bool IsChanged() const;
    virtual bool Touch();

private:
    IStream* Inner() const { return m_source ? m_source : m_destination; }

    IStream* m_source;
    IStreamW* m_destination;
    unsigned m_streamOffset;
    unsigned m_size;
    mutable unsigned m_offset;
    unsigned m_startCount;
    bool m_isMapped;
};

}
}

#endif
