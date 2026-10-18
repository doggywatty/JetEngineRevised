// core/jet/src/stream/streammgr.cpp
#include "stream/streammgr.h"
#include "stream/encryptedstream.h"
#include "stream/ifilesystem.h"
#include "stream/istreamfactory.h"
#include "stream/streamutil.h"

namespace jet {
namespace stream {

StreamMgr* StreamMgr::s_instance = nullptr;

StreamMgr& StreamMgr::GetInstance()
{
    if (!s_instance)
        s_instance = new StreamMgr;
    return *s_instance;
}

void StreamMgr::FreeInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

std::shared_ptr<IStreamFactory> StreamMgr::Add(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext, bool front)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    std::shared_ptr<IStreamFactory> f = CreateStreamFactoryFromPath(fs, path, ext);
    if (f) {
        StreamFactoryData d{fs, f};
        if (front)
            m_streamFactoryArray.insert(m_streamFactoryArray.begin(), d);
        else
            m_streamFactoryArray.push_back(d);
    }
    return f;
}

std::shared_ptr<IStreamFactory> StreamMgr::AddPath(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext)
{
    return Add(fs, path, ext, false);
}

std::shared_ptr<IStreamFactory> StreamMgr::AddPath(const String& path, const String& ext) { return Add(nullptr, path, ext, false); }

std::shared_ptr<IStreamFactory> StreamMgr::AddPathFront(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext)
{
    return Add(fs, path, ext, true);
}

std::shared_ptr<IStreamFactory> StreamMgr::AddPathFront(const String& path, const String& ext) { return Add(nullptr, path, ext, true); }

std::shared_ptr<IStreamFactory> StreamMgr::FindStreamFactoryByPath(const String& path)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (d.factory->GetPath().Equals(path))
            return d.factory;
    return nullptr;
}

std::shared_ptr<IStreamFactory> StreamMgr::FindStreamFactoryByFilename(const String& name)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (d.factory->CanCreateStream(name))
            return d.factory;
    return nullptr;
}

std::shared_ptr<IStreamFactory> StreamMgr::GetFactoryFromPath(const String& path, const String& ext)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (d.factory->GetPath().Equals(path) && d.factory->GetExtension().Equals(ext))
            return d.factory;
    return nullptr;
}

bool StreamMgr::RemovePath(const String& path)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (auto it = m_streamFactoryArray.begin(); it != m_streamFactoryArray.end(); ++it) {
        if (it->factory->GetPath().Equals(path)) {
            m_streamFactoryArray.erase(it);
            return true;
        }
    }
    return false;
}

void StreamMgr::RemoveAllPaths()
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    m_streamFactoryArray.clear();
}

bool StreamMgr::RemoveFactory(std::shared_ptr<IStreamFactory> f)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (auto it = m_streamFactoryArray.begin(); it != m_streamFactoryArray.end(); ++it) {
        if (it->factory == f) {
            m_streamFactoryArray.erase(it);
            return true;
        }
    }
    return false;
}

bool StreamMgr::CanCreateStream(const String& name, bool tryEncrypted) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (d.factory->CanCreateStream(name))
            return true;
    if (tryEncrypted)
        for (const EncryptionPair& p : m_encryptionPairArray)
            for (const StreamFactoryData& d : m_streamFactoryArray)
                if (d.factory->CanCreateStream(name + p.suffix))
                    return true;
    return false;
}

IStream* StreamMgr::CreateStream(const String& name)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (IStream* s = d.factory->CreateStream(name))
            return s;
    for (const EncryptionPair& p : m_encryptionPairArray)
        for (const StreamFactoryData& d : m_streamFactoryArray)
            if (IStream* s = d.factory->CreateStream(name + p.suffix))
                return new EncryptedStream(s, p.key);
    return nullptr;
}

std::vector<IStream*> StreamMgr::CreateStreamArray(const String& name)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    std::vector<IStream*> r;
    for (const StreamFactoryData& d : m_streamFactoryArray)
        if (d.factory->CanCreateStream(name))
            r.push_back(d.factory->CreateStream(name));
    for (const EncryptionPair& p : m_encryptionPairArray) {
        String n = name + p.suffix;
        for (const StreamFactoryData& d : m_streamFactoryArray)
            if (d.factory->CanCreateStream(n))
                if (IStream* s = d.factory->CreateStream(n))
                    r.push_back(new EncryptedStream(s, p.key));
    }
    return r;
}

void StreamMgr::RefreshStreams()
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const StreamFactoryData& d : m_streamFactoryArray)
        d.factory->RefreshStreams();
}

std::vector<String> StreamMgr::GetFileList(const String& ext) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    std::vector<String> r;
    r.reserve(1000);
    for (const StreamFactoryData& d : m_streamFactoryArray) {
        std::vector<String> f = d.factory->GetFileList(ext);
        r.insert(r.end(), f.begin(), f.end());
    }
    return r;
}

unsigned StreamMgr::GetFileCount() const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    unsigned n = 0;
    for (const StreamFactoryData& d : m_streamFactoryArray)
        n += d.factory->GetFileCount();
    return n;
}

void StreamMgr::AddFileSystem(std::shared_ptr<IFileSystem> fs)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    m_fileSystems.push_back(fs);
}

void StreamMgr::AddFileSystemFront(std::shared_ptr<IFileSystem> fs)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    m_fileSystems.insert(m_fileSystems.begin(), fs);
}

bool StreamMgr::CanCreateStreamByPath(const String& path) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const auto& fs : m_fileSystems) {
        const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
        if (e && fs->CanCreateStream(unsigned(e.id)))
            return true;
    }
    return false;
}

IStream* StreamMgr::CreateStreamByPath(const String& path) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const auto& fs : m_fileSystems) {
        const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
        if (e && fs->CanCreateStream(unsigned(e.id)))
            return fs->CreateStream(unsigned(e.id));
    }
    return nullptr;
}

std::vector<IStream*> StreamMgr::CreateStreamsByPath(const String& path) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    std::vector<IStream*> r;
    for (const auto& fs : m_fileSystems) {
        const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
        if (e && fs->CanCreateStream(unsigned(e.id)))
            if (IStream* s = fs->CreateStream(unsigned(e.id)))
                r.push_back(s);
    }
    return r;
}

bool StreamMgr::CanCreateStreamByName(const String& name, bool tryEncrypted) const { return CanCreateStream(name, tryEncrypted); }

IStream* StreamMgr::CreateStreamByName(const String& name) const { return const_cast<StreamMgr*>(this)->CreateStream(name); }

std::vector<IStream*> StreamMgr::CreateStreamsByName(const String& name) const
{
    return const_cast<StreamMgr*>(this)->CreateStreamArray(name);
}

bool StreamMgr::IsFile(const String& path) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const auto& fs : m_fileSystems) {
        const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
        if (e && !e.isDirectory)
            return true;
    }
    return false;
}

bool StreamMgr::IsDirectory(const String& path) const
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    for (const auto& fs : m_fileSystems) {
        const IFileSystem::Entry& e = fs->FindEntryByPath(0, path);
        if (e && e.isDirectory)
            return true;
    }
    return false;
}

void StreamMgr::AddEncryptionPair(const String& key, const String& suffix)
{
    thread::ScopedMutex l(m_streamFactoryArrayMutex);
    m_encryptionPairArray.push_back(EncryptionPair{key, suffix});
}

}
}
