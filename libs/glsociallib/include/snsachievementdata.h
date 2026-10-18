// libs/glsociallib/include/snsachievementdata.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_SNSACHIEVEMENTDATA_H
#define LIBS_GLSOCIALLIB_INCLUDE_SNSACHIEVEMENTDATA_H

namespace sociallib {
struct SNSAchievementData {  // sizeof 0x1B0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_unlockedDescription;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_lockedDescription;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_pictureUrl;  // 0x40
    unsigned m_id;  // 0x60
    bool m_isEarned;  // 0x64
    bool m_isSecret;  // 0x65
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_name;  // 0x68
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_objectId;  // 0x88
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_userId;  // 0xA8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_userName;  // 0xC8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_createdTime;  // 0xE8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_appId;  // 0x108
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_appName;  // 0x128
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_achievementId;  // 0x148
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_achievementUrl;  // 0x168
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_achievementType;  // 0x188
    unsigned m_likesCount;  // 0x1A8
    unsigned m_commentsCount;  // 0x1AC
};
}

#endif
