// libs/vox1.1/include/vox_stream_native_segment.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_STREAM_NATIVE_SEGMENT_H
#define LIBS_VOX1_1_INCLUDE_VOX_STREAM_NATIVE_SEGMENT_H

namespace vox {
class StreamNativeSegment : public vox::StreamInterface {  // sizeof 0x14
public:
    StreamNativeSegment(vox::StreamNativeSegmentParams*);
    virtual vox::StreamCursorInterface *CreateNewCursor();
    virtual void DestroyCursor(vox::StreamCursorInterface*);
    virtual int GetType();
    vox::StreamNativeSegmentParams *GetSegmentParams();
protected:
    vox::StreamNativeSegmentParams m_segmentParams;  // 0x8
};
}

namespace vox {
class StreamNativeSegmentCursor : public vox::StreamCursorInterface {  // sizeof 0x10
public:
    StreamNativeSegmentCursor(vox::StreamInterface*);
    virtual ~StreamNativeSegmentCursor();
    virtual void Shutdown();
    virtual int Seek(int, int);
    virtual bool IsSeekable();
    virtual int Tell();
    virtual int Read(unsigned char*, int);
    virtual bool EndOfStream();
protected:
    vox::StreamNativeSegmentParams *m_pSegmentParams;  // 0x8
    int m_position;  // 0xC
};
}

#endif
