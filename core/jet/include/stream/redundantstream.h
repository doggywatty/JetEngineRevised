// core/jet/include/stream/redundantstream.h
#ifndef CORE_JET_INCLUDE_STREAM_REDUNDANTSTREAM_H
#define CORE_JET_INCLUDE_STREAM_REDUNDANTSTREAM_H

#include <memory>
#include "stream/memorystream.h"
#include "stream/substream.h"

namespace jet {
namespace stream {

// Writes the payload N times (2..100), each copy framed by signature blocks that carry its CRC and size,
// so a save survives partial corruption. Reading finds the first copy whose CRC matches.
class RedundantStream : public IStreamW {
    JET_RTTI(RedundantStream, IStreamW)
public:
    RedundantStream() : m_source(nullptr), m_destination(nullptr), m_redundantCopyCount(0), m_strictDecoding(false) {}
    virtual ~RedundantStream() {}

    void SetSource(IStream& src, bool strict);
    void SetDestination(IStreamW& dst, unsigned copies);
    unsigned GetRedundantCopyCount() const { return m_redundantCopyCount; }

    virtual void Start();
    virtual void Finish();
    virtual bool IsStarted() const;
    virtual unsigned Read(void* dst, unsigned size) const { return IsStarted() ? m_subStream->Read(dst, size) : 0; }
    virtual unsigned Read1(void* dst) const { return Read(dst, 1); }
    virtual unsigned Read2(void* dst) const { return Read(dst, 2); }
    virtual unsigned Read4(void* dst) const { return Read(dst, 4); }
    virtual void SeekBeg(unsigned pos) { if (IsStarted()) Active()->SeekBeg(pos); }
    virtual void SeekCur(int delta) { if (IsStarted()) Active()->SeekCur(delta); }
    virtual unsigned Tell() const { return IsStarted() ? Active()->Tell() : 0; }
    virtual void* Map(unsigned size) { return IsStarted() && IsForReading() ? m_subStream->Map(size) : nullptr; }
    virtual void Unmap() { if (IsStarted() && IsForReading()) m_subStream->Unmap(); }
    virtual bool IsMapped() const;
    virtual unsigned Size() const { return IsStarted() ? Active()->Size() : 0; }
    virtual unsigned Write(const void* src, unsigned size) { return IsStarted() && !IsForReading() ? m_bufferStream->Write(src, size) : 0; }
    virtual bool IsChanged() const { return IsStarted() ? Active()->IsChanged() : true; }
    virtual bool Touch() { return IsStarted() ? Active()->Touch() : false; }

private:
    bool IsForReading() const { return m_source != nullptr; }
    IStreamW* Active() const { return IsForReading() ? static_cast<IStreamW*>(m_subStream.get()) : m_bufferStream.get(); }
    bool OpenForReading();
    bool OpenForWriting();
    void CloseForReading() { m_subStream.reset(); }
    void CloseForWriting();

    IStream* m_source;
    IStreamW* m_destination;
    std::unique_ptr<SubStream> m_subStream;
    std::unique_ptr<MemoryStream> m_bufferStream;
    unsigned m_redundantCopyCount;
    bool m_strictDecoding;
};

}
}

#endif
