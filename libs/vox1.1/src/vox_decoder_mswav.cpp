// libs/vox1.1/src/vox_decoder_mswav.cpp
#include "vox_decoder_mswav.h"

namespace vox {

// @0x5e2be4
DecoderInterface *DecoderMSWavFactory(void *) { return new DecoderMSWav(); }

// @0x5e2c3c
DecoderMSWav::DecoderMSWav() : m_needParsing(true) {}

// @0x5e2c64
DecoderMSWav::~DecoderMSWav() {}

DecoderCursorInterface *DecoderMSWav::CreateNewCursor(StreamCursorInterface *stream) { return new DecoderMSWavCursor(this, stream); }

void DecoderMSWav::DestroyCursor(DecoderCursorInterface *cursor) { delete cursor; }

int DecoderMSWav::GetType() { return kFormatWav; }

void *DecoderMSWav::GetParam() { return 0; }

static unsigned ReadLE32(const unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24); }
static unsigned short ReadLE16(const unsigned char *p) { return (unsigned short)(p[0] | (p[1] << 8)); }

// the chunks of the file are read once by the first cursor of the decoder, and the sub decoder of the format is made
DecoderMSWavCursor::DecoderMSWavCursor(DecoderInterface *decoder, StreamCursorInterface *stream)
    : DecoderCursorInterface(decoder, stream), m_pWaveChunks(0), m_pSubDecoder(0) {
    DecoderMSWav *d = static_cast<DecoderMSWav*>(decoder);
    m_pWaveChunks = d->GetWaveChunks();
    if (d->NeedParsing()) {
        if (ParseFile()) d->SetParsed();
        else { m_isDecoderInError = true; return; }
    }
    switch (m_pWaveChunks->formatTag) {
        case 1: m_pSubDecoder = new VoxMSWavSubDecoderPCM(stream, m_pWaveChunks); break;
        case 2: m_pSubDecoder = new VoxMSWavSubDecoderMSADPCM(stream, m_pWaveChunks); break;
        case 0x11: m_pSubDecoder = new VoxMSWavSubDecoderIMAADPCM(stream, m_pWaveChunks); break;
        default: m_isDecoderInError = true; return;
    }
    m_trackParams = m_pSubDecoder->GetTrackParams();
}

DecoderMSWavCursor::~DecoderMSWavCursor() { delete m_pSubDecoder; }

bool DecoderMSWavCursor::ParseFile() {
    if (m_pStreamCursor->IsSeekable()) m_pStreamCursor->Seek(0, kSeekSet);
    unsigned char head[12];
    if (m_pStreamCursor->Read(head, 12) != 12 || memcmp(head, "RIFF", 4) || memcmp(head + 8, "WAVE", 4)) return false;
    const int size = m_pStreamCursor->GetStream()->GetSize();
    WaveChunk &w = *m_pWaveChunks;
    bool haveFmt = false;
    unsigned pos = 12;
    unsigned char ch[8];
    while (m_pStreamCursor->Read(ch, 8) == 8) {
        pos += 8;
        unsigned csize = ReadLE32(ch + 4);
        if (!memcmp(ch, "fmt ", 4)) {
            unsigned char fmt[64];
            const unsigned n = csize < sizeof(fmt) ? csize : (unsigned)sizeof(fmt);
            if (n < 16 || m_pStreamCursor->Read(fmt, n) != (int)n) return false;
            w.formatTag = ReadLE16(fmt);
            w.channels = ReadLE16(fmt + 2);
            w.sampleRate = ReadLE32(fmt + 4);
            w.avgBytesPerSec = ReadLE32(fmt + 8);
            w.blockAlign = ReadLE16(fmt + 12);
            w.bitsPerSample = ReadLE16(fmt + 14);
            if (n >= 22) {
                w.extended.samplesPerBlock = ReadLE16(fmt + 18);
                if (w.formatTag == 2 && n >= 24) {
                    w.extended.numCoefficients = ReadLE16(fmt + 20);
                    for (int i = 0; i < w.extended.numCoefficients && i < 7 && 22 + i * 4 + 4 <= (int)n; i++) {
                        w.extended.coefficients[i][0] = (short)ReadLE16(fmt + 22 + i * 4);
                        w.extended.coefficients[i][1] = (short)ReadLE16(fmt + 24 + i * 4);
                    }
                }
            }
            if (csize > n) m_pStreamCursor->Seek((int)(csize - n), kSeekCur);
            haveFmt = true;
        } else if (!memcmp(ch, "fact", 4)) {
            unsigned char f[4];
            if (csize >= 4 && m_pStreamCursor->Read(f, 4) == 4) w.factSamples = ReadLE32(f);
            if (csize > 4) m_pStreamCursor->Seek((int)(csize - 4), kSeekCur);
        } else if (!memcmp(ch, "data", 4)) {
            if (size > 0 && pos + csize > (unsigned)size) csize = (unsigned)size - pos;
            DataNode node = { pos, csize };
            w.dataNodes.push_back(node);
            w.totalDataSize += csize;
            m_pStreamCursor->Seek((int)csize, kSeekCur);
        } else {
            m_pStreamCursor->Seek((int)csize, kSeekCur);
        }
        pos += csize;
        if (csize & 1) { m_pStreamCursor->Seek(1, kSeekCur); pos++; }
    }
    return haveFmt && !w.dataNodes.empty();
}

int DecoderMSWavCursor::Decode(void *dst, int bytes) { return m_pSubDecoder ? m_pSubDecoder->Decode(dst, bytes) : -1; }

int DecoderMSWavCursor::Seek(unsigned sample) { return m_pSubDecoder ? m_pSubDecoder->Seek(sample) : -1; }

bool DecoderMSWavCursor::HasData() { return m_pSubDecoder && m_pSubDecoder->HasData(); }

void DecoderMSWavCursor::SetLoop(bool loop) {
    DecoderCursorInterface::SetLoop(loop);
    if (m_pSubDecoder) m_pSubDecoder->SetLoop(loop);
}

}  // namespace vox
