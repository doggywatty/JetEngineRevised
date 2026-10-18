// source/game/interfaces/ignotifications/IGNotification_SocialChallenge.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_SOCIALCHALLENGE_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_SOCIALCHALLENGE_H

class IGNotification_SocialChallenge : public BaseIGNotification {  // sizeof 0x1DC
public:
    IGNotification_SocialChallenge(int);
    virtual ~IGNotification_SocialChallenge();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
    static const int k_showTimeStart;
    static const int k_showTimeCompleted;
private:
    InterfaceObject *m_photoArea;  // 0x1D4
    Interface3DCostumePos *m_i3DCostume;  // 0x1D8
};

namespace gui {
struct IGNotification_SocialChallenge {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Notification_BG();
    static jet::String &_Photo_friend_area();
    static jet::String &_title_text();
    static jet::String &_description_text();
    static jet::String &_reward_parent();
    static jet::String &_reward_tokensIcon();
    static jet::String &_reward_bananasIcon();
    static jet::String &_reward_text();
    static jet::String &_Icon_finished();
    static jet::String &_InterfaceSize();
    static jet::String &_Taunt2DPos();
};
}

#endif
