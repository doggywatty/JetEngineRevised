// libs/social_framework/include/social/downloadable/downloadablesmanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_DOWNLOADABLE_DOWNLOADABLESMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_DOWNLOADABLE_DOWNLOADABLESMANAGER_H

namespace social::downloadable {
class DownloadablesManager {  // sizeof 0x40
public:
    DownloadablesManager();
    ~DownloadablesManager();
    social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock> CreateIrisDownloadable(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool HasDownloadable(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock> GetDownloadable(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Update();
private:
    social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock> FindDownloadableByKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock>,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock> > > > m_downloadablesByKey;  // 0x0
    social::cache::CacheDepotHandle m_cacheDepot;  // 0x1C
};
}

#endif
