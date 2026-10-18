// libs/vox1.1/src/vox_decoder_raw.cpp
#include "vox_decoder_raw.h"

namespace vox {

DecoderInterface *DecoderRawFactory(void *params) { return new DecoderRaw(static_cast<const TrackParams*>(params)); }

DecoderRaw::DecoderRaw(const TrackParams *params) { if (params) m_trackParams = *params; }

DecoderCursorInterface *DecoderRaw::CreateNewCursor(StreamCursorInterface *stream) { return new DecoderRawCursor(this, stream); }

void DecoderRaw::DestroyCursor(DecoderCursorInterface *cursor) { delete cursor; }

int DecoderRaw::GetType() { return kFormatRaw; }

void *DecoderRaw::GetParam() { return &m_trackParams; }

DecoderRawCursor::DecoderRawCursor(DecoderInterface *decoder, StreamCursorInterface *stream) : DecoderCursorInterface(decoder, stream) {
    m_trackParams = static_cast<DecoderRaw*>(decoder)->GetTrackParams();
}

DecoderRawCursor::~DecoderRawCursor() {}

// the track is read from its start again when it loops
int DecoderRawCursor::Decode(void *dst, int bytes) {
    int total = 0;
    unsigned char *out = static_cast<unsigned char*>(dst);
    while (total < bytes) {
        const int n = m_pStreamCursor->Read(out + total, bytes - total);
        total += n;
        if (total >= bytes) break;
        if (!m_loop || n <= 0 || !m_pStreamCursor->EndOfStream()) break;
        if (m_pStreamCursor->Seek(0, kSeekSet) != 0) break;
    }
    return total;
}

int DecoderRawCursor::DecodeRef(void *&ref, int bytes) {
    unsigned char *p = 0;
    const int n = m_pStreamCursor->ReadRef(p, bytes);
    ref = p;
    return n;
}

int DecoderRawCursor::Seek(unsigned sample) { return m_pStreamCursor->Seek((int)(sample * m_trackParams.numChannels * (m_trackParams.bitsPerSample / 8)), kSeekSet); }

bool DecoderRawCursor::HasData() { return m_loop || !m_pStreamCursor->EndOfStream(); }

bool DecoderRawCursor::AllowBufferReference() { return m_pStreamCursor->AllowBufferReference(); }

}  // namespace vox
