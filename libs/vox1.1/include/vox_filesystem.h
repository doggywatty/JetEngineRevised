// libs/vox1.1/include/vox_filesystem.h
// How the engine reads its files. The game gives six callbacks (SoundMgr::InitFileSystem @0x63d174 stores OpenSoundFile
// @0x63b96c, CloseSoundFile @0x63b9d0, ReadSoundFile @0x63b8b8, WriteSoundFile @0x63b8e8, SeekSoundFile @0x63b8f0 and
// TellSoundFile @0x63b958 in the engine's file system table); FileSystemStdio is what the engine uses until it does.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_FILESYSTEM_H
#define LIBS_VOX1_1_INCLUDE_VOX_FILESYSTEM_H

#include <cstdio>

namespace vox {

enum VoxFileSeekOrigin { kSeekSet = 0, kSeekCur = 1, kSeekEnd = 2 };
enum VoxFileAccessMode { kAccessRead = 0, kAccessWrite = 1 };

// the table of callbacks (the .so's layout: +0 read, +4 write, +8 seek, +0xC tell, +0x10 open, +0x14 close)
struct FileSystemInterface {
    int (*read)(void *dst, int size, int count, void *file);     // the number of items read
    int (*write)(const void *src, int size, int count, void *file);
    int (*seek)(void *file, int offset, VoxFileSeekOrigin origin);
    int (*tell)(void *file);
    void *(*open)(const char *path, VoxFileAccessMode mode);
    int (*close)(void *file);
};

// the table in use (the stdio one until the game replaces its entries)
FileSystemInterface &GetFileSystem();

class FileSystemStdio {
public:
    FileSystemStdio();
    virtual ~FileSystemStdio();
    static FileSystemInterface Table();
};

// an open file
class FileInterface {  // sizeof 0x8
public:
    FileInterface(void *file, const char *path);
    FileInterface();
    virtual ~FileInterface();
    virtual int Read(void *dst, int size, int count);
    virtual int Seek(int offset, VoxFileSeekOrigin origin);
    virtual int Tell();
    virtual int Write(const void *src, int size, int count);
    virtual const char *GetFilePath();
    virtual void *GetFilePtr();
    // opens a file with the table's callbacks; 0 when it cannot be opened
    static FileInterface *Open(const char *path, VoxFileAccessMode mode);
protected:
    void *m_file;
    char *m_path;
};

// a window of a file (a segment of a native file)
class FileLimited : public FileInterface {  // sizeof 0x14
public:
    FileLimited(void *file, const char *path, int baseOffset, int size);
    FileLimited();
    virtual ~FileLimited();
    virtual int Read(void *dst, int size, int count);
    virtual int Seek(int offset, VoxFileSeekOrigin origin);
    virtual int Tell();
    virtual int Write(const void *src, int size, int count);
protected:
    int m_baseOffset;  // 0x8
    int m_fileSize;  // 0xC
    int m_filePosition;  // 0x10
};

}  // namespace vox

#endif
