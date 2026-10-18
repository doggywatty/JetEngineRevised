// libs/vox1.1/include/vox_decoder_raw.h
// 16 bit PCM as it is: the decoder of a sound that has been decoded to memory ("load and decode", ConvertToRawSource)
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_RAW_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_RAW_H

#include "vox_decoder.h"

namespace vox {

class DecoderRaw : public DecoderInterface {  // sizeof 0x14
public:
    explicit DecoderRaw(const TrackParams *params);
    virtual DecoderCursorInterface *CreateNewCursor(StreamCursorInterface *stream);
    virtual void DestroyCursor(DecoderCursorInterface *cursor);
    virtual int GetType();
    virtual void *GetParam();
    const TrackParams &GetTrackParams() const { return m_trackParams; }
protected:
    TrackParams m_trackParams;  // 0x4
};

class DecoderRawCursor : public DecoderCursorInterface {  // sizeof 0x24
public:
    DecoderRawCursor(DecoderInterface *decoder, StreamCursorInterface *stream);
    virtual ~DecoderRawCursor();
    virtual int Decode(void *dst, int bytes);
    virtual int DecodeRef(void *&ref, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
    virtual bool AllowBufferReference();
};

// the type 0 of VoxEngine::RegisterDecoderType (params: a TrackParams)
DecoderInterface *DecoderRawFactory(void *params);

}  // namespace vox

#endif
