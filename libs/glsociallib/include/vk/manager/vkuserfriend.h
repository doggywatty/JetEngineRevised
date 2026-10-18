// libs/glsociallib/include/vk/manager/vkuserfriend.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKUSERFRIEND_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKUSERFRIEND_H

namespace sociallib {
class VKUserFriend : public sociallib::VKWebComponent {  // sizeof 0x1C
public:
    VKUserFriend();
    virtual ~VKUserFriend();
    virtual void OnUpdateResponse(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual bool OnUpdateSuccess(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual bool OnUpdateFailure(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool SendGetUserFriends(const int, bool, const char*, const char*, const char*, const int, const int, const int, const char*);
    bool SendGetUserFriendsPlaying(bool);
    bool SendGetUserFriendsNotPlaying(bool);
    void ProcessFriendsList(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessFriendsData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SaveFriendsList(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    bool m_bGetAvatar;  // 0x4
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_vFriendsAll;  // 0x8
};
}

#endif
