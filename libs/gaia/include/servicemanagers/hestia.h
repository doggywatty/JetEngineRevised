// libs/gaia/include/servicemanagers/hestia.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_HESTIA_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_HESTIA_H

namespace gaia {
class Hestia : public gaia::BaseServiceManager {  // sizeof 0x148
public:
    Hestia(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Hestia();
    int GetClientConfig(void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetFeedEtag(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_etag;  // 0x128
};
}

#endif
