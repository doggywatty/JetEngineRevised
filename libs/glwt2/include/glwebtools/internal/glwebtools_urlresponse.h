// libs/glwt2/include/glwebtools/internal/glwebtools_urlresponse.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_URLRESPONSE_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_URLRESPONSE_H

namespace glwebtools {
class UrlResponseCore {  // sizeof 0x24
public:
    ~UrlResponseCore();
    int GetData(const void*&, unsigned&);
    int GetDataAsString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    long GetResponseCode();
    const char *GetHeaderField(const char*);
private:
    UrlResponseCore();
    void SetResponseCode(long);
    bool AddHeaderField(const char*, const char*);
    bool AddData(const void*, unsigned);
    long m_responseCode;  // 0x0
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringLowerCaseCompare<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,glwebtools::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,4> > m_headerFields;  // 0x4
    glwebtools::MutableData *m_data;  // 0x20
};
}

#endif
