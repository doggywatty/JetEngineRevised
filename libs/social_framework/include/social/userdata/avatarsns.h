// libs/social_framework/include/social/userdata/avatarsns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATARSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATARSNS_H

namespace social {
class AvatarSNS : public social::Avatar {  // sizeof 0xAC
public:
    AvatarSNS(const social::User*, sociallib::ClientSNSEnum);
    virtual ~AvatarSNS();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
    static bool sOnDataLoaded(void*, bool, sociallib::ClientSNSEnum);
private:
    static void sOnAvatarLoaded(const glwebtools::UrlResponse&, void*, bool, bool);
protected:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    sociallib::ClientSNSEnum m_sns;  // 0xA0
    social::request::SocialRequestHandle m_downloadRequestHandle;  // 0xA4
};
}

#endif
