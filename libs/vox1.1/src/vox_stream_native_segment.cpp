// libs/vox1.1/src/vox_stream_native_segment.cpp
#include "vox_stream.h"

namespace vox {

StreamNativeSegment::StreamNativeSegment(StreamNativeSegmentParams *params) : m_segmentParams(*params) { m_size = params->size; }

StreamCursorInterface *StreamNativeSegment::CreateNewCursor() { return new StreamNativeSegmentCursor(this); }

void StreamNativeSegment::DestroyCursor(StreamCursorInterface *cursor) { delete cursor; }

int StreamNativeSegment::GetType() { return kStreamNativeSegment; }

StreamNativeSegmentCursor::StreamNativeSegmentCursor(StreamInterface *stream)
    : StreamCursorInterface(stream), m_pSegmentParams(static_cast<StreamNativeSegment*>(stream)->GetSegmentParams()), m_position(0) {}

StreamNativeSegmentCursor::~StreamNativeSegmentCursor() {}

void StreamNativeSegmentCursor::Shutdown() {}

int StreamNativeSegmentCursor::Seek(int offset, int origin) {
    const int size = m_pSegmentParams->size;
    int pos = origin == kSeekSet ? offset : origin == kSeekCur ? m_position + offset : size + offset;
    if (pos < 0 || pos > size) return -1;
    m_position = pos;
    return 0;
}

bool StreamNativeSegmentCursor::IsSeekable() { return true; }

int StreamNativeSegmentCursor::Tell() { return m_position; }

// the segments share one cursor of the native file, so each read goes to the place of this one first
int StreamNativeSegmentCursor::Read(unsigned char *dst, int size) {
    const int room = m_pSegmentParams->size - m_position;
    if (size > room) size = room;
    if (size <= 0 || !m_pSegmentParams->source) return 0;
    if (m_pSegmentParams->source->Seek(m_pSegmentParams->offset + m_position, kSeekSet) < 0) return 0;
    const int n = m_pSegmentParams->source->Read(dst, size);
    m_position += n;
    return n;
}

bool StreamNativeSegmentCursor::EndOfStream() { return m_position >= m_pSegmentParams->size; }

}  // namespace vox
