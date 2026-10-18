// libs/social_framework/include/social/userdata/avatarosiris.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATAROSIRIS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATAROSIRIS_H

namespace social {
class AvatarOsiris : public social::Avatar {  // sizeof 0x120
public:
    AvatarOsiris(const social::User*, social::Profile*);
    virtual ~AvatarOsiris();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetAvatarUrl();
    void LoadForced();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    bool LoadDataFromCacheObject();
    bool UnloadCache();
    bool DiscardCache();
    bool StoreToCache(const char*, unsigned);
    static bool sOnAvatarLoadedFromCache(void*, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static void sOnGamecenterAvatarLoaded(bool, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned char*, int, void*);
    static void sOnAvatarLoaded(const glwebtools::UrlResponse&, void*, bool, bool);
protected:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    social::Profile *m_profile;  // 0xA0
    social::request::SocialRequestHandle m_downloadRequestHandle;  // 0xA4
    social::cache::CacheObjectHandle m_cacheObject;  // 0xAC
    social::cache::CacheDepotHandle m_cacheDepot;  // 0xDC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_avatarMd5;  // 0x100
};
}

#endif
