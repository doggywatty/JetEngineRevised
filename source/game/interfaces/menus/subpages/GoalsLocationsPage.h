// source/game/interfaces/menus/subpages/GoalsLocationsPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSLOCATIONSPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_GOALSLOCATIONSPAGE_H

#include <vector>
#include "game/interfaces/interfaceobjects/BasicPage.h"

class InterfaceGrid;
class LocationBox;
class MissionsMultiplierBox;

// The locations tab of the Goals menu: a grid with a box for each location that is not hidden, the least multiplier first, and
// the box of the multiplier of the missions over it
class GoalsLocationsPage : public BasicPage {  // sizeof 0x1D0
public:
    GoalsLocationsPage();  // @0x49f8bc
    virtual ~GoalsLocationsPage();
    void Refresh();  // @0x49fb84
    void OnEnter() { PopulateLocationBoxData(); }  // @0x49fdf4
    bool ShouldResetChildren() const;  // @0x49ffe8
private:
    void ClearLocationBoxData();  // @0x49fdf8
    void PopulateLocationBoxData();  // @0x49fe48
    void InitMissionMultiplier();  // @0x49fff4
    MissionsMultiplierBox *m_missionsMultiplierBox;  // 0x1BC
    std::vector<LocationBox*> m_locationBoxes;  // 0x1C0
    InterfaceGrid *m_grid;  // 0x1CC
};

#endif
