// libs/vox1.1/include/vox_decoder_mswav.h
// Microsoft RIFF WAVE files (".wav"): PCM, MS ADPCM (tag 2) and IMA ADPCM (tag 0x11). The decoder reads the chunks once
// (WaveChunk); each cursor has a sub decoder of the format.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_MSWAV_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_MSWAV_H

#include <vector>
#include "adpcmstate.h"
#include "vox_decoder.h"

namespace vox {

// the extra data of the "fmt " chunk of an MS ADPCM file
struct FmtExtendedInfos {  // sizeof 0x40
    unsigned short samplesPerBlock;
    unsigned short numCoefficients;
    short coefficients[7][2];
    FmtExtendedInfos() : samplesPerBlock(0), numCoefficients(0) { memset(coefficients, 0, sizeof(coefficients)); }
};

// a "data" chunk of the file
struct DataNode {
    unsigned offset;
    unsigned size;
};

// what the chunks of a file say (the .so's WaveChunk: the "fmt " chunk and the list of the "data" chunks)
struct WaveChunk {  // sizeof 0x3C
    unsigned short formatTag;
    unsigned short channels;
    unsigned sampleRate;
    unsigned avgBytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    FmtExtendedInfos extended;
    std::vector<DataNode> dataNodes;
    unsigned totalDataSize;
    unsigned factSamples;  // the "fact" chunk, 0 when there is none
    WaveChunk() : formatTag(0), channels(0), sampleRate(0), avgBytesPerSec(0), blockAlign(0), bitsPerSample(0), totalDataSize(0), factSamples(0) {}
};

class VoxMSWavSubDecoder {  // sizeof 0x2C
public:
    VoxMSWavSubDecoder(StreamCursorInterface *stream, WaveChunk *chunks);
    virtual ~VoxMSWavSubDecoder();
    const TrackParams &GetTrackParams() const { return m_trackParams; }
    void SetLoop(bool loop) { m_loop = loop; }
    virtual int Decode(void *dst, int bytes) = 0;
    virtual int Seek(unsigned sample) = 0;
    virtual bool HasData() = 0;
protected:
    // reads size bytes at the position `pos` of the data (the "data" chunks as one stream of bytes)
    int ReadData(unsigned pos, unsigned char *dst, int size);
    StreamCursorInterface *m_pStreamCursor;  // 0x4
    WaveChunk *m_pWaveChunks;  // 0x8
    TrackParams m_trackParams;  // 0x10
    unsigned m_decodedSamples;  // 0x24
    bool m_loop;  // 0x28
    bool m_isDecoderInError;  // 0x29
};

// ".wav" PCM
class VoxMSWavSubDecoderPCM : public VoxMSWavSubDecoder {  // sizeof 0x2C
public:
    VoxMSWavSubDecoderPCM(StreamCursorInterface *stream, WaveChunk *chunks);
    virtual ~VoxMSWavSubDecoderPCM();
    virtual int Decode(void *dst, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
private:
    int GetNbSamples() const;
    unsigned m_position;  // in bytes of the data
};

// blocks of blockAlign bytes decoded to samplesPerBlock samples; the base of the two ADPCM ones
class VoxMSWavSubDecoderBlock : public VoxMSWavSubDecoder {
public:
    VoxMSWavSubDecoderBlock(StreamCursorInterface *stream, WaveChunk *chunks, int samplesPerBlock);
    virtual ~VoxMSWavSubDecoderBlock();
    virtual int Decode(void *dst, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
protected:
    void ComputeNumSamples();  // called once by the constructor of the format
    // decodes the block of `bytes` bytes (blockAlign, less for the last one) at m_totalDataBytesRead into m_samples
    // (interleaved), returns the samples of it
    virtual int DecodeBlock(int bytes) = 0;
    // the samples a block of `bytes` bytes holds
    virtual int SamplesInBlock(int bytes) = 0;
    unsigned char *m_blockReadBuffer;
    std::vector<short> m_samples;
    int m_samplesPerBlock;
    unsigned m_totalDataBytesRead;  // the position of the block to decode next, in bytes of the data
    int m_samplesInBuffer;
    int m_samplesInBufferConsumed;
    unsigned m_totalSampleDecoded;
};

class VoxMSWavSubDecoderMSADPCM : public VoxMSWavSubDecoderBlock {  // sizeof 0x450
public:
    VoxMSWavSubDecoderMSADPCM(StreamCursorInterface *stream, WaveChunk *chunks);
    virtual ~VoxMSWavSubDecoderMSADPCM();
protected:
    virtual int DecodeBlock(int bytes);
    virtual int SamplesInBlock(int bytes);
private:
    struct MsAdpcmState { int coef1, coef2, delta, sample1, sample2; };
    short DecodeSample(MsAdpcmState *state, unsigned nibble);
};

class VoxMSWavSubDecoderIMAADPCM : public VoxMSWavSubDecoderBlock {  // sizeof 0x88
public:
    VoxMSWavSubDecoderIMAADPCM(StreamCursorInterface *stream, WaveChunk *chunks);
    virtual ~VoxMSWavSubDecoderIMAADPCM();
protected:
    virtual int DecodeBlock(int bytes);
    virtual int SamplesInBlock(int bytes);
private:
    AdpcmState m_states[8];  // 0x2C
};

class DecoderMSWav : public DecoderInterface {  // sizeof 0x44
public:
    DecoderMSWav();
    virtual ~DecoderMSWav();
    virtual DecoderCursorInterface *CreateNewCursor(StreamCursorInterface *stream);
    virtual void DestroyCursor(DecoderCursorInterface *cursor);
    virtual int GetType();
    virtual void *GetParam();
    WaveChunk *GetWaveChunks() { return &m_waveChunks; }
    bool NeedParsing() const { return m_needParsing; }
    void SetParsed() { m_needParsing = false; }
private:
    WaveChunk m_waveChunks;  // 0x4
    bool m_needParsing;  // 0x40
};

class DecoderMSWavCursor : public DecoderCursorInterface {  // sizeof 0x28
public:
    DecoderMSWavCursor(DecoderInterface *decoder, StreamCursorInterface *stream);
    virtual ~DecoderMSWavCursor();
    virtual int Decode(void *dst, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
    virtual void SetLoop(bool loop);
private:
    bool ParseFile();
    WaveChunk *m_pWaveChunks;  // 0x20
    VoxMSWavSubDecoder *m_pSubDecoder;  // 0x24
};

// @0x5e2be4 (DecoderMSWavFactory): the type 1 of VoxEngine::RegisterDecoderType
DecoderInterface *DecoderMSWavFactory(void *params);

}  // namespace vox

#endif
