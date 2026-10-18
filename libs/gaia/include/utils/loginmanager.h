// libs/gaia/include/utils/loginmanager.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_LOGINMANAGER_H
#define LIBS_GAIA_INCLUDE_UTILS_LOGINMANAGER_H

class LoginManager {  // sizeof 0x58
    LoginManager();
    void SetAppId(const char*);
    void SaveSessionCredentials(CredentialsType, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    LoginManager::SessionCredentials *GetSessionCredentials(CredentialsType);
public:
    static LoginManager *GetInstance();
    bool SaveCredentials(CredentialsType, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    int GetCredentials(CredentialsType, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void DeleteCredentials(CredentialsType);
private:
    static LoginManager *s_instance;
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_keychain_access_group;  // 0x0
    std::map<enum CredentialsType,LoginManager::SessionCredentials,std::less<enum CredentialsType>,std::allocator<std::pair<enum CredentialsType const ,LoginManager::SessionCredentials> > > m_sessionCredentials;  // 0x20
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_FakeKeychain;  // 0x3C
};

#endif
