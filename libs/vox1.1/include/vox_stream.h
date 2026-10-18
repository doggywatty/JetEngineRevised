// libs/vox1.1/include/vox_stream.h
// A stream is where the bytes of a sound come from (a file, a buffer in memory, a segment of a native file); a cursor is a
// reader of it, so several emitters can read the same stream at different positions.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_STREAM_H
#define LIBS_VOX1_1_INCLUDE_VOX_STREAM_H

#include <string>
#include "vox_filesystem.h"
#include "vox_types.h"

namespace vox {

class StreamCursorInterface;

class StreamInterface {  // sizeof 0x8 + the size
public:
    StreamInterface() : m_size(-1) {}
    virtual ~StreamInterface() {}
    virtual void Init() {}
    virtual StreamCursorInterface *CreateNewCursor() = 0;
    virtual void DestroyCursor(StreamCursorInterface *cursor) = 0;
    virtual int GetType() = 0;
    int GetSize() const { return m_size; }
protected:
    int m_size;
};

class StreamCursorInterface {  // sizeof 0x8
public:
    explicit StreamCursorInterface(StreamInterface *stream) : m_pStream(stream) {}
    virtual ~StreamCursorInterface() {}
    virtual void Init() {}
    virtual void Shutdown() {}
    virtual int Seek(int offset, int origin) = 0;
    virtual bool IsSeekable() = 0;
    virtual int Tell() = 0;
    virtual int Read(unsigned char *dst, int size) = 0;
    // a pointer into the data itself instead of a copy, for the streams that are in memory
    virtual int ReadRef(unsigned char *&ref, int size) { ref = 0; return 0; }
    virtual bool EndOfStream() = 0;
    virtual bool AllowBufferReference() { return false; }
    StreamInterface *GetStream() { return m_pStream; }
protected:
    StreamInterface *m_pStream;
};

// a file read through the file system table
class StreamCFile : public StreamInterface {  // sizeof 0x28
public:
    explicit StreamCFile(const char *filename);
    virtual void Init();
    virtual StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(StreamCursorInterface *cursor);
    virtual int GetType();
    const char *GetFileName() const { return m_filename.c_str(); }
    void SetSize(int size) { m_size = size; }
protected:
    std::string m_filename;  // 0x8
};

class StreamCFileCursor : public StreamCursorInterface {  // sizeof 0x14
public:
    explicit StreamCFileCursor(StreamInterface *stream);
    virtual ~StreamCFileCursor();
    virtual void Init();
    virtual void Shutdown();
    virtual int Seek(int offset, int origin);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char *dst, int size);
    virtual bool EndOfStream();
    bool IsValid() const { return m_isValid; }
protected:
    FileInterface *m_fp;  // 0x8
    int m_position;  // 0xC
    bool m_isValid;  // 0x10
};

struct StreamMemoryBufferParams {
    unsigned char *buffer;
    int size;
    bool ownsBuffer;  // the stream frees the buffer
    StreamMemoryBufferParams() : buffer(0), size(0), ownsBuffer(false) {}
};

class StreamMemoryBuffer : public StreamInterface {  // sizeof 0x10
public:
    explicit StreamMemoryBuffer(StreamMemoryBufferParams *params);
    virtual ~StreamMemoryBuffer();
    virtual StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(StreamCursorInterface *cursor);
    virtual int GetType();
    unsigned char *GetBuffer() { return m_buffer; }
protected:
    unsigned char *m_buffer;  // 0x8
    bool m_isBufferOwned;  // 0xC
};

class StreamMemoryBufferCursor : public StreamCursorInterface {  // sizeof 0xC
public:
    explicit StreamMemoryBufferCursor(StreamInterface *stream);
    virtual int Seek(int offset, int origin);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char *dst, int size);
    virtual int ReadRef(unsigned char *&ref, int size);
    virtual bool EndOfStream();
    virtual bool AllowBufferReference();
protected:
    int m_cursorPosition;  // 0x8
};

// a window [offset, offset + size) of the cursor of a stream (the segments of a native file are consecutive in it)
struct StreamNativeSegmentParams {
    StreamCursorInterface *source;
    int offset;
    int size;
    StreamNativeSegmentParams() : source(0), offset(0), size(0) {}
};

class StreamNativeSegment : public StreamInterface {  // sizeof 0x14
public:
    explicit StreamNativeSegment(StreamNativeSegmentParams *params);
    virtual StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(StreamCursorInterface *cursor);
    virtual int GetType();
    StreamNativeSegmentParams *GetSegmentParams() { return &m_segmentParams; }
protected:
    StreamNativeSegmentParams m_segmentParams;  // 0x8
};

class StreamNativeSegmentCursor : public StreamCursorInterface {  // sizeof 0x10
public:
    explicit StreamNativeSegmentCursor(StreamInterface *stream);
    virtual ~StreamNativeSegmentCursor();
    virtual void Shutdown();
    virtual int Seek(int offset, int origin);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char *dst, int size);
    virtual bool EndOfStream();
protected:
    StreamNativeSegmentParams *m_pSegmentParams;  // 0x8
    int m_position;  // 0xC
};

}  // namespace vox

#endif
