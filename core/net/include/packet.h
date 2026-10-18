// core/net/include/packet.h
#ifndef CORE_NET_INCLUDE_PACKET_H
#define CORE_NET_INCLUDE_PACKET_H

#include <vector>
#include "jtypes.h"

namespace net {

class CNetSocket;

// Wire format: u32 payload size, u32 type (only the low 16 bits are kept on receive), payload. Little endian.
class PacketWriter {
public:
    explicit PacketWriter(unsigned type);
    void Write(const void* data, unsigned size);
    void Write(u8 v) { Write(&v, 1); }
    void Write(unsigned v);  // LE
    void Write(const char* s);  // u32 length including the NUL, then the bytes and NUL
    void Send(CNetSocket* s, float timeout = 1.0f);
    const std::vector<u8>& GetData() const { return m_data; }

private:
    std::vector<u8> m_data;
};

class PacketReader {
public:
    PacketReader() : m_size(0), m_type(0), m_pos(0) {}
    // 4 on an empty packet, -1/-2 when the size/type could not be read, else the payload bytes read.
    int Receive(CNetSocket* s, float timeout);
    unsigned GetSize() const { return m_size; }
    unsigned GetType() const { return m_type; }
    unsigned Read(u8* dst, unsigned size) const;
    u8 ReadByte() const { return m_data[m_pos++]; }
    s16 ReadShortLE() const;
    u32 ReadIntLE() const;
    s64 ReadLongLE() const;
    const u8* GetData() const { return m_data.data(); }
    unsigned GetRemaining() const { return unsigned(m_data.size()) - m_pos; }

private:
    unsigned m_size;
    u16 m_type;
    std::vector<u8> m_data;
    mutable unsigned m_pos;
};

}

#endif
