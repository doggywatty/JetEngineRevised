// libs/gaia/include/gaiaimplementation/gaia_janus.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_JANUS_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_JANUS_H

namespace gaia {
class Gaia_Janus {  // sizeof 0x8
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetJanusStatus();
    int StartAndAuthorizeJanus(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int StartAndAuthorizeJanus(gaia::GaiaRequest&);
    int GetJanusRefreshToken(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetJanusRefreshToken(gaia::GaiaRequest&);
    int GetJanusToken(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetJanusToken(gaia::BaseServiceManager::Credentials);
    int GetJanusToken(gaia::GaiaRequest&);
    int GetMinimalFedAccountList(std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CheckForConflictingCredentials(gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetCredentialListForAccount(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetCredentialListForAccount(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int Authorize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int Authorize(gaia::GaiaRequest&);
    int AuthorizeExclusive(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AuthorizeExclusive(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AuthorizeExclusive(gaia::GaiaRequest&);
    int CheckIfAccountExists(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CheckIfAccountExists(gaia::GaiaRequest&);
    int SendAuthentificate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SendAuthentificate(gaia::GaiaRequest&);
    int CreateAccount(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CreateAccount(gaia::GaiaRequest&);
    int ChangeCredentialPassword(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ChangeCredentialPassword(gaia::GaiaRequest&);
    int ChangeCredentialContactDetails(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ChangeCredentialContactDetails(gaia::GaiaRequest&);
    int SetDeviceInfo(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SetDeviceInfo(gaia::GaiaRequest&);
    int Login(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int Login(gaia::GaiaRequest&);
    bool IsLoggedIn(gaia::BaseServiceManager::Credentials);
    void Logout(gaia::BaseServiceManager::Credentials);
    int RetrievePassword(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrievePassword(gaia::GaiaRequest&);
    int AddCredential(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddCredential(gaia::GaiaRequest&);
    int RemoveCredential(gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RemoveCredential(gaia::GaiaRequest&);
    int AddPermission(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddPermission(gaia::GaiaRequest&);
    int EncryptToken(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int EncryptToken(gaia::GaiaRequest&);
    int RefreshAccessToken(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RefreshAccessToken(gaia::GaiaRequest&);
    int FindUserByAlias(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int AddAlias(gaia::BaseServiceManager::Credentials, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
