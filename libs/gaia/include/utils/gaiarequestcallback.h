// libs/gaia/include/utils/gaiarequestcallback.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_GAIAREQUESTCALLBACK_H
#define LIBS_GAIA_INCLUDE_UTILS_GAIAREQUESTCALLBACK_H

namespace gaia {
class GaiaRequestCallback {  // sizeof 0x38
public:
    GaiaRequestCallback(gaia::GaiaRequest*);
    GaiaRequestCallback(void*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, gaia::OpCodes, void*);
protected:
    GaiaRequestCallback(const gaia::GaiaRequestCallback&);
    gaia::GaiaRequestCallback &operator=(const gaia::GaiaRequestCallback&);
public:
    void TriggerCallback();
    ~GaiaRequestCallback();
protected:
    gaia::GaiaRequestCallback::eCallbackTyle m_type;  // 0x0
    void* m_RequestCallback;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_RequestAnswer;  // 0x8
    int m_RequestError;  // 0x28
    gaia::OpCodes m_RequestCode;  // 0x2C
    void* m_RequestCaller;  // 0x30
    gaia::GaiaRequest *m_newRequest;  // 0x34
};
}

#endif
