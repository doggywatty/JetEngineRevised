// libs/social_framework/include/social/userdata/gift.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_GIFT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_GIFT_H

namespace social {
class Gift {  // sizeof 0x44
public:
    Gift(const Json::Value&);
    Gift();
    virtual ~Gift();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetName();
    void SetName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetValue();
    void SetValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Parse(const Json::Value&);
    void UnParse(Json::Value&);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_name;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_value;  // 0x24
};
}

#endif
