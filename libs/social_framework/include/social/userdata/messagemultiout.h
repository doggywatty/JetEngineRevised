// libs/social_framework/include/social/userdata/messagemultiout.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEMULTIOUT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEMULTIOUT_H

namespace social {
class MessageMultiOut : public social::MessageOut {  // sizeof 0x68
public:
    static social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> Create(std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, const char*, int, const char*, int, bool);
    const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetToList();
protected:
    MessageMultiOut(std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, const char*, int, const char*, int, bool);
private:
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_toList;  // 0x54
};
}

#endif
