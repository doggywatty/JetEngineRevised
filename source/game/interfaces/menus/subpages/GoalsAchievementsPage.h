// source/game/interfaces/menus/subpages/GoalsAchievementsPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSACHIEVEMENTSPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSACHIEVEMENTSPAGE_H

#include <vector>
#include "game/interfaces/interfaceobjects/BasicPage.h"

class AchievementsBox;
class InterfaceGrid;

// The achievements tab of the goals (the achievements part of the "Menu_Missions" layer, 0x1d0 bytes in the .so): the
// achievements the player can see in a grid, with how many of them are done.
class GoalsAchievementsPage : public BasicPage {
public:
    GoalsAchievementsPage();                                               // @0x49e1d4
    virtual ~GoalsAchievementsPage();                                      // @0x49e414
    virtual void Update(int dt) { BasicPage::Update(dt); }                 // @0x49e494
    virtual void Render(const jet::scene::Camera &c, jet::video::Painter &p) { BasicPage::Render(c, p); }  // @0x49e498
    void Refresh();                                                        // @0x49e49c
    bool ShouldResetChildren() const;                                      // @0x49e9b8
private:
    void ClearAchievementArray();                                          // @0x49e70c
    void PopulateAchievementArray();                                       // @0x49e754
    std::vector<AchievementsBox*> m_achievementArray;  // 0x1bc
    InterfaceGrid *m_achievementsGrid;                 // 0x1cc
};

#endif
