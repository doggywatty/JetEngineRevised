// libs/glf/include/glf/io/filestreammgr.h
#ifndef LIBS_GLF_INCLUDE_GLF_IO_FILESTREAMMGR_H
#define LIBS_GLF_INCLUDE_GLF_IO_FILESTREAMMGR_H

namespace glf {
class FileStreamMgr {  // sizeof 0x40
public:
    FileStreamMgr();
    ~FileStreamMgr();
    void FileOutAllFiles();
private:
    bool Open(glf::FileStream*, const char*, glf::ios::openflag);
    void Close(glf::FileStream*);
    void Ensure(glf::FileStream*);
    void Insert(glf::FileStream*);
    unsigned __int64 IncrCounter();
    static const unsigned MAX_FILESTREAM;
    std::vector<glf::FileStream *,std::allocator<glf::FileStream *> > mActiveList;  // 0x0
    std::vector<glf::FileStream *,std::allocator<glf::FileStream *> > mInactiveList;  // 0x14
    glf::GenericRecursiveMutex<glf::SpinLock> mMutex;  // 0x28
    unsigned __int64 mCounter;  // 0x38
};
}

#endif
