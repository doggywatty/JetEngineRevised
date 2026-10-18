// source/game/interfaces/menus/Menu_MainMenu.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_MAINMENU_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_MAINMENU_H

#include <vector>
#include "Menu_BaseCostumes.h"
#include "core/delayer.h"

class InterfaceGraph;
class InterfaceText;
class InterfacePBar;
class InterfaceList;
class InterfaceButton;
class BananasAndTokensBox;

// The first menu: the lab with the tap-to-play minion, the bananas / tokens counters, the buttons to the shop,
// goals, options and friends, the daily login calendar and the challenge list.
// Not ported (the systems are not in this build): the sounds, COPPA, the cheat detection, the location download popup, the
// bapple hunt, the online services (friends, the IGP reward, the facebook popup), the weekly contests' marks,
// the tracking and the missions' refresh.
class Menu_MainMenu : public Menu_BaseCostumes {
public:
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    Menu_MainMenu();
    virtual ~Menu_MainMenu();
    virtual bool UpdateGameInBG() { return true; }
    virtual bool RenderGameInBG() { return true; }
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Refresh();
    virtual int GetCurrentTrackingScreen() const { return 0x1bca4; }  // @0x488d04
    virtual void BasicPage_Reset() { Menu_Base::BasicPage_Reset(); }
    void Reset();
    void ExecuteTouchInteractEvent(int);
    void TouchInteract_StartingOutroAnim(int);
    void TouchInteract_Reset3dEventInterface(int);
    static void StartMainMenuIntroMovie();
    // the play was tapped and the costumes are away: the state starts the game
    bool IsStartPlay() const { return m_startPlay; }
    void EnableTouches(bool);
    void StopMainMenuSfx();
    bool isMainMenuScrollListShowing() const { return m_scrollListShowing; }
    void ShowMainMenuScrollList(bool);
    bool IsEnteringToSocial() const { return m_enterToSocial; }
    static const jet::String k_menuName;

private:
    InterfaceObject *Find(const char *name) { return GetUIObject(jet::String(name)); }
    void SetupDailyLogin();
    void HideAll(const std::vector<InterfaceGraph*>&);
    void GetDailyAward();
    void StartEnteringGamePlay();
    void UpdateChallengeScrollVisibility();
    void _ClearChallengeData();
    void RefreshMarks();
    void RefreshCostumePromo();
    void RefreshCostumePromo_dailylogin();
    void RefreshCostumePromo_magician();
    void ShowPlusButton() {}
    void HidePlusButton() {}
    void UpdateAwardsTimers();
    void UpdateEnteringGamePlay(int dt);
    void UpdateReleasedButtons(int dt);
    static void OnPopupYes(void*);
    static void OnPopupNo(void*);
    void OnLocationDownloadContentCancelCallback();

    // 0x1f0 / 0x1f4
    InterfaceList *m_mainMenuScrollList, *m_challengeScrollList;
    bool m_touchesEnabled;                      // 0x22c
    InterfaceButton *m_shopButton, *m_goalsButton, *m_optionsButton, *m_cheatsButton, *m_glGamesButton, *m_cmccExitButton, *m_moreGameButton;
    InterfaceButton *m_cancelButton, *m_locationsButton, *m_friendsButton, *m_menuButton, *m_arrowR, *m_arrowL, *m_multiplierButton, *m_bapplesButton;
    InterfaceGraph *m_shopNotify, *m_friendsNotify, *m_multiplierBonus, *m_smallCostumeBG;
    InterfaceText *m_multiplierBonusValue;
    BananasAndTokensBox *m_counters = 0;           // 0x2a4
    InterfaceObject *m_menuBG, *m_waitingBg;
    math::vec2<float> m_menuBGOffset;
    bool m_scrollListShowing;                   // 0x298
    bool m_enterToSocial;
    bool m_waiting;                             // 0x2a0: the friends button waits for the online services (the cancel button shows)
    bool m_refreshRequested;                    // 0x2b8: the menu refreshes itself in the next update
    int m_equippedIndex;                        // 0x2ac: the costume the play starts with, in the big ones
    bool m_enteringPlay, m_outroStarted, m_startPlay;  // 0x1fc 0x1fd 0x1fe
    jet::core::Delayer<float, jet::core::SmoothStep> m_scrollDelayer;  // 0x280
    int m_scrollShowDelay, m_scrollHideDelay;   // 0x278 0x27c
    // daily login calendar
    int m_dailyDays, m_dailyDayFirst, m_dailyDayInWeek;  // 0x2c8 0x2cc 0x2d0
    int m_dailyCurTime;                          // 0x2d4
    bool m_dailyDone;                            // 0x330
    InterfaceGraph *m_dailyFrame, *m_glass;
    InterfaceButton *m_getAward;
    std::vector<InterfaceGraph*> m_finish, m_banana, m_xWhite, m_xGreen, m_x950White, m_x950Green, m_token;
    std::vector<InterfaceText*> m_dayText;
    std::vector<InterfacePBar*> m_dayBar;
    int m_dailyFrameLimit;                       // 0x3ac: the frames the award's bar takes to fill
    int m_dailyFrameTimer;                       // 0x3b0
    bool m_dailyAnimating = false;               // 0x3b4
};

#endif
