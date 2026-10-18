// core/jet/src/stream/encryptedstream.cpp
#include "stream/encryptedstream.h"
#include <vector>
#include "encryption.h"
#include "stream/streamutil.h"

namespace jet {
namespace stream {

EncryptedStream::EncryptedStream(IStream* encrypted, const String& key)
    : m_encryptedStream(encrypted), m_isDecrypted(false), m_key(key), m_startCount(0), m_isMapped(false)
{
}

EncryptedStream::~EncryptedStream() { delete m_encryptedStream; }

void EncryptedStream::Decrypt() const
{
    m_encryptedStream->SeekBeg(0);
    if (m_isDecrypted || !m_encryptedStream->Size())
        return;
    u16 type = 0;
    *m_encryptedStream >> type;
    unsigned rem = m_encryptedStream->Size() - m_encryptedStream->Tell();
    std::vector<u32> buf((rem + 3) / 4);
    m_encryptedStream->Read(buf.data(), rem);
    unsigned bytes = rem & ~3u;
    if (type == 0) {
        m_decryptedStream.Write(buf.data(), rem);
    } else if (type == 1) {
        encryption::DecipherXTEA(buf.data(), rem / 4, m_key);
        if (bytes < 8 || buf[0] > bytes - 8)
            return;
        const u8* data = reinterpret_cast<const u8*>(buf.data() + 2);
        if (ComputeCRC(data, buf[0]) != buf[1])
            return;
        m_decryptedStream.Write(data, buf[0]);
    }
    m_decryptedStream.SeekBeg(0);
    m_isDecrypted = true;
}

}
}
