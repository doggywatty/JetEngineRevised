// source/game/interfaces/menus/Menu_Pause.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_PAUSE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_PAUSE_H

// See Menu_Shop.h for why these additions were needed (no #includes at all originally, and
// GetMenuName() missing `const` left the class abstract -- same two bugs, same fix). This header is
// also missing several real members entirely (Menu_Pause::SetState(safe_enum<EStateDef>, bool),
// Subpages_ClearPages/OnEnter/Update -- confirmed via `armdis.py -s Menu_Pause`), not just this enum;
// left unported here, this is a minimal render-only pass. See re/NOTES.md.
#include "Menu_Base.h"
#include "ustl.h"
#include "game/online/tracking/tracking.h"
class PauseMenuChallengeInfoBox;
class MissionsMultiplierBox;
class InterfaceArea;
class MissionsBox;
class BananasAndTokensBox;

class Menu_Pause : public Menu_Base {  // sizeof 0x148
public:
    // Not in the PDB header. Menu_Pause::UpdateExit() at 0x48d094 branches `cmp r3,#0`/`cmp r3,#1`
    // (PopState+PopMenu, i.e. Resume)/`cmp r3,#2` (ClearChallenge+PopMenu x2+Game::GoToMainMenu) on
    // m_exitOption -- confirms at least 3 values with clear, verified semantics for 2 of them.
    enum ExitOptions { Exit_None = 0, Exit_Resume = 1, Exit_MainMenu = 2 };
    virtual const jet::String &GetMenuName() const;
    Menu_Pause();
    virtual ~Menu_Pause();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    void Exit(Menu_Pause::ExitOptions);
    void UpdateExit();
    void InitChallengeBox();
    void DeleteChallengeBox();
    void _ClearMissionBoxes();
    void InitMissionBoxes();
    void UpdateMissionBoxes();
    void InitMissionMultiplier();
public:
    static const jet::String k_menuName;
    static const unsigned k_num_missions;
private:
    Menu_Pause::ExitOptions m_exitOption;  // 0x124
    PauseMenuChallengeInfoBox *m_challengeBox;  // 0x128
    MissionsMultiplierBox *m_missionsMultiplierBox;  // 0x12C
    InterfaceArea *m_missionsParentObj;  // 0x130
    ustl::vector<MissionsBox *> m_missionsBox;  // 0x134
    BananasAndTokensBox *m_bananasAndTokensBox;  // 0x144
};

#endif
