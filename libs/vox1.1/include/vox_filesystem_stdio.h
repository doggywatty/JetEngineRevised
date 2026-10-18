// libs/vox1.1/include/vox_filesystem_stdio.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_FILESYSTEM_STDIO_H
#define LIBS_VOX1_1_INCLUDE_VOX_FILESYSTEM_STDIO_H

namespace vox {
class FileSystemStdio : public vox::FileSystemInterface {  // sizeof 0x10
public:
    FileSystemStdio();
    virtual ~FileSystemStdio();
};
}

#endif
