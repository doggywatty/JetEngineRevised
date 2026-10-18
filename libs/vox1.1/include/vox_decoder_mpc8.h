// libs/vox1.1/include/vox_decoder_mpc8.h
// Musepack SV8 (".mpc"): the stock libmpcdec (libs/vox1.1/{include,src}/mpc) reading from a vox stream cursor.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_MPC8_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_MPC8_H

#include <mpc/mpcdec.h>
#include "vox_decoder.h"

namespace vox {

struct DecoderMPC8Params {
    int forceSampleRate;  // the rate the track is said to have, 0 or less for the one of the file
};

class DecoderMPC8 : public DecoderInterface {  // sizeof 0x8
public:
    explicit DecoderMPC8(DecoderMPC8Params *params);
    virtual DecoderCursorInterface *CreateNewCursor(StreamCursorInterface *stream);
    virtual void DestroyCursor(DecoderCursorInterface *cursor);
    virtual int GetType();
    virtual void *GetParam();
    int GetForceSampleRate() const { return m_forceSampleRate; }
private:
    int m_forceSampleRate;  // 0x4
};

class DecoderMPC8Cursor : public DecoderCursorInterface {  // sizeof 0x50
public:
    DecoderMPC8Cursor(DecoderInterface *decoder, StreamCursorInterface *stream);
    virtual ~DecoderMPC8Cursor();
    virtual int Decode(void *dst, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
protected:
    void ConvertFloatToShort(short *dst, const float *src, int count);
    bool CheckForEndOfSample();
    unsigned m_totalSampleDecoded;  // 0x20
    int m_samplesInBuffer;  // 0x24
    int m_samplesInBufferConsumed;  // 0x28
    mpc_demux *m_pDemux;  // 0x2C
    mpc_reader m_reader;  // 0x30
    float *m_sampleBuffer;  // 0x48
    mpc_status m_error;  // 0x4C
};

// @0x5e2418 (DecoderMPC8Factory): the type 3 of VoxEngine::RegisterDecoderType
DecoderInterface *DecoderMPC8Factory(void *params);

}  // namespace vox

#endif
