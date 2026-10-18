// source/game/entities/FriendScoreInteractiveObject.h
#ifndef SOURCE_GAME_ENTITIES_FRIENDSCOREINTERACTIVEOBJECT_H
#define SOURCE_GAME_ENTITIES_FRIENDSCOREINTERACTIVEOBJECT_H

class FriendScoreInteractiveObject : public AutoplaceInteractiveObjectTemplate {  // sizeof 0x328
public:
    FriendScoreInteractiveObject(const clara::Template*);
    virtual ~FriendScoreInteractiveObject();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    void UpdateAvatar();
    void SetFriendOnlineUser(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, int);
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    static jet::String k_tmplName;
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_onlineUserId;  // 0x300
    int m_friendScore;  // 0x320
    bool m_isUsingAvatar;  // 0x324
    bool m_wasPassFriendScoreAdded;  // 0x325
    static const jet::String k_facebookAvatarMaterialName;
};

#endif
