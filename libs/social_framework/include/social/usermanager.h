// libs/social_framework/include/social/usermanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERMANAGER_H

namespace social {
class UserManager : public social::SSingleton<social::UserManager> {  // sizeof 0x40
public:
    UserManager();
    virtual ~UserManager();
    social::UserOsiris *GetUserOsiris(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::User *GetUser(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void CleanUsers();
    void RegisterUser(social::User*);
    void UnregisterUser(social::User*);
    social::UserOsiris *GetPlayer();
    const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::User *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::User *> > > &GetAllKnownUsers();
    social::UserSNS *GetPlayerSNS(sociallib::ClientSNSEnum);
private:
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::User *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::User *> > > m_userDictionary;  // 0x4
    std::map<enum sociallib::ClientSNSEnum,social::UserSNS *,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,social::UserSNS *> > > m_userSNSMap;  // 0x20
    social::UserOsiris *m_player;  // 0x3C
};
}

#endif
