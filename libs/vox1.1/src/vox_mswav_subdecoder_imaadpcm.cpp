// libs/vox1.1/src/vox_mswav_subdecoder_imaadpcm.cpp
// IMA ADPCM in WAVE blocks: every block starts with, per channel, a 16 bit predictor and a step index (that is also its first
// sample); the nibbles follow in runs of 4 bytes (8 samples, low nibble first) per channel in turn
#include "adpcmdecoder.h"
#include "vox_decoder_mswav.h"

namespace vox {

static int IMASamplesPerBlock(WaveChunk *c) {
    if (c->extended.samplesPerBlock) return c->extended.samplesPerBlock;
    return c->channels ? 1 + (int)(c->blockAlign - 4 * c->channels) * 2 / c->channels : 0;
}

VoxMSWavSubDecoderIMAADPCM::VoxMSWavSubDecoderIMAADPCM(StreamCursorInterface *stream, WaveChunk *chunks)
    : VoxMSWavSubDecoderBlock(stream, chunks, IMASamplesPerBlock(chunks)) {
    ComputeNumSamples();
}

int VoxMSWavSubDecoderIMAADPCM::SamplesInBlock(int bytes) {
    const int ch = m_pWaveChunks->channels;
    if (bytes < 4 * ch) return 0;
    const int n = 1 + (bytes - 4 * ch) * 2 / ch;
    return n < m_samplesPerBlock ? n : m_samplesPerBlock;
}

VoxMSWavSubDecoderIMAADPCM::~VoxMSWavSubDecoderIMAADPCM() {}

int VoxMSWavSubDecoderIMAADPCM::DecodeBlock(int blockAlign) {
    const int channels = m_pWaveChunks->channels;
    if (channels < 1 || channels > 8 || blockAlign < 4 * channels || ReadData(m_totalDataBytesRead, m_blockReadBuffer, blockAlign) != blockAlign) return 0;
    short *out = &m_samples[0];
    const unsigned char *p = m_blockReadBuffer;
    for (int c = 0; c < channels; c++) {
        m_states[c].SetPrevSample((short)(p[0] | (p[1] << 8)));
        m_states[c].SetPrevIndex(p[2] > 88 ? 88 : p[2]);
        out[c] = m_states[c].GetPrevSample();
        p += 4;
    }
    int n = 1;
    const unsigned char *end = m_blockReadBuffer + blockAlign;
    while (p + 4 * channels <= end && n < m_samplesPerBlock) {
        const int run = m_samplesPerBlock - n < 8 ? m_samplesPerBlock - n : 8;
        for (int c = 0; c < channels; c++) AdpcmDecoder::DecodeAdpcm(run, &m_states[c], out + n * channels + c, p + 4 * c, channels);
        p += 4 * channels;
        n += run;
    }
    return n;
}

}  // namespace vox
