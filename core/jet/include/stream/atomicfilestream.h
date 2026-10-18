// core/jet/include/stream/atomicfilestream.h
#ifndef CORE_JET_INCLUDE_STREAM_ATOMICFILESTREAM_H
#define CORE_JET_INCLUDE_STREAM_ATOMICFILESTREAM_H

#include <vector>
#include "core/pstring.h"
#include "jenum.h"
#include "stream/istream.h"

namespace jet {
namespace stream {

// Whole-file stream for saves: reads the file into memory on open; when opened for writing, the
// buffer is written to "<path>_tmp", synced, and renamed over the file on close.
class AtomicFileStream : public IStreamW {
    JET_RTTI(AtomicFileStream, IStreamW)
public:
    struct OpenModeDef {
        enum type { NONE, READ, WRITE };
    };
    typedef safe_enum<OpenModeDef> OpenMode;

    AtomicFileStream(const String& path, OpenMode mode);
    virtual ~AtomicFileStream();

    virtual void Start();
    virtual void Finish();
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual unsigned Read(void* dst, unsigned size) const;
    virtual unsigned Read1(void* dst) const { return Read(dst, 1); }
    virtual unsigned Read2(void* dst) const { return Read(dst, 2); }
    virtual unsigned Read4(void* dst) const { return Read(dst, 4); }
    virtual void SeekBeg(unsigned pos) { m_offset = pos < Size() ? pos : Size(); }
    virtual void SeekCur(int delta);
    virtual unsigned Tell() const { return m_offset; }
    virtual void* Map(unsigned);
    virtual void Unmap() { m_isMapped = false; }
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const { return unsigned(m_data.size()); }
    virtual unsigned Write(const void* src, unsigned size);
    virtual bool IsChanged() const { return false; }
    virtual bool Touch() { return true; }

    bool Open(const String& path, OpenMode mode);
    void Close();
    bool IsOpen() const { return m_isOpen; }

protected:
    String m_fullPath;
    bool m_isMapped;

private:
    bool m_isOpen;
    unsigned m_startCount;
    std::vector<u8> m_data;
    OpenMode m_openMode;
    mutable unsigned m_offset;
};

}
}

#endif
