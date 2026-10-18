// libs/glsociallib/include/snsuserdata.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_SNSUSERDATA_H
#define LIBS_GLSOCIALLIB_INCLUDE_SNSUSERDATA_H

namespace sociallib {
class SNSUserData {  // sizeof 0x34
    void EnsureParamExistence();
public:
    SNSUserData(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&, const sociallib::CDynamicMemoryStream&);
    SNSUserData(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    SNSUserData();
    void SetIfUnavailable(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool HasParam(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetTextMapData(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    void SetImageData(sociallib::CDynamicMemoryStream&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetParamValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetImageUrl();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetFullName();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetUserId();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFirstName();
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_userName;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_userId;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_userPicture;
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_textMapData;  // 0x0
    bool m_bHasImageData;  // 0x1C
    sociallib::CDynamicMemoryStream m_imageData;  // 0x20
};
}

#endif
