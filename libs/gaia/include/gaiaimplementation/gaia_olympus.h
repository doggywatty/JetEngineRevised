// libs/gaia/include/gaiaimplementation/gaia_olympus.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_OLYMPUS_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_OLYMPUS_H

namespace gaia {
class Gaia_Olympus {  // sizeof 0x8
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetOlympusStatus();
    int StartAndAuthorizeOlympus(gaia::GaiaRequest&);
    int StartAndAuthorizeOlympus(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int ClearLeaderboard(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ClearLeaderboard(gaia::GaiaRequest&);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
    int RetrieveLeaderboardAroundCurrentUser(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrieveLeaderboardAroundCurrentUser(gaia::GaiaRequest&);
    int RetrieveFriendLeaderboard(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, int, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrieveFriendLeaderboard(gaia::GaiaRequest&);
    int PostEntry(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int PostEntry(gaia::GaiaRequest&);
    int PostArbitraryEntry(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int PostArbitraryEntry(gaia::GaiaRequest&);
    int RetrieveLeaderboardAroundArbitraryEntry(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrieveLeaderboardAroundArbitraryEntry(gaia::GaiaRequest&);
    int RetriveTopOfLeaderboard(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, int, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetriveTopOfLeaderboard(gaia::GaiaRequest&);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
