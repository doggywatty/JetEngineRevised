// core/jet/src/stream/redundantstream.cpp
#include "stream/redundantstream.h"
#include <cstdio>
#include "stream/streamutil.h"

namespace jet {
namespace stream {

// Layout: 2 x header; (copies - 1) x (data, 2 x header); 5 x header; data.
// header = kHeaderSig (176 bytes) + u32 crc + u32 size; data = kDataSig (144 bytes) + payload.
static const u32 kHeaderSig[44] = {0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,0xedb88320,0x9abfb3b6,0x3b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x4db2615,0x73dc1683,0xe3630b12,0x94643b84,0xd6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0xa00ae27,0x7d079eb1,0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc};
static const u32 kDataSig[36] = {0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf};

static bool MatchSignature(IStream& s, const u32* sig, unsigned n)
{
    unsigned pos = s.Tell();
    if (pos + n * 4 > s.Size())
        return false;
    for (unsigned i = 0; i < n; ++i) {
        u32 v = 0;
        s >> v;
        if (v != sig[i]) {
            s.SeekBeg(pos);
            return false;
        }
    }
    return true;
}

// Next header whose size fits in the rest of the stream; returns its position or -1.
static int FindNextHeader(IStream& s)
{
    unsigned total = s.Size();
    for (;;) {
        while (!MatchSignature(s, kHeaderSig, 44)) {
            if (s.Tell() + 176 > total)
                return -1;
            s.SeekCur(1);
        }
        unsigned pos = s.Tell() - 176;
        u32 crc = 0, size = 0;
        s >> crc >> size;
        if (size <= total - s.Tell())
            return int(pos);
    }
}

// First data block whose payload has the given CRC; returns the payload position or -1.
static int FindData(IStream& s, u32 crc, unsigned size)
{
    unsigned total = s.Size();
    if (size + 144 > total)
        return -1;
    s.SeekBeg(0);
    while (s.Tell() + size + 144 <= total) {
        if (!MatchSignature(s, kDataSig, 36)) {
            s.SeekCur(1);
            continue;
        }
        unsigned pos = s.Tell();
        if (ComputeCRC(s, size) == crc)
            return int(pos);
        s.SeekBeg(pos);
    }
    return -1;
}

void RedundantStream::SetSource(IStream& src, bool strict)
{
    m_source = &src;
    m_destination = nullptr;
    m_strictDecoding = strict;
}

void RedundantStream::SetDestination(IStreamW& dst, unsigned copies)
{
    m_source = nullptr;
    m_destination = &dst;
    m_redundantCopyCount = copies < 2 ? 2 : copies > 100 ? 100 : copies;
    if (copies != m_redundantCopyCount)
        printf("RedundantStream: copy count must be in [2, 100], got %u, using %u\n", copies, m_redundantCopyCount);
}

bool RedundantStream::IsStarted() const
{
    if (IsForReading())
        return m_subStream && m_subStream->IsStarted();
    return m_bufferStream && m_bufferStream->IsStarted();
}

bool RedundantStream::IsMapped() const
{
    if (IsForReading())
        return m_subStream && m_subStream->IsMapped();
    return m_bufferStream && m_bufferStream->IsMapped();
}

void RedundantStream::Start()
{
    if (IsForReading()) {
        if (!m_subStream && !OpenForReading())
            return;
        m_subStream->Start();
    } else {
        if (!m_bufferStream && !OpenForWriting())
            return;
        m_bufferStream->Start();
    }
}

void RedundantStream::Finish()
{
    if (!IsStarted())
        return;
    if (IsForReading()) {
        m_subStream->Finish();
        if (!m_subStream->IsStarted())
            CloseForReading();
    } else {
        m_bufferStream->Finish();
        if (!m_bufferStream->IsStarted())
            CloseForWriting();
    }
}

// Without a matching copy the whole source is used as-is, unless decoding is strict.
bool RedundantStream::OpenForReading()
{
    m_subStream.reset();
    if (!m_source)
        return false;
    m_source->Start();
    if (!m_source->IsStarted())
        return false;
    int data = -1;
    u32 crc = 0, size = 0;
    for (int h; (h = FindNextHeader(*m_source)) >= 0;) {
        m_source->SeekBeg(unsigned(h) + 176);
        *m_source >> crc >> size;
        data = FindData(*m_source, crc, size);
        if (data >= 0)
            break;
        m_source->SeekBeg(unsigned(h) + 1);
    }
    if (data < 0) {
        if (m_strictDecoding) {
            m_source->Finish();
            return false;
        }
        size = m_source->Size();
        data = 0;
    }
    m_subStream.reset(new SubStream);
    m_subStream->SetSource(*m_source, unsigned(data), size);
    m_source->Finish();
    return true;
}

bool RedundantStream::OpenForWriting()
{
    m_bufferStream.reset(new MemoryStream);
    return true;
}

void RedundantStream::CloseForWriting()
{
    MemoryStream& b = *m_bufferStream;
    if (!m_destination) {
        printf("RedundantStream: no destination for %u bytes\n", b.Size());
    } else {
        m_destination->Start();
        if (!m_destination->IsStarted()) {
            printf("RedundantStream: cannot open destination for %u bytes\n", b.Size());
        } else if (b.Size()) {
            IStreamW& d = *m_destination;
            b.SeekBeg(0);
            u32 crc = ComputeCRC(b, b.Size()), size = b.Size();
            std::vector<u8>& data = b.GetInternalArray();
            auto header = [&](int n) {
                for (int i = 0; i < n; ++i) {
                    d.Write(kHeaderSig, sizeof(kHeaderSig));
                    d << crc << size;
                }
            };
            header(2);
            for (unsigned i = 0; i + 1 < m_redundantCopyCount; ++i) {
                d.Write(kDataSig, sizeof(kDataSig));
                d.Write(data.data(), size);
                header(2);
            }
            header(5);
            d.Write(kDataSig, sizeof(kDataSig));
            d.Write(data.data(), size);
            d.Finish();
        }
    }
    m_bufferStream.reset();
}

}
}
