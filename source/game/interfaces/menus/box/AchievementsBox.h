// source/game/interfaces/menus/box/AchievementsBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_ACHIEVEMENTSBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_ACHIEVEMENTSBOX_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

class Achievement;
class InterfaceButton;

// One achievement ("AchievementsTab" layer, 0x1c4 bytes in the .so): its name and how far it is (or the checked background
// when done), its text, the prize and, when it is done and the prize is not taken, the button to take it.
// Not ported: the sound, the tracking of the prize (GameTrackingMgr) and the points of the CRM (GameCrmMgr).
class AchievementsBox : public BasicPage {
public:
    explicit AchievementsBox(Achievement*);                                // @0x4cd884
    virtual ~AchievementsBox();                                            // @0x4cdd34
    virtual void Update(int);                                              // @0x4cdd84
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);  // @0x4cdec4
    void Refresh();                                                        // @0x4cdefc
private:
    Achievement *m_achievement;  // 0x1bc
    InterfaceButton *m_claimButton;  // 0x1c0
};

#endif
