// libs/vox1.1/src/vox_mswav_subdecoder_msadpcm.cpp
// Microsoft ADPCM: every block starts with, per channel, the index of a pair of coefficients, the delta and the two first
// samples; the rest are nibbles (high nibble first, the channels alternate for stereo)
#include "vox_decoder_mswav.h"

namespace vox {

static const int kAdaptationTable[16] = { 230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230 };

VoxMSWavSubDecoderMSADPCM::VoxMSWavSubDecoderMSADPCM(StreamCursorInterface *stream, WaveChunk *chunks)
    : VoxMSWavSubDecoderBlock(stream, chunks, chunks->extended.samplesPerBlock) {
    ComputeNumSamples();
}

// the 7 bytes of header per channel hold the two first samples; each byte after is the nibbles of 2 samples
int VoxMSWavSubDecoderMSADPCM::SamplesInBlock(int bytes) {
    const int ch = m_pWaveChunks->channels;
    if (bytes < 7 * ch) return 0;
    const int n = 2 + (bytes - 7 * ch) * 2 / ch;
    return n < m_samplesPerBlock ? n : m_samplesPerBlock;
}

VoxMSWavSubDecoderMSADPCM::~VoxMSWavSubDecoderMSADPCM() {}

short VoxMSWavSubDecoderMSADPCM::DecodeSample(MsAdpcmState *s, unsigned nibble) {
    int predictor = (s->sample1 * s->coef1 + s->sample2 * s->coef2) / 256;
    predictor += ((nibble & 8) ? (int)nibble - 16 : (int)nibble) * s->delta;
    if (predictor > 32767) predictor = 32767;
    else if (predictor < -32768) predictor = -32768;
    s->sample2 = s->sample1;
    s->sample1 = predictor;
    s->delta = (kAdaptationTable[nibble] * s->delta) / 256;
    if (s->delta < 16) s->delta = 16;
    return (short)predictor;
}

int VoxMSWavSubDecoderMSADPCM::DecodeBlock(int blockAlign) {
    const int channels = m_pWaveChunks->channels;
    if (channels < 1 || channels > 2 || blockAlign < 7 * channels || ReadData(m_totalDataBytesRead, m_blockReadBuffer, blockAlign) != blockAlign) return 0;
    const unsigned char *p = m_blockReadBuffer;
    MsAdpcmState st[2];
    int idx[2];
    for (int c = 0; c < channels; c++) {
        idx[c] = *p++;
        if (idx[c] > 6) idx[c] = 6;
    }
    for (int c = 0; c < channels; c++) { st[c].delta = (short)(p[0] | (p[1] << 8)); p += 2; }
    for (int c = 0; c < channels; c++) { st[c].sample1 = (short)(p[0] | (p[1] << 8)); p += 2; }
    for (int c = 0; c < channels; c++) { st[c].sample2 = (short)(p[0] | (p[1] << 8)); p += 2; }
    for (int c = 0; c < channels; c++) {
        st[c].coef1 = m_pWaveChunks->extended.coefficients[idx[c]][0];
        st[c].coef2 = m_pWaveChunks->extended.coefficients[idx[c]][1];
    }
    // the two first samples of a block are the ones of its header, the older one first
    short *out = &m_samples[0];
    for (int c = 0; c < channels; c++) { out[c] = (short)st[c].sample2; out[channels + c] = (short)st[c].sample1; }
    int n = 2;
    const unsigned char *end = m_blockReadBuffer + blockAlign;
    while (p < end && n < m_samplesPerBlock) {
        const unsigned char b = *p++;
        if (channels == 2) {
            out[n * 2] = DecodeSample(&st[0], b >> 4);
            out[n * 2 + 1] = DecodeSample(&st[1], b & 15);
            n++;
        } else {
            out[n++] = DecodeSample(&st[0], b >> 4);
            if (n < m_samplesPerBlock) out[n++] = DecodeSample(&st[0], b & 15);
        }
    }
    return n;
}

}  // namespace vox
