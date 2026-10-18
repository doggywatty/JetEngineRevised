// libs/gaia/include/gaiaimplementation/gaia_kairos.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_KAIROS_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_KAIROS_H

namespace gaia {
class Gaia_Kairos {  // sizeof 0x10
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetKairosStatus(gaia::BaseServiceManager::Credentials);
    int StartAndAuthorizeKairos(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int StartAndAuthorizeKairos(gaia::GaiaRequest&);
    int GetAlerts(gaia::GaiaRequest&);
    int GetAlerts(void (gaia::State, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*)*, void*, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int StopService();
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
    glwebtools::Mutex m_runningMutex;  // 0x8
};
}

#endif
