// core/jet/include/stream/encryptedstream.h
#ifndef CORE_JET_INCLUDE_STREAM_ENCRYPTEDSTREAM_H
#define CORE_JET_INCLUDE_STREAM_ENCRYPTEDSTREAM_H

#include "core/pstring.h"
#include "stream/memorystream.h"

namespace jet {
namespace stream {

// Decrypts the wrapped stream into memory on first access.
// Format: u16 type; 0 = plain, 1 = XTEA over the rest, which is then [u32 size][u32 crc32][data].
class EncryptedStream : public IStream {
    JET_RTTI(EncryptedStream, IStream)
public:
    EncryptedStream(IStream* encrypted, const String& key);  // takes ownership
    virtual ~EncryptedStream();

    virtual void Start() { m_encryptedStream->Start(); ++m_startCount; }
    virtual void Finish() { m_encryptedStream->Finish(); --m_startCount; }
    virtual bool IsStarted() const { return m_startCount != 0; }
    virtual unsigned Read(void* dst, unsigned size) const { Decrypt(); return m_decryptedStream.Read(dst, size); }
    virtual unsigned Read1(void* dst) const { Decrypt(); return m_decryptedStream.Read1(dst); }
    virtual unsigned Read2(void* dst) const { Decrypt(); return m_decryptedStream.Read2(dst); }
    virtual unsigned Read4(void* dst) const { Decrypt(); return m_decryptedStream.Read4(dst); }
    virtual void SeekBeg(unsigned pos) { Decrypt(); m_decryptedStream.SeekBeg(pos); }
    virtual void SeekCur(int delta) { Decrypt(); m_decryptedStream.SeekCur(delta); }
    virtual unsigned Tell() const { Decrypt(); return m_decryptedStream.Tell(); }
    virtual void* Map(unsigned size) { Decrypt(); m_isMapped = true; return m_decryptedStream.Map(size); }
    virtual void Unmap() { m_isMapped = false; }
    virtual bool IsMapped() const { return m_isMapped; }
    virtual unsigned Size() const { Decrypt(); return m_decryptedStream.Size(); }
    virtual bool IsChanged() const { return m_encryptedStream->IsChanged(); }
    virtual bool Touch() { return m_encryptedStream->Touch(); }

protected:
    void Decrypt() const;

    IStream* m_encryptedStream;
    mutable MemoryStream m_decryptedStream;
    mutable bool m_isDecrypted;
    String m_key;
    unsigned m_startCount;
    bool m_isMapped;
};

}
}

#endif
