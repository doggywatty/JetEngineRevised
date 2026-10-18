// core/jet/include/stream/istream.h
#ifndef CORE_JET_INCLUDE_STREAM_ISTREAM_H
#define CORE_JET_INCLUDE_STREAM_ISTREAM_H

#include <string>
#include "jrtti.h"
#include "jtypes.h"

namespace jet {
class String;

namespace stream {

class IStream {
    JET_RTTI_ROOT(IStream)
public:
    IStream(const IStream&) = delete;
    IStream& operator=(const IStream&) = delete;
    virtual ~IStream() {}

    virtual void Start() = 0;
    virtual void Finish() = 0;
    virtual bool IsStarted() const = 0;
    virtual unsigned Read(void* dst, unsigned size) const = 0;
    virtual unsigned Read1(void* dst) const = 0;
    virtual unsigned Read2(void* dst) const = 0;
    virtual unsigned Read4(void* dst) const = 0;
    virtual void SeekBeg(unsigned pos) = 0;
    virtual void SeekCur(int delta) = 0;
    virtual unsigned Tell() const = 0;
    virtual void* Map(unsigned size) = 0;
    virtual void Unmap() = 0;
    virtual bool IsMapped() const = 0;
    virtual unsigned Size() const = 0;
    virtual bool IsChanged() const = 0;
    virtual bool Touch() = 0;

    s64 GetTimestamp() const { return m_timestamp; }

protected:
    IStream() : m_timestamp(0) {}
    void SetTimestamp(s64 t) { m_timestamp = t; }

private:
    s64 m_timestamp;
};

class IStreamW : public IStream {
    JET_RTTI(IStreamW, IStream)
public:
    virtual unsigned Write(const void* src, unsigned size) = 0;
};

// Raw (host byte order) serialization, as in the original templates.
template <class T>
const IStream& operator>>(const IStream& s, T& v)
{
    s.Read(&v, sizeof(T));
    return s;
}

template <class T>
IStreamW& operator<<(IStreamW& s, const T& v)
{
    s.Write(&v, sizeof(T));
    return s;
}

inline const IStream& operator>>(const IStream& s, u8& v) { s.Read1(&v); return s; }
inline const IStream& operator>>(const IStream& s, u16& v) { s.Read2(&v); return s; }
inline const IStream& operator>>(const IStream& s, s16& v) { s.Read2(&v); return s; }
inline const IStream& operator>>(const IStream& s, u32& v) { s.Read4(&v); return s; }
inline const IStream& operator>>(const IStream& s, s32& v) { s.Read4(&v); return s; }
inline const IStream& operator>>(const IStream& s, float& v) { s.Read4(&v); return s; }

const IStream& operator>>(const IStream& s, String& v);
const IStream& operator>>(const IStream& s, std::string& v);
IStreamW& operator<<(IStreamW& s, const String& v);
IStreamW& operator<<(IStreamW& s, const std::string& v);

}
}

#endif
