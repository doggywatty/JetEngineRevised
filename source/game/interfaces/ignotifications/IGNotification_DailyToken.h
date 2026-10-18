// source/game/interfaces/ignotifications/IGNotification_DailyToken.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_DAILYTOKEN_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_DAILYTOKEN_H

class IGNotification_DailyToken : public BaseIGNotification {  // sizeof 0x1E0
public:
    IGNotification_DailyToken(int, int);
    virtual ~IGNotification_DailyToken();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
private:
    int m_collectedTokens;  // 0x1D4
    int m_tokensCount;  // 0x1D8
    InterfaceObject *m_dailyTokenObj;  // 0x1DC
};

namespace gui {
struct IGNotification_DailyToken {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Notification_BG();
    static jet::String &_BoxSize();
    static jet::String &_title_text();
    static jet::String &_description_text();
    static jet::String &_3dElemPos();
};
}

#endif
