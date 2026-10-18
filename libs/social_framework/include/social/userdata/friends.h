// libs/social_framework/include/social/userdata/friends.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDS_H

namespace social {
class Friends : public social::Loadable {  // sizeof 0x11C
public:
    Friends(const social::User*);
    virtual ~Friends();
    const social::UserSet &GetFriends();
    const social::UserSet &GetPlayingFriends();
    const social::UserSet &GetNonPlayingFriends();
    social::User *operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void UnLoadImpl();
protected:
    social::UserSet m_friendsVector;  // 0x8C
    social::UserSet m_playingFriendsVector;  // 0xBC
    social::UserSet m_nonPlayingFriendsVector;  // 0xEC
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
};
}

#endif
