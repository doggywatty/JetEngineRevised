// libs/social_framework/include/social/userdata/messagesecure.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGESECURE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGESECURE_H

namespace social {
class MessageSecure : public social::MessageIn {  // sizeof 0x90
public:
    MessageSecure(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, long, const Json::Value&);
    const Json::Value &GetJsonData();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetType();
private:
    Json::Value m_json;  // 0x60
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_type;  // 0x70
};
}

#endif
