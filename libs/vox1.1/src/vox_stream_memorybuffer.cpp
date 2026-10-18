// libs/vox1.1/src/vox_stream_memorybuffer.cpp
#include <cstdlib>
#include <cstring>
#include "vox_stream.h"

namespace vox {

StreamMemoryBuffer::StreamMemoryBuffer(StreamMemoryBufferParams *params) : m_buffer(params->buffer), m_isBufferOwned(params->ownsBuffer) {
    m_size = params->size;
}

StreamMemoryBuffer::~StreamMemoryBuffer() {
    if (m_isBufferOwned) free(m_buffer);
}

StreamCursorInterface *StreamMemoryBuffer::CreateNewCursor() { return new StreamMemoryBufferCursor(this); }

void StreamMemoryBuffer::DestroyCursor(StreamCursorInterface *cursor) { delete cursor; }

int StreamMemoryBuffer::GetType() { return kStreamMemoryBuffer; }

StreamMemoryBufferCursor::StreamMemoryBufferCursor(StreamInterface *stream) : StreamCursorInterface(stream), m_cursorPosition(0) {}

int StreamMemoryBufferCursor::Seek(int offset, int origin) {
    const int size = m_pStream->GetSize();
    int pos = origin == kSeekSet ? offset : origin == kSeekCur ? m_cursorPosition + offset : size + offset;
    if (pos < 0 || pos > size) return -1;
    m_cursorPosition = pos;
    return 0;
}

bool StreamMemoryBufferCursor::IsSeekable() { return true; }

int StreamMemoryBufferCursor::Tell() { return m_cursorPosition; }

int StreamMemoryBufferCursor::Read(unsigned char *dst, int size) {
    const int room = m_pStream->GetSize() - m_cursorPosition;
    if (size > room) size = room;
    if (size <= 0) return 0;
    memcpy(dst, static_cast<StreamMemoryBuffer*>(m_pStream)->GetBuffer() + m_cursorPosition, size);
    m_cursorPosition += size;
    return size;
}

int StreamMemoryBufferCursor::ReadRef(unsigned char *&ref, int size) {
    const int room = m_pStream->GetSize() - m_cursorPosition;
    if (size > room) size = room;
    if (size <= 0) { ref = 0; return 0; }
    ref = static_cast<StreamMemoryBuffer*>(m_pStream)->GetBuffer() + m_cursorPosition;
    m_cursorPosition += size;
    return size;
}

bool StreamMemoryBufferCursor::EndOfStream() { return m_cursorPosition >= m_pStream->GetSize(); }

bool StreamMemoryBufferCursor::AllowBufferReference() { return true; }

}  // namespace vox
