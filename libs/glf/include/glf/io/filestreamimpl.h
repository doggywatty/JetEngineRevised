// libs/glf/include/glf/io/filestreamimpl.h
#ifndef LIBS_GLF_INCLUDE_GLF_IO_FILESTREAMIMPL_H
#define LIBS_GLF_INCLUDE_GLF_IO_FILESTREAMIMPL_H

namespace glf {
class FileStreamImpl : public glf::IOStream {  // sizeof 0x50
public:
    FileStreamImpl(const char*, glf::ios::openflag);
    FileStreamImpl();
    virtual ~FileStreamImpl();
    bool Open(const char*, glf::ios::openflag);
    virtual void Close();
    virtual unsigned GetSize();
    virtual bool Seek(int, glf::ios::seek);
    virtual unsigned Peek(void*, unsigned);
    bool CancelAsync(void (int, glf::Stream&, void*)*, void*);
    virtual bool CanSeek();
    virtual bool CanRead();
    virtual bool CanWrite();
    int WaitASync(unsigned);
    virtual unsigned Tell();
    bool IsEof();
    virtual unsigned Read(void*, unsigned);
    virtual unsigned ReadWithOffset(void*, unsigned, unsigned);
    virtual bool ReadASync(glf::FileStream*, void*, unsigned, unsigned, void (int, glf::Stream&, void*)*, void*);
    virtual unsigned Write(const void*, unsigned);
    glf::ios::openflag CopyPath(char*, unsigned);
    const char *GetPath(glf::ios::openflag&);
    static void DispatchDoneReads();
private:
    bool CheckCrc(const char*);
public:
    glf::FileStreamImpl::Impl &GetImpl();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mFullPath;  // 0x24
    glf::ios::openflag mFlag;  // 0x44
    glf::Atomic<int> mAsyncCount;  // 0x48
    glf::FileStreamImpl::Impl &m;  // 0x4C
};
}

namespace glf {
struct FileStreamImplBase : public glf::ios {  // sizeof 0x8
    FileStreamImplBase(glf::FileStreamImpl&);
    virtual ~FileStreamImplBase();
    virtual bool Open(const char*, glf::ios::openflag);
    virtual void Close();
    virtual bool Seek(int, glf::ios::seek);
    virtual unsigned Tell();
    virtual unsigned Read(void*, unsigned);
    virtual unsigned ReadWithOffset(void*, unsigned, unsigned);
    virtual unsigned Write(const void*, unsigned);
    glf::FileStreamImpl &mFileStream;  // 0x4
};
}

namespace glf {
struct ReadASyncThread : public glf::Thread4k {  // sizeof 0x70
    virtual void Run();
};
}

#endif
