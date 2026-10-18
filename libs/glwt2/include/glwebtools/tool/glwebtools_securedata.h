// libs/glwt2/include/glwebtools/tool/glwebtools_securedata.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_SECUREDATA_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_SECUREDATA_H

namespace glwebtools {
struct SecureBuffer {  // sizeof 0x48
    SecureBuffer(const glwebtools::SecureBuffer&);
    SecureBuffer(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, unsigned);
    SecureBuffer(const unsigned char*, unsigned);
    SecureBuffer(const std::vector<unsigned char,std::allocator<unsigned char> >&);
    SecureBuffer();
    glwebtools::SecureBuffer &operator=(const glwebtools::SecureBuffer&);
    ~SecureBuffer();
    std::vector<unsigned char,std::allocator<unsigned char> > Get();
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, unsigned);
    void Set(const unsigned char*, unsigned);
    void Set(const std::vector<unsigned char,std::allocator<unsigned char> >&);
    std::vector<unsigned char,std::allocator<unsigned char> > operator class std::vector<unsigned char,class std::allocator<unsigned char> >();
    bool IsValid();
    int read(const glwebtools::JsonReader&);
    int write(glwebtools::JsonWriter&);
    void clear();
    unsigned size();
    bool empty();
private:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > encrypt(const unsigned char*, unsigned, &);
    static std::vector<unsigned char,std::allocator<unsigned char> > decrypt(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, &);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > hash(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned m_seeds[2];  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_data;  // 0x8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_hash;  // 0x28
};
}

namespace glwebtools {
struct SecureString {  // sizeof 0x48
    SecureString(const glwebtools::SecureString&);
    SecureString(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, unsigned);
    SecureString(const char*, unsigned);
    SecureString(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    SecureString();
    glwebtools::SecureString &operator=(const glwebtools::SecureString&);
    ~SecureString();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > Get();
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, unsigned);
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Set(const char*, unsigned);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > operator class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >();
    bool IsValid();
    int read(const glwebtools::JsonReader&);
    int write(glwebtools::JsonWriter&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > PrintDebug();
    void clear();
    unsigned size();
    bool empty();
private:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > encrypt(const char*, unsigned, &);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > decrypt(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, &);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > hash(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned m_seeds[2];  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_data;  // 0x8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_hash;  // 0x28
};
}

#endif
