// libs/glf/include/glf/io/stream.h
#ifndef LIBS_GLF_INCLUDE_GLF_IO_STREAM_H
#define LIBS_GLF_INCLUDE_GLF_IO_STREAM_H

struct Stream {  // sizeof 0x4
    virtual long QueryCb() = 0;
    virtual int Read(long, void*, long*) = 0;
    virtual int Write(long, void*, long) = 0;
    virtual int Replace(void*, long) = 0;
    virtual int Append(void*, long) = 0;
    virtual int Delete() = 0;
    virtual int Release() = 0;
    virtual int Read2(long, void*, long) = 0;
    virtual int Truncate(long) = 0;
};

namespace glf {
class IOStream : public glf::Stream {  // sizeof 0x24
public:
    virtual unsigned GetSize() = 0;
    virtual bool CanSeek() = 0;
    virtual bool Seek(int, glf::ios::seek);
    virtual unsigned Tell() = 0;
    virtual unsigned Peek(void*, unsigned);
    bool IsEof();
protected:
    IOStream();
    glf::IOStream::FilePosition mFilePosition;  // 0x1C
};
}

namespace glf {
class Stream : private glf::NonCopyable, public glf::ios {  // sizeof 0x1C
public:
    virtual ~Stream();
    glf::fs::Error GetLastError();
    virtual void Close() = 0;
    virtual unsigned Read(void*, unsigned);
    virtual unsigned ReadWithOffset(void*, unsigned, unsigned);
    virtual unsigned Write(const void*, unsigned);
    virtual bool ReadASync(void*, unsigned, unsigned, void (int, glf::Stream&, void*)*, void*);
    virtual unsigned GetOffsetAlign();
    virtual unsigned GetSizeAlign();
    virtual unsigned GetBufferAlign();
    virtual bool CanWrite() = 0;
    virtual bool CanRead() = 0;
    int WaitASync(unsigned);
    virtual bool IsOpened();
    bool IsBusy();
protected:
    Stream();
    glf::fs::Error mLastError;  // 0x8
    bool mIsOpened;  // 0xC
    volatile bool mIsBusy;  // 0xD
    int mCallbackResult;  // 0x10
    void (*mCallback)(int, glf::Stream&, void*);  // 0x14
    void* mCallbackArg;  // 0x18
};
}

#endif
