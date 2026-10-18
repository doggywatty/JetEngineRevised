// libs/social_framework/include/social/usersns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERSNS_H

namespace social {
class UserSNS : public social::User {  // sizeof 0x6DC
    UserSNS(sociallib::ClientSNSEnum);
public:
    UserSNS(sociallib::ClientSNSEnum, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~UserSNS();
    virtual void Login(bool);
    virtual bool LogoutImpl();
    sociallib::ClientSNSEnum GetSNS();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetSecret();
    virtual social::Profile *GetProfile();
    virtual social::Friends *GetFriends();
    virtual social::Avatar *GetAvatar();
    virtual social::Wall *GetWall();
    virtual social::Inbox *GetInbox();
    virtual social::GameProfile *GetGameProfile();
    virtual social::Achievements *GetAchievements();
    virtual social::Activity *GetActivity();
    virtual social::StandardProfile *GetStandardProfile();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAppName();
    virtual void LoginCancel();
    social::ScoreSNS *GetScore();
    void ForceUid(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ForceSecret(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    static bool sOnLoggedIn(void*, bool, sociallib::ClientSNSEnum);
    static bool sOnUidLoaded(void*, bool, sociallib::ClientSNSEnum);
    static bool sOnLoggedOut(void*, bool, sociallib::ClientSNSEnum);
    static bool sOnCredentialRemoved(void*, bool, social::OnlineEventData);
protected:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_secret;  // 0x70
    sociallib::ClientSNSEnum m_sns;  // 0x90
private:
    social::ProfileSNS m_profile;  // 0x94
    social::FriendsSNS m_friends;  // 0x164
    social::AvatarSNS m_avatar;  // 0x284
    social::WallSNS m_wall;  // 0x330
    social::AchievementsSNS m_achievements;  // 0x414
    social::InboxSNS m_inbox;  // 0x4F0
    social::Activity *m_activity;  // 0x5F4
    social::ScoreSNS m_score;  // 0x5F8
};
}

#endif
