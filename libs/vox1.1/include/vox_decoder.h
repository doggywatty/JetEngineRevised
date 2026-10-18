// libs/vox1.1/include/vox_decoder.h
// A decoder knows a format; its cursor decodes one reading of a stream into 16 bit PCM. The base layout of a cursor is the
// .so's (DecoderMPC8Cursor ctor @0x5e24fc: +4 channels, +8 sample rate, +0xC bits per sample, +0x10 samples, +0x14 the decoder,
// +0x18 the stream cursor, +0x1C loop).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_H

#include "vox_memory.h"
#include "vox_stream.h"
#include "vox_types.h"

namespace vox {

class DecoderCursorInterface;

class DecoderInterface {  // sizeof 0x8
public:
    DecoderInterface() {}
    virtual ~DecoderInterface() {}
    virtual DecoderCursorInterface *CreateNewCursor(StreamCursorInterface *stream) = 0;
    virtual void DestroyCursor(DecoderCursorInterface *cursor) = 0;
    virtual int GetType() = 0;
    virtual void *GetParam() = 0;
};

class DecoderCursorInterface {  // sizeof 0x20
public:
    DecoderCursorInterface(DecoderInterface *decoder, StreamCursorInterface *stream)
        : m_pDecoder(decoder), m_pStreamCursor(stream), m_loop(false), m_isDecoderInError(false) {}
    virtual ~DecoderCursorInterface() {}
    // decodes up to `bytes` bytes of 16 bit PCM into dst, returns the number of bytes written
    virtual int Decode(void *dst, int bytes) = 0;
    // decodes into a buffer the cursor owns, when the data is already PCM in memory (AllowBufferReference)
    virtual int DecodeRef(void *&ref, int bytes) { ref = 0; return 0; }
    // goes to a sample of the track, 0 when it could
    virtual int Seek(unsigned sample) = 0;
    virtual bool HasData() = 0;
    virtual void SetLoop(bool loop) { m_loop = loop; }
    virtual void Reset() { Seek(0); }
    virtual int GetRewindLimit() { return 0; }
    virtual void Rewind(int) {}
    virtual bool AllowBufferReference() { return false; }
    virtual void SetInteractiveMusicState(const char*) {}
    // the labels of the states a native track went through since the last call (see EmitterObj::ProcessNativeData)
    virtual bool IsNative() { return false; }
    const TrackParams &GetTrackParams() const { return m_trackParams; }
    bool GetLoop() const { return m_loop; }
    bool IsInError() const { return m_isDecoderInError; }
protected:
    TrackParams m_trackParams;  // 0x4
    DecoderInterface *m_pDecoder;  // 0x14
    StreamCursorInterface *m_pStreamCursor;  // 0x18
    bool m_loop;  // 0x1C
    bool m_isDecoderInError;  // 0x1D
};

}  // namespace vox

#endif
