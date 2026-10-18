// libs/social_framework/include/social/userdata/messagein.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEIN_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEIN_H

namespace social {
class MessageIn : public social::Message {  // sizeof 0x60
public:
    MessageIn(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, long, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, int, const char*, int, bool);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetId();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetUserId();
    long GetDate();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_id;  // 0x1C
    long m_date;  // 0x3C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_fromUid;  // 0x40
};
}

#endif
