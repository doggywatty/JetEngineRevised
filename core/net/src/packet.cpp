// core/net/src/packet.cpp
#include "packet.h"
#include <cstring>
#include "netinterface.h"

namespace net {

PacketWriter::PacketWriter(unsigned type)
{
    m_data.reserve(0x1000);
    u8 h[8] = {0, 0, 0, 0, u8(type), u8(type >> 8), u8(type >> 16), u8(type >> 24)};
    Write(h, 8);
}

void PacketWriter::Write(const void* data, unsigned size)
{
    const u8* p = static_cast<const u8*>(data);
    m_data.insert(m_data.end(), p, p + size);
}

void PacketWriter::Write(unsigned v)
{
    Write(u8(v));
    Write(u8(v >> 8));
    Write(u8(v >> 16));
    Write(u8(v >> 24));
}

void PacketWriter::Write(const char* s)
{
    unsigned n = unsigned(strlen(s));
    Write(n + 1);
    Write(s, n);
    Write(u8(0));
}

void PacketWriter::Send(CNetSocket* s, float timeout)
{
    unsigned n = unsigned(m_data.size()) - 8;
    m_data[0] = u8(n);
    m_data[1] = u8(n >> 8);
    m_data[2] = u8(n >> 16);
    m_data[3] = u8(n >> 24);
    s->write(m_data.data(), int(m_data.size()), timeout);
}

int PacketReader::Receive(CNetSocket* s, float timeout)
{
    u8 h[4];
    if (s->read(h, 4, timeout) != 4 || !s->isOpen())
        return -1;
    u32 size = u32(h[0]) | u32(h[1]) << 8 | u32(h[2]) << 16 | u32(h[3]) << 24;
    if (s->read(h, 4, timeout) != 4 || !s->isOpen())
        return -2;
    m_data.resize(size);
    m_pos = 0;
    m_size = size;
    m_type = u16(h[0] | h[1] << 8);
    if (!size)
        return 4;
    return s->read(m_data.data(), int(size), timeout);
}

unsigned PacketReader::Read(u8* dst, unsigned size) const
{
    memcpy(dst, m_data.data() + m_pos, size);
    m_pos += size;
    return size;
}

s16 PacketReader::ReadShortLE() const
{
    u16 v = u16(m_data[m_pos] | m_data[m_pos + 1] << 8);
    m_pos += 2;
    return s16(v);
}

u32 PacketReader::ReadIntLE() const
{
    const u8* p = m_data.data() + m_pos;
    m_pos += 4;
    return u32(p[0]) | u32(p[1]) << 8 | u32(p[2]) << 16 | u32(p[3]) << 24;
}

s64 PacketReader::ReadLongLE() const
{
    u64 v = 0;
    for (int i = 7; i >= 0; --i)
        v = v << 8 | m_data[m_pos + i];
    m_pos += 8;
    return s64(v);
}

}
