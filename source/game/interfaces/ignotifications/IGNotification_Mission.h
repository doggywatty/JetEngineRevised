// source/game/interfaces/ignotifications/IGNotification_Mission.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_MISSION_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_IGNOTIFICATION_MISSION_H

class IGNotification_Mission : public BaseIGNotification {  // sizeof 0x1D8
public:
    IGNotification_Mission(Mission*);
    virtual ~IGNotification_Mission();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
private:
    void InitMissionGiverPBar();
    Mission *m_mission;  // 0x1D4
};

namespace gui {
struct IGNotification_Mission {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Notification_BG();
    static jet::String &_BoxSize();
    static jet::String &_missions_NefarioPBar();
    static jet::String &_missions_GruPBar();
    static jet::String &_missions_GirlsPBar();
    static jet::String &_mission_txt();
    static jet::String &_mission_finished();
};
}

#endif
