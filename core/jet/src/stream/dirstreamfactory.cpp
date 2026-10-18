// core/jet/src/stream/dirstreamfactory.cpp
#include "stream/dirstreamfactory.h"
#include <cstdio>
#include "stream/filestream.h"
#include "stream/streamutil.h"

namespace jet {
namespace stream {

DirStreamFactory::DirStreamFactory(const String& path, const String& ext) : m_path(path), m_extension(ext.empty() ? String("*") : ext)
{
    m_path2 = m_path;
    if (!m_path2.empty() && m_path2[m_path2.size() - 1] != '/')
        m_path2.append('/');
    RefreshStreams();
}

IStream* DirStreamFactory::CreateStream(const String& name)
{
    thread::ScopedMutex l(m_filenameMapMutex);
    if (!m_filenameMap.count(name))
        return nullptr;
    return new FileStream(m_path2 + name, FileStream::k_in, true);
}

void DirStreamFactory::RefreshStreams()
{
    thread::ScopedMutex l(m_filenameMapMutex);
    std::vector<String> files = EnumerateFiles(m_path2, String("*.") + m_extension);
    for (const String& f : files) {
        for (const char* c = f.begin(); c != f.end(); ++c) {
            if (*c != detail::tolowerascii(*c)) {
                printf("File with uppercase letters (will not be opened): %s\n", (m_path2 + f).c_str());
                break;
            }
        }
        m_filenameMap.insert(f);
    }
}

std::vector<String> DirStreamFactory::GetFileList(const String& ext) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return FilterByExtension(m_filenameMap, ext);
}

unsigned DirStreamFactory::GetFileCount() const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return unsigned(m_filenameMap.size());
}

bool DirStreamFactory::CanCreateStream(const String& name) const
{
    thread::ScopedMutex l(m_filenameMapMutex);
    return m_filenameMap.count(name) != 0;
}

}
}
