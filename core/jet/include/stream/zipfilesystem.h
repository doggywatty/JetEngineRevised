// core/jet/include/stream/zipfilesystem.h
#ifndef CORE_JET_INCLUDE_STREAM_ZIPFILESYSTEM_H
#define CORE_JET_INCLUDE_STREAM_ZIPFILESYSTEM_H

#include <vector>
#include "stream/ifilesystem.h"

namespace jet {
namespace stream {

// Zip archive read from its local file headers, in order. Entries are stored (mapped) or deflated.
class ZipFileSystem : public IFileSystem {
public:
    struct LocalHeader {
        u32 signature;
        u16 version, flags, method, time, date;
        u32 crc, compressedSize, size;
        u16 nameLength, extraLength;
    };

    // Lets an archive use a custom header signature or scrambled headers. Return false to stop reading.
    class Deobfuscator {
    public:
        virtual ~Deobfuscator() {}
        virtual bool Deobfuscate(LocalHeader& h) = 0;
    };

    explicit ZipFileSystem(const String& path, u32 signature = 0x04034b50);
    ZipFileSystem(const String& path, Deobfuscator* d);  // takes ownership
    virtual ~ZipFileSystem();

    virtual const Entry& GetRootEntry() const { return GetEntry(0); }
    virtual const Entry& GetEntry(unsigned parent, unsigned child) const;
    virtual const Entry& GetEntry(unsigned id) const;
    virtual const Entry& FindEntryByPath(unsigned from, const String& path) const;
    virtual const Entry& FindEntryByName(unsigned from, const String& name, bool recursive) const;
    virtual String GetEntryFullPath(unsigned id) const;
    virtual bool CanCreateStream(unsigned id) const;
    virtual IStream* CreateStream(unsigned id);

private:
    struct ZipFileEntry {
        u32 dataOffset = 0;
        u16 method = 0;
        u32 compressedSize = 0;
        u32 size = 0;
    };
    struct EntryData {
        Entry entry;
        std::vector<unsigned> children;
        ZipFileEntry zipData;
    };

    void LoadFileSystem(IStream* s);

    String m_path;
    u32 m_signature;
    Deobfuscator* m_deobfuscator;
    Entry m_emptyEntry;
    std::vector<EntryData> m_entries;
};

}
}

#endif
