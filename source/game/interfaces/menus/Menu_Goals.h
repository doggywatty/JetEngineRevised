// source/game/interfaces/menus/Menu_Goals.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_GOALS_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_GOALS_H

#include <vector>
#include "Menu_Base.h"

// set by the back key of the Goals and the options menus (@0xd0df28, defined with Menu_Options)
extern bool g_bBackToMenuOption;

class InterfaceScrollBar;
class BananasAndTokensBox;
class RedeemCodePage;
class GoalsMissionsPage;
class GoalsAchievementsPage;
class GoalsLocationsPage;
class GoalsBappleObjectivesPage;

// The goals ("Goals_Common" layer, 0x180 bytes in the .so): a row of tabs over a list that holds the page of the tab (the
// missions, the achievements, the locations; the leaderboard's and the bapples' are not reachable here, see below) and a
// back button; the counters of bananas and tokens are on top.
// Not ported: the sounds (SoundMgr), the tracking of the screens (GameTrackingMgr, TrackingScreenMgr), the popups
// (PopupMgr: the first tutorial's information, the blind box's prize), the key events (KeyEventManager) and the pages that
// are not reachable in this build: the one of the leaderboard tab (RedeemCodePage, whose button is out of the screen) and
// the bapples' (GoalsBappleObjectivesPage: the bapple hunt is never started, and its button is hidden by Init).
class Menu_Goals : public Menu_Base {
public:
    // the tab shown; the order of the tab buttons
    enum EStateDef { Goals_Missions = 0, Goals_Achievements = 1, Goals_HallOfFame = 2, Goals_Locations = 3, Goals_Bapples = 4, Goals_None = 5 };
    Menu_Goals();                                                    // @0x4e77dc
    virtual ~Menu_Goals();                                           // @0x4e78f4
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    virtual void Init();                                             // @0x4e7a48
    virtual void Update(int);                                        // @0x4e8358
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);  // @0x4e86b4
    virtual void Refresh();                                          // @0x4e86b8
    virtual void OnEnter();                                          // @0x4e8320
    virtual void OnQuit() {}                                         // @0x4e8354
    virtual bool RenderGameInBG() { return false; }                  // @0x4de1c4
    virtual int GetCurrentTrackingScreen() const { return GetTrackingState(m_state); }   // @0x4e8bc8
    virtual int GetPreviousTrackingScreen() const { return GetTrackingState(m_previousState); }  // @0x4e8bd0
    bool AnyTabWithNotification();                                   // @0x4e8bfc
    // the tab the menu opens at
    void SetEnterState(int tab) { m_targetState = tab; }             // @0x4e8904
    // true while a prize of the blind box is being opened with the button of the bapples' tab
    static bool checkSoundClickOpen;                                 // @0xd0d050
    static const jet::String k_menuName;

private:
    void SetState(int tab, bool keepPrevious);                       // @0x4e890c
    static int GetTrackingState(int tab);                            // @0x4e8bd8
    void UpdateShowAchievementNotify(bool force);                    // @0x4e8a90

    std::vector<InterfaceButton*> m_tabButtons;   // 0x124
    InterfaceButton *m_backButton;                // 0x130
    InterfaceList *m_subpagesParent;              // 0x134
    InterfaceScrollBar *m_subpagesScroll;         // 0x138
    InterfaceText *m_textCategory;                // 0x13c
    BananasAndTokensBox *m_bananasAndTokensBox;   // 0x140
    std::vector<jet::String> m_tabHeaderStrings;  // 0x144
    int m_targetState;                            // 0x150: the tab the menu opens at
    int m_state;                                  // 0x154: the tab shown
    int m_previousState;                          // 0x158
    RedeemCodePage *m_redeemCodePage;             // 0x15c: never made
    GoalsMissionsPage *m_missionsPage;            // 0x160
    GoalsAchievementsPage *m_achievementsPage;    // 0x164
    GoalsLocationsPage *m_locationsPage;          // 0x168
    GoalsBappleObjectivesPage *m_bapplesPage;     // 0x16c: never made
    InterfaceButton *m_openButton;                // 0x170
    InterfaceText *m_itemNumber;                  // 0x174
    bool m_showAchievementNotify;                 // 0x178
    InterfaceButton *m_gsAchievementButton;       // 0x17c
};

#endif
