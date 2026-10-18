// source/version.h
#ifndef SOURCE_VERSION_H
#define SOURCE_VERSION_H

#include <string>

namespace despicableme {
class Version {  // sizeof 0x1
public:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGameVersion();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetProductId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGGI();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPlatformId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetDlcPlatformId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetGameCode();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetStoreType();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFederationClientId();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetIAPDeviceType();
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > VERSION;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > PRODUCT_ID;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GGI_NUM;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > PLATFORM_ID;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > DLC_PLATFORM_ID;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GAME_CODE;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > STORE_TYPE;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > IAP_DEVICE_TYPE;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_igpCode;
};
}

#endif
