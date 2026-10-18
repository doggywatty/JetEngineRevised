// core/jet/src/stream/libstreamfactory.cpp
#include "stream/libstreamfactory.h"
#include "stream/ifilesystem.h"
#include "stream/memlinkstream.h"
#include "stream/mmapstream.h"

namespace jet {
namespace stream {

LibStreamFactory::LibStreamFactory(const String& path, const String& ext)
    : m_path(path), m_extension(ext), m_stream(new MMapStream(path)), m_mappedMemory(nullptr), m_chunkCount(0)
{
    Init();
}

LibStreamFactory::LibStreamFactory(IStream* stream, const String& path, const String& ext)
    : m_path(path), m_extension(ext), m_stream(stream), m_mappedMemory(nullptr), m_chunkCount(0)
{
    Init();
}

LibStreamFactory::LibStreamFactory(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext)
    : m_path(path), m_extension(ext), m_stream(nullptr), m_mappedMemory(nullptr), m_chunkCount(0)
{
    const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
    m_stream = fs->CreateStream(unsigned(e.id));
    Init();
}

LibStreamFactory::~LibStreamFactory()
{
    if (m_stream) {
        if (m_stream->IsMapped())
            m_stream->Unmap();
        m_stream->Finish();
        m_mappedMemory = nullptr;
        delete m_stream;
    }
}

void LibStreamFactory::Init()
{
    if (!m_stream)
        return;
    m_stream->Start();
    if (!m_stream->IsStarted())
        return;
    m_mappedMemory = static_cast<u8*>(m_stream->Map(m_stream->Size()));
    if (!m_mappedMemory)
        return;
    *m_stream >> m_chunkCount;
    if (!m_chunkCount)
        return;
    if (m_chunkCount >= m_stream->Size()) {
        m_chunkCount = 0;
        m_stream->Unmap();
        m_mappedMemory = nullptr;
        return;
    }
    m_offsets.resize(m_chunkCount);
    m_stream->Read(m_offsets.data(), m_chunkCount * 4);
    m_offsets.push_back(m_stream->Size());
    bool filter = !m_extension.empty() && m_extension != "*";
    thread::ScopedMutex l(m_filenameToStreamIdxMapMutex);
    for (unsigned i = 0; i < m_chunkCount; ++i) {
        String name;
        *m_stream >> name;
        if (filter) {
            unsigned dot = name.find_last_of('.');
            if (dot == String::npos || !m_extension.Equals(name.substr(dot + 1)))
                continue;
        }
        m_filenameToStreamIdxMap[name] = i;
    }
    m_padding.resize(m_chunkCount);
    m_stream->Read(m_padding.data(), m_chunkCount);
}

IStream* LibStreamFactory::CreateStream(const String& name)
{
    thread::ScopedMutex l(m_filenameToStreamIdxMapMutex);
    auto it = m_filenameToStreamIdxMap.find(name);
    if (it == m_filenameToStreamIdxMap.end())
        return nullptr;
    unsigned i = it->second;
    unsigned size = m_offsets[i + 1] - m_offsets[i] - m_padding[i];
    if (!size)
        return nullptr;
    return new MemLinkStream(static_cast<const u8*>(m_mappedMemory + m_offsets[i]), size);
}

std::vector<String> LibStreamFactory::GetFileList(const String& ext) const
{
    thread::ScopedMutex l(m_filenameToStreamIdxMapMutex);
    std::vector<String> names;
    for (const auto& kv : m_filenameToStreamIdxMap)
        names.push_back(kv.first);
    return FilterByExtension(names, ext);
}

bool LibStreamFactory::CanCreateStream(const String& name) const
{
    thread::ScopedMutex l(m_filenameToStreamIdxMapMutex);
    return m_filenameToStreamIdxMap.count(name) != 0;
}

}
}
