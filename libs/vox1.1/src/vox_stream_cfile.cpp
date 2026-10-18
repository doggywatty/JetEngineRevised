// libs/vox1.1/src/vox_stream_cfile.cpp
#include "vox_stream.h"

namespace vox {

StreamCFile::StreamCFile(const char *filename) : m_filename(filename ? filename : "") {}

// the size of the file, found once
void StreamCFile::Init() {
    if (m_size >= 0) return;
    FileInterface *f = FileInterface::Open(m_filename.c_str(), kAccessRead);
    if (!f) return;
    f->Seek(0, kSeekEnd);
    m_size = f->Tell();
    delete f;
}

StreamCursorInterface *StreamCFile::CreateNewCursor() {
    StreamCFileCursor *c = new StreamCFileCursor(this);
    c->Init();
    return c;
}

void StreamCFile::DestroyCursor(StreamCursorInterface *cursor) {
    if (!cursor) return;
    cursor->Shutdown();
    delete cursor;
}

int StreamCFile::GetType() { return kStreamCFile; }

StreamCFileCursor::StreamCFileCursor(StreamInterface *stream) : StreamCursorInterface(stream), m_fp(0), m_position(0), m_isValid(false) {}

StreamCFileCursor::~StreamCFileCursor() { Shutdown(); }

void StreamCFileCursor::Init() {
    Shutdown();
    StreamCFile *s = static_cast<StreamCFile*>(m_pStream);
    m_fp = FileInterface::Open(s->GetFileName(), kAccessRead);
    m_position = 0;
    m_isValid = m_fp != 0;
}

void StreamCFileCursor::Shutdown() {
    delete m_fp;
    m_fp = 0;
    m_isValid = false;
}

int StreamCFileCursor::Seek(int offset, int origin) {
    if (!m_fp) return -1;
    const int r = m_fp->Seek(offset, (VoxFileSeekOrigin)origin);
    m_position = m_fp->Tell();
    return r;
}

bool StreamCFileCursor::IsSeekable() { return true; }

int StreamCFileCursor::Tell() { return m_position; }

int StreamCFileCursor::Read(unsigned char *dst, int size) {
    if (!m_fp) return 0;
    const int n = m_fp->Read(dst, 1, size);
    m_position += n;
    return n;
}

bool StreamCFileCursor::EndOfStream() {
    const int size = m_pStream->GetSize();
    return !m_fp || (size >= 0 && m_position >= size);
}

}  // namespace vox
