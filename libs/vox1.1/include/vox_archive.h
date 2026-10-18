// libs/vox1.1/include/vox_archive.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_ARCHIVE_H
#define LIBS_VOX1_1_INCLUDE_VOX_ARCHIVE_H

namespace vox {
class VoxArchive : public vox::FileArchive {  // sizeof 0x44
public:
    VoxArchive(const char*, bool, bool, bool);
    virtual ~VoxArchive();
    virtual bool getFileInfo(const char*, int&, int&);
    virtual const char *getZipFileName();
    virtual bool isValid();
    virtual int getFileCount();
    virtual bool isVoxArchive();
    static bool isVoxArchiveFormat(const char*);
    bool overrideFile(const char*, bool&);
protected:
    static const char *getFilenameWithoutPath(const char*);
    static void MurmurHash2DualNoCase(const char*, int, unsigned, unsigned, unsigned&, unsigned&);
    static void MurmurHash2Dual(const unsigned*, int, unsigned, unsigned, unsigned&, unsigned&);
    bool collisionCheck(vox::VoxArchive*);
    bool collisionCheckStart(unsigned, unsigned);
    bool collisionCheckCallback(unsigned, unsigned);
    void ReverseInt(unsigned&);
    int ReadInt(unsigned char*);
    bool getHashIndex(const char*, int&);
    bool m_valid;  // 0x4
    bool m_usePaths;  // 0x5
    bool m_allowOverride;  // 0x6
    bool m_debug;  // 0x7
    bool m_hasValidInfo;  // 0x8
    vox::LinAlloc m_memory;  // 0xC
    int m_nbFiles;  // 0x1C
    vox::VoxArchiveEntry *m_hash;  // 0x20
    char* m_filename;  // 0x24
    unsigned m_hashSeed1;  // 0x28
    unsigned m_hashSeed2;  // 0x2C
    unsigned* m_filenameOffsets;  // 0x30
    char* m_filenameData;  // 0x34
    bool* m_hashOverride;  // 0x38
    vox::VoxArchiveIntegrityHash *m_integrityHash;  // 0x3C
    int m_collisionCheckOffset;  // 0x40
};
}

#endif
