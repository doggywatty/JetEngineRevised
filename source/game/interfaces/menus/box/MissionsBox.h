// source/game/interfaces/menus/box/MissionsBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_MISSIONSBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_MISSIONSBOX_H

#include "core/delayer.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"

class Mission;
class InterfaceButton;
class InterfacePBar;

// One mission of the goals ("MissionsBox" layer, 0x238 bytes in the .so): its text with the number of its slot, the bar of
// its giver that fills with what the conditions have done, and the "finished" mark. A new mission comes in from the side;
// a finished one stays for two seconds and goes out, and the page then takes its points and gives the slot a new mission.
// The button to skip a mission for tokens is there but hidden (the .so hides it in the constructor and nothing shows it).
// Not ported: the tracking of the skip (GameTrackingMgr::SendCurrencySpentEvent), the sounds, and the GUI object that
// explains the mission (Mission::DisplayGuiAssociated).
class MissionsBox : public BasicPage {
public:
    // what the box is doing
    enum EStateDef { State_Done = 0, State_WaitIntro = 1, State_Intro = 2, State_Shown = 3, State_Finished = 4, State_Outro = 5 };
    MissionsBox(const jet::String &missionName, bool isNew, int slot);  // @0x4d54a4
    virtual ~MissionsBox();
    virtual void Update(int);                                           // @0x4d5a94
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);  // @0x4d5ca8
    virtual void SetOffset(const math::vec2<float>&);                   // @0x4d5ce0
    void SetOutroPos(const math::vec2<float>&);                         // @0x4d5d58
    bool HasIntroOrOutroAnim() const;                                   // @0x4d5d98
    void TryStartIntroOutroAnim();                                      // @0x4d5dbc
    bool AnyDelayersOn() const;                                         // @0x4d5f18
    void TryForcingConditionUpdate();                                   // @0x4d62d4
    bool IsCompleted() const { return m_completed; }
    int GetMissionPoints() const { return m_missionPoints; }
    int GetState() const { return m_state; }
    void EnableExitScreenAnim(bool b) { m_enableExitScreenAnim = b; }
    static math::vec2<float> k_introOffset, k_outroOffset;
private:
    void InitMissionGiverPBar();                                        // @0x4d5f40
    void BuySkipMission(int, int);                                      // @0x4d60c0
    jet::String m_missionName;                                          // 0x1bc
    bool m_isNew;                                                       // 0x1c0
    bool m_completed;                                                   // 0x1c1
    int m_missionPoints;                                                // 0x1c4
    bool m_enableExitScreenAnim;                                        // 0x1c8
    InterfaceButton *m_completeButton;                                  // 0x1cc
    int m_state;                                                        // 0x1d0
    jet::core::Delayer<math::vec2<float>, jet::core::SmoothStep> m_introDelayer;  // 0x1d4
    jet::core::Delayer<math::vec2<float>, jet::core::SmoothStep> m_outroDelayer;  // 0x1f8
    int m_completedTimer;                                               // 0x21c
    int m_slot;                                                         // 0x220
    math::vec2<float> m_boxOriginalOffset;                              // 0x224
    float m_outroDistance;                                              // 0x22c: how far the outro has to go
    Mission *m_mission;                                                 // 0x230
    InterfacePBar *m_missionGiverPBar;                                  // 0x234
};

#endif
