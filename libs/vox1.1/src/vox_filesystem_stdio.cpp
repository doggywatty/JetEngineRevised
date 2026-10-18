// libs/vox1.1/src/vox_filesystem_stdio.cpp
// The engine's own file access (what the game replaces by its streams, see SoundMgr::InitFileSystem @0x63d174)
#include "vox_filesystem.h"

namespace vox {

static int StdioRead(void *dst, int size, int count, void *f) { return (int)fread(dst, (size_t)size, (size_t)count, (FILE*)f); }
static int StdioWrite(const void *src, int size, int count, void *f) { return (int)fwrite(src, (size_t)size, (size_t)count, (FILE*)f); }
static int StdioSeek(void *f, int offset, VoxFileSeekOrigin origin) {
    return fseek((FILE*)f, offset, origin == kSeekSet ? SEEK_SET : origin == kSeekCur ? SEEK_CUR : SEEK_END);
}
static int StdioTell(void *f) { return (int)ftell((FILE*)f); }
static void *StdioOpen(const char *path, VoxFileAccessMode mode) { return fopen(path, mode == kAccessWrite ? "wb" : "rb"); }
static int StdioClose(void *f) { return fclose((FILE*)f); }

FileSystemStdio::FileSystemStdio() {}

FileSystemStdio::~FileSystemStdio() {}

FileSystemInterface FileSystemStdio::Table() {
    FileSystemInterface t;
    t.read = StdioRead;
    t.write = StdioWrite;
    t.seek = StdioSeek;
    t.tell = StdioTell;
    t.open = StdioOpen;
    t.close = StdioClose;
    return t;
}

}  // namespace vox
