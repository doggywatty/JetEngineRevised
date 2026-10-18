// core/jet/include/stream/streammgr.h
#ifndef CORE_JET_INCLUDE_STREAM_STREAMMGR_H
#define CORE_JET_INCLUDE_STREAM_STREAMMGR_H

#include <memory>
#include <vector>
#include "core/pstring.h"
#include "thread/mutex.h"

namespace jet {
namespace stream {

class IFileSystem;
class IStream;
class IStreamFactory;

// Resolves stream names against the registered factories, in order. A name that is not found directly is
// retried with each encryption suffix; a hit there is wrapped in an EncryptedStream using the pair's key.
class StreamMgr {
public:
    static StreamMgr& GetInstance();
    static void FreeInstance();

    std::shared_ptr<IStreamFactory> FindStreamFactoryByPath(const String& path);
    std::shared_ptr<IStreamFactory> FindStreamFactoryByFilename(const String& name);
    std::shared_ptr<IStreamFactory> AddPath(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext);
    std::shared_ptr<IStreamFactory> AddPath(const String& path, const String& ext);
    std::shared_ptr<IStreamFactory> AddPathFront(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext);
    std::shared_ptr<IStreamFactory> AddPathFront(const String& path, const String& ext);
    std::shared_ptr<IStreamFactory> GetFactoryFromPath(const String& path, const String& ext);
    bool RemovePath(const String& path);
    void RemoveAllPaths();
    bool RemoveFactory(std::shared_ptr<IStreamFactory> f);

    bool CanCreateStream(const String& name, bool tryEncrypted = true) const;
    IStream* CreateStream(const String& name);
    std::vector<IStream*> CreateStreamArray(const String& name);
    void RefreshStreams();
    std::vector<String> GetFileList(const String& ext) const;
    unsigned GetFileCount() const;

    void AddFileSystem(std::shared_ptr<IFileSystem> fs);
    void AddFileSystemFront(std::shared_ptr<IFileSystem> fs);
    bool CanCreateStreamByPath(const String& path) const;
    IStream* CreateStreamByPath(const String& path) const;
    std::vector<IStream*> CreateStreamsByPath(const String& path) const;
    bool CanCreateStreamByName(const String& name, bool tryEncrypted = true) const;
    IStream* CreateStreamByName(const String& name) const;
    std::vector<IStream*> CreateStreamsByName(const String& name) const;
    bool IsFile(const String& path) const;
    bool IsDirectory(const String& path) const;

    void AddEncryptionPair(const String& key, const String& suffix);

private:
    struct StreamFactoryData {
        std::shared_ptr<IFileSystem> filesystem;
        std::shared_ptr<IStreamFactory> factory;
    };
    struct EncryptionPair {
        String key;
        String suffix;
    };

    StreamMgr() {}
    ~StreamMgr() {}
    std::shared_ptr<IStreamFactory> Add(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext, bool front);

    mutable thread::RecursiveMutex m_streamFactoryArrayMutex;
    std::vector<StreamFactoryData> m_streamFactoryArray;
    std::vector<EncryptionPair> m_encryptionPairArray;
    std::vector<std::shared_ptr<IFileSystem>> m_fileSystems;
    static StreamMgr* s_instance;
};

}
}

#endif
