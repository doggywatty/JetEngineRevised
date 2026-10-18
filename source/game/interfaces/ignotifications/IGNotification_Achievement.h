// source/game/interfaces/ignotifications/IGNotification_Achievement.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_ACHIEVEMENT_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_ACHIEVEMENT_H

class IGNotification_Achievement : public BaseIGNotification {  // sizeof 0x1D8
public:
    IGNotification_Achievement(Achievement*);
    virtual ~IGNotification_Achievement();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
private:
    Achievement *m_achievement;  // 0x1D4
};

namespace gui {
struct IGNotification_Achievement {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Notification_BG();
    static jet::String &_BoxSize();
    static jet::String &_title_text();
    static jet::String &_description_text();
    static jet::String &_Achievement_Icon();
    static jet::String &_Icon_finished();
};
}

#endif
