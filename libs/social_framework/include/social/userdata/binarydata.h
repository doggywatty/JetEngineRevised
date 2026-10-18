// libs/social_framework/include/social/userdata/binarydata.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_BINARYDATA_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_BINARYDATA_H

namespace social {
class BinaryData : public social::Storable {  // sizeof 0xE8
public:
    BinaryData(const social::User*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~BinaryData();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetKey();
    virtual void SetData(const char*, int, social::BinaryData::Visibility) = 0;
    const char *GetData();
    int GetDataSize();
    virtual void Load(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&) = 0;
protected:
    char* m_buffer;  // 0xBC
    int m_bufferSize;  // 0xC0
    social::BinaryData::Visibility m_visibility;  // 0xC4
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0xC8
};
}

#endif
