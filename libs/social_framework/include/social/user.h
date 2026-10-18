// libs/social_framework/include/social/user.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USER_H

namespace social {
class User : public social::SimpleEventDispatcher<social::OnlineEventData> {  // sizeof 0x70
public:
    virtual ~User();
    social::OnlineStatus<enum social::User::LoginState> GetLoginState();
    bool IsLoggedIn();
    virtual void Login(bool) = 0;
    void Logout();
    virtual bool LogoutImpl() = 0;
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetUid();
    bool IsSNSUser();
    virtual social::Profile *GetProfile() = 0;
    virtual social::Friends *GetFriends() = 0;
    virtual social::Avatar *GetAvatar() = 0;
    virtual social::Wall *GetWall() = 0;
    virtual social::Inbox *GetInbox() = 0;
    virtual social::GameProfile *GetGameProfile() = 0;
    virtual social::Achievements *GetAchievements() = 0;
    virtual social::Activity *GetActivity() = 0;
    virtual social::StandardProfile *GetStandardProfile() = 0;
    bool IsPlayer();
    bool IsFriend();
    void SetIsFriend(bool);
    virtual void LoginCancel() = 0;
protected:
    User(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    User(bool);
    void SetIsPlayer(bool);
    void SetUid(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void FinishLogout(bool);
    social::OnlineStatus<enum social::User::LoginState> m_loginState;  // 0x20
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_uid;  // 0x4C
    bool m_isPlayer;  // 0x6C
    bool m_isFriend;  // 0x6D
    bool m_isSNSUser;  // 0x6E
};
}

#endif
