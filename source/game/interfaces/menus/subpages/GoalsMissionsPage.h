// source/game/interfaces/menus/subpages/GoalsMissionsPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSMISSIONSPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSMISSIONSPAGE_H

#include <vector>
#include "game/interfaces/interfaceobjects/BasicPage.h"

class MissionsBox;
class MissionsMultiplierBox;
class InterfaceArea;

// The missions tab of the goals ("Menu_Missions" layer, 0x1d4 bytes in the .so): the active missions one under the other,
// each one a MissionsBox, and under them the bar of the score multiplier. A mission that is done and has gone out gives its
// points to the bar and its slot a new mission that comes in.
class GoalsMissionsPage : public BasicPage {
public:
    GoalsMissionsPage();                                                   // @0x4a0110
    virtual ~GoalsMissionsPage();                                          // @0x4a02c8
    virtual void Update(int);                                              // @0x4a0360
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);  // @0x4a079c
    void Refresh();                                                        // @0x4a0574
private:
    void _ClearMissionBoxes();                                             // @0x4a07e8
    math::vec2<float> GetOutroPos();
    void InitMissionMultiplier();                                          // @0x4a0830
    MissionsMultiplierBox *m_missionsMultiplierBox;  // 0x1bc
    InterfaceArea *m_missionsParentObj;              // 0x1c0
    std::vector<MissionsBox*> m_missionsBox;         // 0x1c4
};

#endif
