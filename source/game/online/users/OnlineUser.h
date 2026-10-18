// source/game/online/users/OnlineUser.h
#ifndef SOURCE_GAME_ONLINE_USERS_ONLINEUSER_H
#define SOURCE_GAME_ONLINE_USERS_ONLINEUSER_H

class OnlineUser {  // sizeof 0x28
protected:
    OnlineUser();
    ~OnlineUser();
public:
    void SetSocialFrameworkUser(social::User*);
    void LoadProfile();
    void LoadAvatar();
    bool IsOnline();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetUid();
    const jet::String &GetName();
    safe_enum<OnlineUser::EStatusDef,enum OnlineUser::EStatusDef::type> GetStatus();
    void SetName(const char*);
    void Refresh();
    void ReloadAvatar();
    ImageSpritePlayer *GetAvatar();
    void CancelAvatarLoading();
    void SetChallenged(bool);
    bool IsChallenged();
    void Update(int);
    void RenderAvatar(const jet::scene::Camera&, jet::video::Painter&, const math::vec2<float>&, float);
    void RenderAvatar(const jet::scene::Camera&, jet::video::Painter&, const math::vec2<float>&, const jet::String&);
private:
    void InitProfileData();
    void InitAvatarData();
    void CancelLoads();
    static bool OnProfileLoaded(void*, bool, social::OnlineEventData);
    static bool OnAvatarLoaded(void*, bool, social::OnlineEventData);
public:
    static const jet::String k_normalPhotoArea;
    static const jet::String k_hudPhotoArea;
    static const jet::String k_smallPhotoArea;
private:
    bool m_online;  // 0x0
    jet::String m_name;  // 0x4
    social::User *m_user;  // 0x8
    ImageSpritePlayer *m_avatarSprite;  // 0xC
    safe_enum<OnlineUser::EStatusDef,enum OnlineUser::EStatusDef::type> m_status;  // 0x10
    safe_enum<OnlineUser::EStatusDef,enum OnlineUser::EStatusDef::type> m_oldStatus;  // 0x14
    bool m_challenged;  // 0x18
    BasicPage *m_basicPage;  // 0x1C
    InterfaceObject *m_loadingIcon;  // 0x20
    InterfaceObject *m_defaultAvatarIcon;  // 0x24
};

#endif
