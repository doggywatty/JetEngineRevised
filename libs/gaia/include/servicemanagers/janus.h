// libs/gaia/include/servicemanagers/janus.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_JANUS_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_JANUS_H

namespace gaia {
class Janus : public gaia::BaseServiceManager {  // sizeof 0x14C
public:
    Janus(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Janus();
    int GetJanusToken(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetJanusToken(gaia::BaseServiceManager::Credentials);
    int GetJanusRefreshToken(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void FlushTokens(gaia::BaseServiceManager::Credentials);
    void FlushTokens();
    int GetCredentialListForAccount(void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int AuthorizeExclusive(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::GaiaRequest*);
    int Authorize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::GaiaRequest*);
    int checkAccountExists(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::GaiaRequest*);
    int sendAuthentificate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::GaiaRequest*);
    int SetDeviceInfo(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetDeviceInfo(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int CreateAccount(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int ChangeCredential(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int RetrievePassword(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::GaiaRequest*);
    int AddCredential(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int RemoveCredential(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int AddPermission(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int VerifyToken(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int EncryptToken(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int RefreshAccessToken(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int FindUserByAlias(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int AddAlias(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
private:
    void UpdateTokens();
    std::map<enum gaia::BaseServiceManager::Credentials,gaia::Janus::JanusToken,std::less<enum gaia::BaseServiceManager::Credentials>,std::allocator<std::pair<enum gaia::BaseServiceManager::Credentials const ,gaia::Janus::JanusToken> > > m_JanusTokenList;  // 0x128
    glwebtools::Mutex m_authorizeMutex;  // 0x144
};
}

#endif
