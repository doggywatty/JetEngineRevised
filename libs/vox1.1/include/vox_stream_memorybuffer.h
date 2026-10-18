// libs/vox1.1/include/vox_stream_memorybuffer.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_STREAM_MEMORYBUFFER_H
#define LIBS_VOX1_1_INCLUDE_VOX_STREAM_MEMORYBUFFER_H

namespace vox {
class StreamMemoryBuffer : public vox::StreamInterface {  // sizeof 0x10
public:
    StreamMemoryBuffer(vox::StreamMemoryBufferParams*);
    virtual ~StreamMemoryBuffer();
    virtual vox::StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(vox::StreamCursorInterface*);
    virtual int GetType();
    unsigned char* GetBuffer();
protected:
    unsigned char* m_buffer;  // 0x8
    bool m_isBufferOwned;  // 0xC
};
}

namespace vox {
class StreamMemoryBufferCursor : public vox::StreamCursorInterface {  // sizeof 0xC
public:
    StreamMemoryBufferCursor(vox::StreamInterface*);
    virtual int Seek(int, int);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char*, int);
    virtual int ReadRef(unsigned char*&, int);
    virtual bool EndOfStream();
    virtual bool AllowBufferReference();
protected:
    int m_cursorPosition;  // 0x8
};
}

#endif
