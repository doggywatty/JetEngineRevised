// libs/gaia/include/gaiaimplementation/gaia_seshat.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_SESHAT_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_SESHAT_H

namespace gaia {
class Gaia_Seshat {  // sizeof 0x8
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetSeshatStatus();
    int StartAndAuthorizeSeshat(gaia::GaiaRequest&);
    int StartAndAuthorizeSeshat(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int PutDataCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int PutDataCheckEtag(gaia::GaiaRequest&);
    int PutDataCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*, int, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int PutData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int PutData(gaia::GaiaRequest&);
    int PutData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const void*, int, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetDataCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetDataCheckEtag(gaia::GaiaRequest&);
    int GetEtagForKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetEtagForKey(gaia::GaiaRequest&);
    int GetData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, gaia::BaseServiceManager::Credentials, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetData(gaia::GaiaRequest&);
    int DeleteData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteData(gaia::GaiaRequest&);
    int SetProfile(gaia::BaseServiceManager::Credentials, const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SetProfile(gaia::GaiaRequest&);
    int GetProfile(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetProfile(gaia::GaiaRequest&);
    gaia::UserProfile *GetStandardUserProfile();
    int ReleaseStandardUserProfile();
    int DeleteProfile(gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteProfile(gaia::GaiaRequest&);
    int SetProfileVisibility(gaia::BaseServiceManager::Credentials, const Json::Value&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SetProfileVisibility(gaia::GaiaRequest&);
    int GetProfileVisibility(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetProfileVisibility(gaia::GaiaRequest&);
    int GetBatchProfiles(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetBatchProfiles(gaia::GaiaRequest&);
    int CreateMatcher(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CreateMatcher(gaia::GaiaRequest&);
    int GetMatches(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetMatches(gaia::GaiaRequest&);
    int DeleteMatcher(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteMatcher(gaia::GaiaRequest&);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
