// libs/gaia/include/utils/servicerequest.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_SERVICEREQUEST_H
#define LIBS_GAIA_INCLUDE_UTILS_SERVICEREQUEST_H

namespace gaia {
struct ServiceRequest {  // sizeof 0xCC
    void Grab();
    void Drop();
protected:
    ServiceRequest(const gaia::ServiceRequest&);
public:
    ServiceRequest(gaia::GaiaRequest*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetStringMethodType();
    bool IsCanceled();
    __int64 GetID();
protected:
    gaia::ServiceRequest &operator=(const gaia::ServiceRequest&);
    ~ServiceRequest();
public:
    gaia::State status;  // 0x0
    gaia::Condition condition;  // 0x4
    bool aSync;  // 0x8
    gaia::HTTPMethodType m_methodType;  // 0xC
    void (*callback)(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);  // 0x10
    void* caller;  // 0x14
    int responseCode;  // 0x18
    gaia::OpCodes opCode;  // 0x1C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > url;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > payload;  // 0x40
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > response;  // 0x60
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > headerMap;  // 0x80
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > requestedHeaderMap;  // 0x9C
    gaia::ConnectionSettings cs;  // 0xB8
    bool requestWasSent;  // 0xBA
    gaia::GaiaRequest *gaiaReq;  // 0xBC
    int nr_RefCount;  // 0xC0
    glwebtools::Mutex m_mutex;  // 0xC4
};
}

#endif
