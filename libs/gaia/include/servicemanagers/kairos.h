// libs/gaia/include/servicemanagers/kairos.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_KAIROS_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_KAIROS_H

namespace gaia {
class Kairos : public gaia::BaseServiceManager {  // sizeof 0x130
public:
    Kairos(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Kairos();
    int GetAlerts(void (gaia::State, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*)*, void*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    void TriggerCallback(gaia::State, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void ProcessServerSideEvent(glwebtools::UrlConnection&);
    virtual bool NeedsLinkRefreshing();
private:
    void (*m_callback)(gaia::State, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*);  // 0x128
    void* m_UserData;  // 0x12C
};
}

#endif
