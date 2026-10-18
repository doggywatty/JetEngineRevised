// libs/vox1.1/src/vox_filesystem.cpp
#include "vox_filesystem.h"
#include <cstdlib>
#include <cstring>

namespace vox {

FileSystemInterface &GetFileSystem() {
    static FileSystemInterface table = FileSystemStdio::Table();
    return table;
}

FileInterface::FileInterface(void *file, const char *path) : m_file(file), m_path(0) {
    if (path) {
        m_path = (char*)malloc(strlen(path) + 1);
        strcpy(m_path, path);
    }
}

FileInterface::FileInterface() : m_file(0), m_path(0) {}

FileInterface::~FileInterface() {
    if (m_file) GetFileSystem().close(m_file);
    free(m_path);
}

int FileInterface::Read(void *dst, int size, int count) { return GetFileSystem().read(dst, size, count, m_file); }

int FileInterface::Seek(int offset, VoxFileSeekOrigin origin) { return GetFileSystem().seek(m_file, offset, origin); }

int FileInterface::Tell() { return GetFileSystem().tell(m_file); }

int FileInterface::Write(const void *src, int size, int count) { return GetFileSystem().write(src, size, count, m_file); }

const char *FileInterface::GetFilePath() { return m_path; }

void *FileInterface::GetFilePtr() { return m_file; }

FileInterface *FileInterface::Open(const char *path, VoxFileAccessMode mode) {
    void *f = GetFileSystem().open(path, mode);
    return f ? new FileInterface(f, path) : 0;
}

FileLimited::FileLimited(void *file, const char *path, int baseOffset, int size)
    : FileInterface(file, path), m_baseOffset(baseOffset), m_fileSize(size), m_filePosition(0) {
    FileInterface::Seek(baseOffset, kSeekSet);
}

FileLimited::FileLimited() : m_baseOffset(0), m_fileSize(0), m_filePosition(0) {}

FileLimited::~FileLimited() {}

int FileLimited::Read(void *dst, int size, int count) {
    const int room = m_fileSize - m_filePosition;
    if (size > 0 && count * size > room) count = room / size;
    if (count <= 0) return 0;
    const int n = FileInterface::Read(dst, size, count);
    m_filePosition += n * size;
    return n;
}

int FileLimited::Seek(int offset, VoxFileSeekOrigin origin) {
    int pos = origin == kSeekSet ? offset : origin == kSeekCur ? m_filePosition + offset : m_fileSize + offset;
    if (pos < 0) pos = 0;
    if (pos > m_fileSize) pos = m_fileSize;
    m_filePosition = pos;
    return FileInterface::Seek(m_baseOffset + pos, kSeekSet);
}

int FileLimited::Tell() { return m_filePosition; }

int FileLimited::Write(const void *, int, int) { return -1; }

}  // namespace vox
