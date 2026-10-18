// libs/social_framework/include/social/userset.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERSET_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERSET_H

namespace social {
class UserSet {  // sizeof 0x30
public:
    UserSet();
    ~UserSet();
    std::_Vector_const_iterator<social::User *,std::allocator<social::User *> > begin();
    std::_Vector_iterator<social::User *,std::allocator<social::User *> > begin();
    std::_Vector_const_iterator<social::User *,std::allocator<social::User *> > end();
    void clear();
    social::User *operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Insert(social::User*);
    unsigned count(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned size();
private:
    std::vector<social::User *,std::allocator<social::User *> > m_userList;  // 0x0
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::User *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::User *> > > m_userMap;  // 0x14
};
}

#endif
