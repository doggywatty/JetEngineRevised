// libs/social_framework/include/social/cache/cachemanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEMANAGER_H

namespace social::cache {
class CacheManager {  // sizeof 0x54
public:
    CacheManager(const social::cache::CacheManager::CreationSettings&);
    ~CacheManager();
    social::cache::CacheManagerStatus GetStatus();
    unsigned GetDepotCount();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPath();
    const social::cache::CacheInfo &GetInfo();
    social::cache::Error Initialize();
    void Update();
    social::cache::Error InitializeDepot(const social::cache::CacheDepot::CreationSettings&);
    social::cache::Error EraseDepot(social::cache::CacheDepotHandle&);
    social::cache::Error UnloadDepot(social::cache::CacheDepotHandle&);
    social::cache::CacheDepotHandle GetDepot(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool HasDepot(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    social::cache::CacheDepot *GetDepotPtr(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::Error InitializeManagerFromDisk();
    social::cache::Error LoadDepot(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::Error AddDepot(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::Error InitDepot(const social::cache::CacheDepot::CreationSettings&);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::cache::CacheDepot *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::cache::CacheDepot *> > > m_depots;  // 0x0
    social::cache::CacheRequestManager *m_requestMgr;  // 0x1C
    social::cache::CacheManager::CreationSettings m_settings;  // 0x20
    social::cache::CacheManagerStatus m_status;  // 0x40
    social::cache::CacheInfo m_info;  // 0x44
};
}

#endif
