// libs/vox1.1/include/vox_stream_cfile.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_STREAM_CFILE_H
#define LIBS_VOX1_1_INCLUDE_VOX_STREAM_CFILE_H

namespace vox {
class StreamCFile : public vox::StreamInterface {  // sizeof 0x28
public:
    StreamCFile(const char*);
    virtual void Init();
    virtual vox::StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(vox::StreamCursorInterface*);
    virtual int GetType();
    const char *GetFileName();
    void SetSize(int);
protected:
    std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > m_filename;  // 0x8
};
}

namespace vox {
class StreamCFileCursor : public vox::StreamCursorInterface {  // sizeof 0x14
public:
    StreamCFileCursor(vox::StreamInterface*);
    virtual ~StreamCFileCursor();
    virtual void Init();
    virtual void Shutdown();
    virtual int Seek(int, int);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char*, int);
    virtual bool EndOfStream();
    bool IsValid();
protected:
    vox::FileInterface *m_fp;  // 0x8
    int m_position;  // 0xC
    bool m_isValid;  // 0x10
};
}

#endif
