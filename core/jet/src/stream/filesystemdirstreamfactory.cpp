// core/jet/src/stream/filesystemdirstreamfactory.cpp
#include "stream/filesystemdirstreamfactory.h"

namespace jet {
namespace stream {

FileSystemDirStreamFactory::FileSystemDirStreamFactory(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext)
    : m_path(path), m_filesystem(fs), m_extension(ext)
{
    if (m_filesystem) {
        m_parent = m_filesystem->FindEntryByPath(0, path);
        RefreshStreams();
    }
}

IStream* FileSystemDirStreamFactory::CreateStream(const String& name)
{
    thread::ScopedMutex l(m_filenameMapMutex);
    if (!m_parent)
        return nullptr;
    const IFileSystem::Entry& e = m_filesystem->FindEntryByName(unsigned(m_parent.id), name, false);
    return e ? m_filesystem->CreateStream(unsigned(e.id)) : nullptr;
}

void FileSystemDirStreamFactory::RefreshStreams()
{
    thread::ScopedMutex l(m_filenameMapMutex);
    m_filenameMap.clear();
    if (!m_parent)
        return;
    for (unsigned i = 0; i < m_parent.childrenCount; ++i) {
        const IFileSystem::Entry& e = m_filesystem->GetEntry(unsigned(m_parent.id), i);
        if (!e.isDirectory)
            m_filenameMap.insert(e.name);
    }
}

std::vector<String> FileSystemDirStreamFactory::GetFileList(const String& ext) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return FilterByExtension(m_filenameMap, ext);
}

unsigned FileSystemDirStreamFactory::GetFileCount() const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return unsigned(m_filenameMap.size());
}

bool FileSystemDirStreamFactory::CanCreateStream(const String& name) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return m_filenameMap.count(name) != 0;
}

}
}
