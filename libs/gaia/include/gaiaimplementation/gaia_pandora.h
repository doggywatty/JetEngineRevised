// libs/gaia/include/gaiaimplementation/gaia_pandora.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_PANDORA_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_PANDORA_H

namespace gaia {
class Gaia_Pandora {  // sizeof 0x8
public:
    int GetServiceUrl(const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetServiceUrl(gaia::GaiaRequest&);
    int GetServiceUrl(gaia::BaseServiceManager::Credentials, const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int getServerTimeStamp(long&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
