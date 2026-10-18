// libs/gaia/include/servicemanagers/pandora.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_PANDORA_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_PANDORA_H

namespace gaia {
class Pandora : public gaia::BaseServiceManager {  // sizeof 0x180
public:
    Pandora(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Pandora();
    int GetServiceUrl(const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetServiceUrl(gaia::BaseServiceManager::Credentials, const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetServerTimeStamp(long&, gaia::GaiaRequest*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetVersionUpdateStatus();
    virtual bool NeedsLinkRefreshing();
    static gaia::OpCodes GetOpCode(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
private:
    virtual void setServerDate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > getServerDate();
    long ConvertTimeStringToSec(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    virtual void setFinalServicesUrls(char*, gaia::ServiceRequest*);
    glwebtools::Mutex m_mutexServerDate;  // 0x128
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_ServerDate;  // 0x130
    Json::Value m_finalServicesUrls;  // 0x150
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_versionUpdateStatus;  // 0x160
};
}

#endif
