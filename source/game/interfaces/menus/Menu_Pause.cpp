// source/game/interfaces/menus/Menu_Pause.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the pause
// screen's static Aurora page ("IngameMenu") via Menu_Base's generic BasicPage rendering. Does not
// call GetUIObject() with guessed element names. The challenge/mission boxes, the real Exit()/
// UpdateExit() flow, and SetState/Subpages_* (not even declared in this header -- see the comment
// at the top of Menu_Pause.h) are NOT wired yet.
#include "Menu_Pause.h"
#include "MenuMgr.h"
#include "game/GameLevel.h"
#include "game/challenges/ChallengeMgr.h"
#include "game/gamestates/GameState.h"
#include "game/Game.h"
#include "Menu_Ingame.h"
#include "Menu_PortraitMessage.h"

const jet::String Menu_Pause::k_menuName("Pause");
const unsigned Menu_Pause::k_num_missions = 3;

Menu_Pause::Menu_Pause()
    : Menu_Base(jet::String("IngameMenu")), m_exitOption(Exit_None), m_challengeBox(0), m_missionsMultiplierBox(0), m_missionsParentObj(0),
      m_bananasAndTokensBox(0) {}

Menu_Pause::~Menu_Pause() {}

const jet::String &Menu_Pause::GetMenuName() const { return k_menuName; }
bool Menu_Pause::UpdateGameInBG() { return true; }
bool Menu_Pause::RenderGameInBG() { return true; }
void Menu_Pause::OnEnter() {}
// @0x48d534: the pause is left, the HUD's countdown starts again, unless the menu under the pause is the portrait message
void Menu_Pause::OnQuit() {
    DeleteChallengeBox();
    Menu_Ingame *ingame = static_cast<Menu_Ingame*>(MenuMgr::GetInstance()->GetMenu(Menu_Ingame::k_menuName));
    if (MenuMgr::GetInstance()->GetMenuBelowCurrent() == MenuMgr::GetInstance()->GetMenu(Menu_PortraitMessage::k_menuName)) return;
    ingame->StartCountdown();
}
void Menu_Pause::Init() { Menu_Base::Init(); }
// Verified structure @0x48d154 (988 bytes, not fully ported -- the rest is challenge/mission-box UI
// bookkeeping for boxes this tree's InitChallengeBox/InitMissionBoxes leave null anyway): the real binary
// only calls Menu_Base::Update(dt) while no exit is pending. UpdateExit() is called every frame (same
// spirit as the real function body that follows this check) so a real Exit() call actually fires.
void Menu_Pause::Update(int dt) {
    if (m_exitOption == Exit_None) Menu_Base::Update(dt);
    UpdateExit();
}
void Menu_Pause::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
TRACKING_SCREEN Menu_Pause::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
void Menu_Pause::Exit(Menu_Pause::ExitOptions o) { if (m_exitOption == Exit_None) m_exitOption = o; }
// Verified @0x48d094: resumes by popping GS_Pause (revealing GS_Gameplay underneath) and this menu;
// going to the main menu clears any active challenge (always a no-op for now, see ChallengeMgr.cpp) and
// pops this menu twice (this menu, then the underlying IngameMenu/Pause stack entry GS_Gameplay pushed)
// before Game::GoToMainMenu().
void Menu_Pause::UpdateExit() {
    if (m_exitOption == Exit_None) return;
    GameLevel::GetInstance()->RemoveEntities();
    if (m_exitOption == Exit_Resume) {
        GameState::PopState();
        MenuMgr::GetInstance()->PopMenu();
    }
    if (m_exitOption == Exit_MainMenu) {
        if (ChallengeMgr::GetInstance()->Challenge_IsActive()) ChallengeMgr::GetInstance()->ClearCurrentChallenge();
        MenuMgr::GetInstance()->PopMenu();
        MenuMgr::GetInstance()->PopMenu();
        Game::GetInstance()->GoToMainMenu();
    }
}
void Menu_Pause::InitChallengeBox() {}
void Menu_Pause::DeleteChallengeBox() {}
void Menu_Pause::_ClearMissionBoxes() {}
void Menu_Pause::InitMissionBoxes() {}
void Menu_Pause::UpdateMissionBoxes() {}
void Menu_Pause::InitMissionMultiplier() {}
