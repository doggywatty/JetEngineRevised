// core/jet/src/stream/zipstreamfactory.cpp
#include "stream/zipstreamfactory.h"
#include "stream/zipfilesystem.h"

namespace jet {
namespace stream {

ZipStreamFactory::ZipStreamFactory(const String& path, const String& ext)
    : m_path(path), m_extension(ext), m_zfs(std::make_shared<ZipFileSystem>(path))
{
    RefreshStreams();
}

IStream* ZipStreamFactory::CreateStream(const String& name)
{
    thread::ScopedMutex l(m_filenameMapMutex);
    const IFileSystem::Entry& e = m_zfs->FindEntryByName(0, name, false);
    return e ? m_zfs->CreateStream(unsigned(e.id)) : nullptr;
}

void ZipStreamFactory::RefreshStreams()
{
    thread::ScopedMutex l(m_filenameMapMutex);
    m_filenameMap.clear();
    const IFileSystem::Entry& root = m_zfs->GetRootEntry();
    for (unsigned i = 0; i < root.childrenCount; ++i)
        m_filenameMap.insert(m_zfs->GetEntry(unsigned(root.id), i).name);
}

std::vector<String> ZipStreamFactory::GetFileList(const String& ext) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return FilterByExtension(m_filenameMap, ext);
}

unsigned ZipStreamFactory::GetFileCount() const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return unsigned(m_filenameMap.size());
}

bool ZipStreamFactory::CanCreateStream(const String& name) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return m_filenameMap.count(name) != 0;
}

}
}
