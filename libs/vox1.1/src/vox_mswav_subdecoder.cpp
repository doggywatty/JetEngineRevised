// libs/vox1.1/src/vox_mswav_subdecoder.cpp
#include "vox_decoder_mswav.h"

namespace vox {

VoxMSWavSubDecoder::VoxMSWavSubDecoder(StreamCursorInterface *stream, WaveChunk *chunks)
    : m_pStreamCursor(stream), m_pWaveChunks(chunks), m_decodedSamples(0), m_loop(false), m_isDecoderInError(false) {
    m_trackParams.numChannels = chunks->channels;
    m_trackParams.sampleRate = (int)chunks->sampleRate;
    m_trackParams.bitsPerSample = 16;
}

VoxMSWavSubDecoder::~VoxMSWavSubDecoder() {}

// the "data" chunks are one run of bytes for the sub decoders
int VoxMSWavSubDecoder::ReadData(unsigned pos, unsigned char *dst, int size) {
    int total = 0;
    unsigned base = 0;
    for (size_t i = 0; i < m_pWaveChunks->dataNodes.size() && size > 0; i++) {
        const DataNode &n = m_pWaveChunks->dataNodes[i];
        if (pos < base + n.size) {
            const unsigned local = pos - base;
            int chunk = (int)(n.size - local);
            if (chunk > size) chunk = size;
            if (m_pStreamCursor->Seek((int)(n.offset + local), kSeekSet) < 0) break;
            const int got = m_pStreamCursor->Read(dst + total, chunk);
            total += got;
            size -= got;
            pos += got;
            if (got < chunk) break;
        }
        base += n.size;
    }
    return total;
}

VoxMSWavSubDecoderPCM::VoxMSWavSubDecoderPCM(StreamCursorInterface *stream, WaveChunk *chunks)
    : VoxMSWavSubDecoder(stream, chunks), m_position(0) {
    m_trackParams.numSamples = GetNbSamples();
}

VoxMSWavSubDecoderPCM::~VoxMSWavSubDecoderPCM() {}

int VoxMSWavSubDecoderPCM::GetNbSamples() const {
    const unsigned frame = m_pWaveChunks->channels * (m_pWaveChunks->bitsPerSample / 8);
    return frame ? (int)(m_pWaveChunks->totalDataSize / frame) : 0;
}

int VoxMSWavSubDecoderPCM::Decode(void *dst, int bytes) {
    const int channels = m_pWaveChunks->channels;
    const int srcBytes = m_pWaveChunks->bitsPerSample / 8;
    const int frameOut = channels * 2;
    int frames = bytes / frameOut, done = 0;
    short *out = static_cast<short*>(dst);
    while (done < frames) {
        const unsigned left = m_pWaveChunks->totalDataSize - m_position;
        if (left < (unsigned)(channels * srcBytes)) {
            if (!m_loop || !m_pWaveChunks->totalDataSize) break;
            m_position = 0;
            m_decodedSamples = 0;
            continue;
        }
        int n = frames - done;
        if ((unsigned)n * channels * srcBytes > left) n = (int)(left / (channels * srcBytes));
        if (srcBytes == 2) {
            const int got = ReadData(m_position, reinterpret_cast<unsigned char*>(out + done * channels), n * channels * 2);
            n = got / (channels * 2);
        } else {
            std::vector<unsigned char> tmp(n * channels);
            const int got = ReadData(m_position, tmp.data(), n * channels);
            n = got / channels;
            for (int i = 0; i < n * channels; i++) out[done * channels + i] = (short)((tmp[i] - 128) << 8);
        }
        if (n <= 0) break;
        m_position += n * channels * srcBytes;
        m_decodedSamples += n;
        done += n;
    }
    return done * frameOut;
}

int VoxMSWavSubDecoderPCM::Seek(unsigned sample) {
    const unsigned frame = m_pWaveChunks->channels * (m_pWaveChunks->bitsPerSample / 8);
    if (sample > (unsigned)m_trackParams.numSamples) return -1;
    m_position = sample * frame;
    m_decodedSamples = sample;
    return 0;
}

bool VoxMSWavSubDecoderPCM::HasData() { return m_position < m_pWaveChunks->totalDataSize || (m_loop && m_pWaveChunks->totalDataSize); }

VoxMSWavSubDecoderBlock::VoxMSWavSubDecoderBlock(StreamCursorInterface *stream, WaveChunk *chunks, int samplesPerBlock)
    : VoxMSWavSubDecoder(stream, chunks), m_blockReadBuffer(0), m_samplesPerBlock(samplesPerBlock), m_totalDataBytesRead(0),
      m_samplesInBuffer(0), m_samplesInBufferConsumed(0), m_totalSampleDecoded(0) {
    m_blockReadBuffer = (unsigned char*)VoxAlloc(chunks->blockAlign);
    m_samples.resize((size_t)samplesPerBlock * chunks->channels);
}

// the samples of the whole data: the full blocks and the last one if it is shorter, or what the "fact" chunk says
void VoxMSWavSubDecoderBlock::ComputeNumSamples() {
    const unsigned align = m_pWaveChunks->blockAlign;
    const unsigned full = align ? m_pWaveChunks->totalDataSize / align : 0;
    unsigned total = full * m_samplesPerBlock;
    const unsigned rest = align ? m_pWaveChunks->totalDataSize % align : 0;
    if (rest) total += SamplesInBlock((int)rest);
    if (m_pWaveChunks->factSamples && m_pWaveChunks->factSamples <= total) total = m_pWaveChunks->factSamples;
    m_trackParams.numSamples = (int)total;
}

VoxMSWavSubDecoderBlock::~VoxMSWavSubDecoderBlock() { VoxFree(m_blockReadBuffer); }

int VoxMSWavSubDecoderBlock::Decode(void *dst, int bytes) {
    const int channels = m_pWaveChunks->channels;
    const int frames = bytes / (channels * 2);
    short *out = static_cast<short*>(dst);
    int done = 0;
    while (done < frames) {
        if (m_samplesInBufferConsumed >= m_samplesInBuffer) {
            if (m_totalSampleDecoded >= (unsigned)m_trackParams.numSamples) {
                if (!m_loop || m_trackParams.numSamples <= 0) break;
                Seek(0);
            }
            if (m_totalDataBytesRead >= m_pWaveChunks->totalDataSize) break;
            int bytes = (int)(m_pWaveChunks->totalDataSize - m_totalDataBytesRead);
            if (bytes > m_pWaveChunks->blockAlign) bytes = m_pWaveChunks->blockAlign;
            m_samplesInBuffer = DecodeBlock(bytes);
            m_samplesInBufferConsumed = 0;
            m_totalDataBytesRead += bytes;
            if (m_samplesInBuffer <= 0) { m_isDecoderInError = true; break; }
        }
        int n = m_samplesInBuffer - m_samplesInBufferConsumed;
        if (n > frames - done) n = frames - done;
        if (m_totalSampleDecoded + n > (unsigned)m_trackParams.numSamples) n = (int)((unsigned)m_trackParams.numSamples - m_totalSampleDecoded);
        if (n <= 0) { m_samplesInBufferConsumed = m_samplesInBuffer; continue; }
        memcpy(out + done * channels, &m_samples[m_samplesInBufferConsumed * channels], n * channels * 2);
        m_samplesInBufferConsumed += n;
        m_totalSampleDecoded += n;
        m_decodedSamples += n;
        done += n;
    }
    return done * channels * 2;
}

// to a sample: the block that has it is decoded and read from its place
int VoxMSWavSubDecoderBlock::Seek(unsigned sample) {
    if (sample > (unsigned)m_trackParams.numSamples) return -1;
    const unsigned block = sample / m_samplesPerBlock;
    m_totalDataBytesRead = block * m_pWaveChunks->blockAlign;
    m_samplesInBuffer = 0;
    m_samplesInBufferConsumed = 0;
    m_totalSampleDecoded = block * m_samplesPerBlock;
    m_decodedSamples = m_totalSampleDecoded;
    unsigned skip = sample - m_totalSampleDecoded;
    if (skip && m_totalDataBytesRead < m_pWaveChunks->totalDataSize) {
        int bytes = (int)(m_pWaveChunks->totalDataSize - m_totalDataBytesRead);
        if (bytes > m_pWaveChunks->blockAlign) bytes = m_pWaveChunks->blockAlign;
        m_samplesInBuffer = DecodeBlock(bytes);
        m_totalDataBytesRead += bytes;
        if (m_samplesInBuffer < (int)skip) skip = (unsigned)m_samplesInBuffer;
        m_samplesInBufferConsumed = (int)skip;
        m_totalSampleDecoded += skip;
    }
    return 0;
}

bool VoxMSWavSubDecoderBlock::HasData() {
    return !m_isDecoderInError && (m_totalSampleDecoded < (unsigned)m_trackParams.numSamples || (m_loop && m_trackParams.numSamples > 0));
}

}  // namespace vox
