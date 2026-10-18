// core/jet/src/stream/substream.cpp
#include "stream/substream.h"

namespace jet {
namespace stream {

SubStream::SubStream()
    : m_source(nullptr), m_destination(nullptr), m_streamOffset(0), m_size(0), m_offset(0), m_startCount(0), m_isMapped(false)
{
}

void SubStream::SetSource(IStream& src, unsigned offset, unsigned size)
{
    m_source = &src;
    m_destination = nullptr;
    m_streamOffset = offset;
    m_size = size;
}

void SubStream::SetDestination(IStreamW& dst, unsigned offset, unsigned size)
{
    m_source = nullptr;
    m_destination = &dst;
    m_streamOffset = offset;
    m_size = size;
}

void SubStream::Start()
{
    if (m_startCount)
        return;
    IStream* s = Inner();
    if (!s)
        return;
    s->Start();
    if (s->IsStarted())
        ++m_startCount;
}

void SubStream::Finish()
{
    if (--m_startCount)
        return;
    if (IStream* s = Inner())
        s->Finish();
}

unsigned SubStream::Read(void* dst, unsigned size) const
{
    unsigned n = Size() - m_offset;
    if (size < n)
        n = size;
    IStream* s = Inner();
    if (!n || !s)
        return 0;
    s->SeekBeg(m_offset + m_streamOffset);
    n = s->Read(dst, n);
    m_offset += n;
    return n;
}

void SubStream::SeekBeg(unsigned pos) { m_offset = pos < Size() ? pos : Size(); }

void SubStream::SeekCur(int delta)
{
    s64 p = s64(m_offset) + delta;
    m_offset = unsigned(p < 0 ? 0 : p > Size() ? Size() : p);
}

unsigned SubStream::Write(const void* src, unsigned size)
{
    if (!m_destination)
        return 0;
    unsigned n = Size() - m_offset;
    if (size < n)
        n = size;
    if (!n)
        return 0;
    m_destination->SeekBeg(m_offset + m_streamOffset);
    n = m_destination->Write(src, n);
    m_offset += n;
    return n;
}

bool SubStream::IsChanged() const { return Inner() ? Inner()->IsChanged() : false; }

bool SubStream::Touch() { return Inner() ? Inner()->Touch() : true; }

}
}
