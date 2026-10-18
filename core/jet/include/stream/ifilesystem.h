// core/jet/include/stream/ifilesystem.h
#ifndef CORE_JET_INCLUDE_STREAM_IFILESYSTEM_H
#define CORE_JET_INCLUDE_STREAM_IFILESYSTEM_H

#include "core/pstring.h"

namespace jet {
namespace stream {

class IStream;

// A read-only tree of entries (e.g. the contents of an archive). Entry 0 is the root.
class IFileSystem {
public:
    struct Entry {
        Entry() : id(-1), isDirectory(false), parentId(0), childrenCount(0) {}
        explicit operator bool() const { return id >= 0; }

        int id;
        bool isDirectory;
        String name;
        unsigned parentId;
        unsigned childrenCount;
    };

    IFileSystem() {}
    IFileSystem(const IFileSystem&) = delete;
    IFileSystem& operator=(const IFileSystem&) = delete;
    virtual ~IFileSystem() {}

    virtual const Entry& GetRootEntry() const = 0;
    virtual const Entry& GetEntry(unsigned parent, unsigned child) const = 0;
    virtual const Entry& GetEntry(unsigned id) const = 0;
    virtual const Entry& FindEntryByPath(unsigned from, const String& path) const = 0;
    virtual const Entry& FindEntryByName(unsigned from, const String& name, bool recursive) const = 0;
    virtual String GetEntryFullPath(unsigned id) const = 0;
    virtual bool CanCreateStream(unsigned id) const = 0;
    virtual IStream* CreateStream(unsigned id) = 0;
};

}
}

#endif
