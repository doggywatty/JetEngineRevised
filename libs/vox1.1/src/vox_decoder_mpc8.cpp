// libs/vox1.1/src/vox_decoder_mpc8.cpp
#include <cstdlib>
#include "vox_decoder_mpc8.h"

namespace vox {

// @0x5e2418
DecoderInterface *DecoderMPC8Factory(void *params) { return new DecoderMPC8(static_cast<DecoderMPC8Params*>(params)); }

// @0x5e2460
DecoderMPC8::DecoderMPC8(DecoderMPC8Params *params) : m_forceSampleRate(params ? params->forceSampleRate : -1) {}

// @0x5e2490
DecoderCursorInterface *DecoderMPC8::CreateNewCursor(StreamCursorInterface *stream) { return new DecoderMPC8Cursor(this, stream); }

// @0x5e24d4
void DecoderMPC8::DestroyCursor(DecoderCursorInterface *cursor) { delete cursor; }

int DecoderMPC8::GetType() { return kFormatMpc; }

void *DecoderMPC8::GetParam() { return 0; }

// the reader the demuxer pulls the file through: the callbacks @0x5e2310 (canseek), @0x5e2384 (read), @0x5e23a4 (seek),
// @0x5e23d0 (tell), @0x5e23f0 (get_size) go to the stream cursor
static mpc_int32_t ReaderRead(mpc_reader *r, void *ptr, mpc_int32_t size) {
    StreamCursorInterface *c = static_cast<StreamCursorInterface*>(r->data);
    return c ? c->Read(static_cast<unsigned char*>(ptr), size) : 0;
}
static mpc_bool_t ReaderSeek(mpc_reader *r, mpc_int32_t offset) {
    StreamCursorInterface *c = static_cast<StreamCursorInterface*>(r->data);
    return c ? c->Seek(offset, kSeekSet) == 0 : 0;
}
static mpc_int32_t ReaderTell(mpc_reader *r) {
    StreamCursorInterface *c = static_cast<StreamCursorInterface*>(r->data);
    return c ? c->Tell() : 0;
}
static mpc_int32_t ReaderGetSize(mpc_reader *r) {
    StreamCursorInterface *c = static_cast<StreamCursorInterface*>(r->data);
    return c ? c->GetStream()->GetSize() : 0;
}
static mpc_bool_t ReaderCanSeek(mpc_reader *r) {
    StreamCursorInterface *c = static_cast<StreamCursorInterface*>(r->data);
    return c ? c->IsSeekable() : 0;
}

// @0x5e24fc
DecoderMPC8Cursor::DecoderMPC8Cursor(DecoderInterface *decoder, StreamCursorInterface *stream)
    : DecoderCursorInterface(decoder, stream), m_totalSampleDecoded(0), m_samplesInBuffer(0), m_samplesInBufferConsumed(0),
      m_pDemux(0), m_sampleBuffer(0), m_error(MPC_STATUS_OK) {
    m_sampleBuffer = (float*)VoxAlloc(MPC_DECODER_BUFFER_LENGTH * sizeof(float));
    if (m_sampleBuffer) {
        m_reader.read = ReaderRead;
        m_reader.seek = ReaderSeek;
        m_reader.tell = ReaderTell;
        m_reader.get_size = ReaderGetSize;
        m_reader.canseek = ReaderCanSeek;
        m_reader.data = stream;
        m_pDemux = mpc_demux_init(&m_reader);
    }
    if (m_pDemux && m_reader.read && m_reader.data) {
        mpc_streaminfo info;
        mpc_demux_get_info(m_pDemux, &info);
        m_trackParams.bitsPerSample = 16;
        m_trackParams.numChannels = (int)info.channels;
        const int force = static_cast<DecoderMPC8*>(decoder)->GetForceSampleRate();
        m_trackParams.sampleRate = force > 0 ? force : (int)info.sample_freq;
        m_trackParams.numSamples = (int)info.samples;
    }
}

// @0x5e26d0
DecoderMPC8Cursor::~DecoderMPC8Cursor() {
    if (m_pDemux) mpc_demux_exit(m_pDemux);
    VoxFree(m_sampleBuffer);
}

// @0x5e2b2c: the float samples are scaled to 16 bits and clipped
void DecoderMPC8Cursor::ConvertFloatToShort(short *dst, const float *src, int count) {
    for (int i = 0; i < count; i++) {
        const int v = (int)(src[i] * 32768.0f);
        dst[i] = v > 32767 ? 32767 : v < -32768 ? -32768 : (short)v;
    }
}

// @0x5e2764: at the end of the track a looping one starts again
bool DecoderMPC8Cursor::CheckForEndOfSample() {
    if (m_totalSampleDecoded != (unsigned)m_trackParams.numSamples) return false;
    if (!m_loop || !m_pDemux) return true;
    m_error = mpc_demux_seek_sample(m_pDemux, 0);
    if (m_error != MPC_STATUS_OK) return true;
    m_totalSampleDecoded = 0;
    m_samplesInBuffer = 0;
    m_samplesInBufferConsumed = 0;
    return false;
}

// @0x5e27c4
int DecoderMPC8Cursor::Decode(void *dst, int bytes) {
    if (!m_pDemux) return -1;
    const int channels = m_trackParams.numChannels;
    const int frameBytes = (m_trackParams.bitsPerSample / 8) * channels;
    const int want = bytes / frameBytes;
    short *out = static_cast<short*>(dst);
    int done = 0;
    while (done < want) {
        if (m_samplesInBufferConsumed >= m_samplesInBuffer) {
            if (CheckForEndOfSample() && !m_loop) break;
            if (m_error != MPC_STATUS_OK) break;
            mpc_frame_info frame;
            frame.buffer = m_sampleBuffer;
            m_error = mpc_demux_decode(m_pDemux, &frame);
            if (m_error != MPC_STATUS_OK || frame.bits == -1) break;
            m_samplesInBuffer = (int)frame.samples;
            m_samplesInBufferConsumed = 0;
            if (m_samplesInBuffer <= 0) continue;
        }
        int n = m_samplesInBuffer - m_samplesInBufferConsumed;
        if (n > want - done) n = want - done;
        // the last frame is padded past the end of the track
        if (m_trackParams.numSamples > 0 && (int)m_totalSampleDecoded + n > m_trackParams.numSamples) n = m_trackParams.numSamples - (int)m_totalSampleDecoded;
        if (n <= 0) { m_samplesInBufferConsumed = m_samplesInBuffer; continue; }
        ConvertFloatToShort(out + done * channels, m_sampleBuffer + m_samplesInBufferConsumed * channels, n * channels);
        m_samplesInBufferConsumed += n;
        m_totalSampleDecoded += n;
        done += n;
    }
    return done * frameBytes;
}

// @0x5e2a44
int DecoderMPC8Cursor::Seek(unsigned sample) {
    if (!m_pDemux) return -1;
    const unsigned total = (unsigned)m_trackParams.numSamples;
    if (sample > total) sample = (m_loop && total) ? sample % total : total;
    m_error = mpc_demux_seek_sample(m_pDemux, sample);
    if (m_error != MPC_STATUS_OK) return -1;
    m_totalSampleDecoded = sample;
    m_samplesInBuffer = 0;
    m_samplesInBufferConsumed = 0;
    return 0;
}

// @0x5e2abc
bool DecoderMPC8Cursor::HasData() {
    if (!m_pStreamCursor) return false;
    if (m_error != MPC_STATUS_OK) return false;
    if (m_totalSampleDecoded >= (unsigned)m_trackParams.numSamples && m_loop) CheckForEndOfSample();
    return m_totalSampleDecoded < (unsigned)m_trackParams.numSamples;
}

}  // namespace vox
