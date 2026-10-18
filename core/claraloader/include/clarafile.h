// core/claraloader/include/clarafile.h
#ifndef CORE_CLARALOADER_INCLUDE_CLARAFILE_H
#define CORE_CLARALOADER_INCLUDE_CLARAFILE_H

#include "folder.h"

namespace clara {

// A loaded .bclara (or one of the files inside a library): u32 version, name, u8, root folder.
class ClaraFile {
public:
    explicit ClaraFile(bool isLibrary) : m_version(0), m_isLibrary(isLibrary) {}
    ClaraFile(const ClaraFile&) = delete;
    ClaraFile& operator=(const ClaraFile&) = delete;

    void Load(jet::stream::IStream& s, Project& p);
    void Unload() { m_root.Unload(); }
    const jet::String& GetName() const { return m_name; }
    unsigned GetVersion() const { return m_version; }
    bool IsLibrary() const { return m_isLibrary; }
    Folder& GetRoot() { return m_root; }
    const Folder& GetRoot() const { return m_root; }

private:
    unsigned m_version;
    jet::String m_name;
    bool m_isLibrary;
    Folder m_root;
};

}

#endif
