// core/jet/include/stream/filestream.h
#ifndef CORE_JET_INCLUDE_STREAM_FILESTREAM_H
#define CORE_JET_INCLUDE_STREAM_FILESTREAM_H

#include <cstdio>
#include "core/pstring.h"
#include "stream/istream.h"

namespace jet {
namespace stream {

// Opened lazily on Start(), closed when the last Finish() balances it.
class FileStream : public IStreamW {
    JET_RTTI(FileStream, IStreamW)
public:
    enum OpenMode { k_in = 1, k_out = 2, k_ate = 4, k_app = 8, k_trunc = 16, k_create = 32 };

    FileStream(const String& path, OpenMode mode, bool useLowercase);
    virtual ~FileStream();

    virtual void Start();
    virtual void Finish();
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual unsigned Read(void* dst, unsigned size) const;
    virtual unsigned Read1(void* dst) const { return Read(dst, 1); }
    virtual unsigned Read2(void* dst) const { return Read(dst, 2); }
    virtual unsigned Read4(void* dst) const { return Read(dst, 4); }
    virtual void SeekBeg(unsigned pos);
    virtual void SeekCur(int delta);
    virtual unsigned Tell() const { return m_offset; }
    virtual void* Map(unsigned size);
    virtual void Unmap();
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const;
    virtual unsigned Write(const void* src, unsigned size);
    virtual bool IsChanged() const;
    virtual bool Touch();

    bool Open(const String& path, OpenMode mode, bool useLowercase);
    void Close();
    bool IsOpen() const { return m_isOpen; }
    const String& GetPath() const { return m_fullPath; }

    static int GetOpenCount();

protected:
    bool m_useLowercase;
    String m_fullPath;
    bool m_isMapped;

private:
    bool m_isOpen;
    u8* m_mappedMemory;
    unsigned m_startCount;
    mutable unsigned m_offset;
    OpenMode m_openMode;
    FILE* m_file;
    mutable s64 m_fileSize;
};

}
}

#endif
