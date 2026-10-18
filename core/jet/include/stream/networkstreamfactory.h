// core/jet/include/stream/networkstreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_NETWORKSTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_NETWORKSTREAMFACTORY_H

namespace jet::stream {
class NetworkStreamFactory : public jet::stream::IStreamFactory {  // sizeof 0x58
public:
    NetworkStreamFactory(net::CNetSocket*, jet::thread::RecursiveMutex*, const jet::String&, const jet::String&, const jet::stream::CacheFolder&, const jet::String&);
    NetworkStreamFactory(net::CNetSocket*, jet::thread::RecursiveMutex*, const jet::String&, const jet::String&, const jet::String&);
    NetworkStreamFactory(const jet::String&, unsigned, const jet::String&, const jet::String&, const jet::String&);
    virtual ~NetworkStreamFactory();
    virtual jet::stream::IStream *CreateStream(const jet::String&);
    virtual void RefreshStreams();
    virtual std::vector<jet::String,std::allocator<jet::String> > GetFileList(const jet::String&);
    virtual unsigned GetFileCount();
    virtual jet::String GetPath();
    virtual jet::String GetExtension();
    virtual bool CanCreateStream(const jet::String&);
private:
    bool Init();
    void UpdateRemoteFileList();
    bool GetRemoteFileTime(const jet::String&, __int64&);
    bool GetLocalFileTime(const jet::String&, __int64&);
    jet::stream::IStream *CreateLocalFileStream(const jet::String&);
    bool RequestFileList(std::vector<jet::String,std::allocator<jet::String> >&);
    bool RequestFileTime(const jet::String&, __int64&);
    bool RequestFile(const jet::String&);
    jet::String m_serverPath;  // 0x4
    jet::String m_localPath;  // 0x8
    jet::String m_extension;  // 0xC
    jet::stream::MMapStream *m_mmapStream;  // 0x10
    std::vector<jet::String,std::allocator<jet::String> > m_fileList;  // 0x14
    boost::unordered::unordered_map<jet::String,__int64,boost::hash<jet::String>,std::equal_to<jet::String>,boost::fast_pool_allocator<__int64,boost::default_user_allocator_new_delete,boost::mutex,32,0> > m_fileMap;  // 0x28
    jet::String m_server;  // 0x44
    unsigned m_port;  // 0x48
    net::CNetInterface *m_net;  // 0x4C
    net::CNetSocket *m_socket;  // 0x50
    jet::thread::RecursiveMutex *m_mutex;  // 0x54
};
}

namespace jet::stream {
class NetworkStreamFactoryManager {  // sizeof 0x50
public:
    static void Init();
    static void DeInit();
    static jet::stream::NetworkStreamFactoryManager *GetInstance();
    jet::String GetCacheFolder();
    bool CreateLocalServer(unsigned);
    void DestroyLocalServer();
    bool IsClientConnected();
    void UpdateLocalServer();
protected:
    bool UpdateCache(const std::vector<jet::String,std::allocator<jet::String> >&);
public:
    void UpdateCache();
    boost::shared_ptr<jet::stream::NetworkStreamFactory> NewLocalServer(const jet::String&, const jet::String&, const jet::String&);
    boost::shared_ptr<jet::stream::NetworkStreamFactory> NewRemoteServer(const jet::String&, unsigned, const jet::String&, const jet::String&, const jet::String&);
    void DestroyFactories();
protected:
    NetworkStreamFactoryManager();
    ~NetworkStreamFactoryManager();
    bool RequestLocalServerList();
public:
    static const int k_requestFileList;
    static const int k_requestFileTime;
    static const int k_requestFileData;
    static const int k_requestError;
    static const int k_requestServerList;
    static const int k_requestFiles;
protected:
    net::CNetInterface *m_net;  // 0x0
    std::map<std::pair<jet::String,unsigned int>,jet::stream::NetworkStreamFactoryManager::Connection *,std::less<std::pair<jet::String,unsigned int> >,std::allocator<std::pair<std::pair<jet::String,unsigned int> const ,jet::stream::NetworkStreamFactoryManager::Connection *> > > m_remoteServers;  // 0x4
    jet::stream::NetworkStreamFactoryManager::Connection m_localServer;  // 0x20
    std::vector<boost::shared_ptr<jet::stream::NetworkStreamFactory>,std::allocator<boost::shared_ptr<jet::stream::NetworkStreamFactory> > > m_factories;  // 0x28
    std::vector<jet::stream::CacheFolder,std::allocator<jet::stream::CacheFolder> > m_folders;  // 0x3C
    static jet::stream::NetworkStreamFactoryManager *s_instance;
};
}

#endif
