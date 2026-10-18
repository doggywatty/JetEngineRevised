// source/game/interfaces/menus/Menu_ChallengeResult.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the challenge
// result screen's static Aurora page ("Ingame_ChallengeResult") via Menu_Base's generic BasicPage
// rendering. Does not call GetUIObject() with guessed element names.
#include "Menu_ChallengeResult.h"

const jet::String Menu_ChallengeResult::k_menuName("ChallengeResult");

Menu_ChallengeResult::Menu_ChallengeResult()
    : Menu_Base(jet::String("Ingame_ChallengeResult")), m_shouldExit(false), m_wasChallengeWon(false), m_challengerText(0), m_challengerAvatarArea(0),
      m_tauntObj(0) {}

const jet::String &Menu_ChallengeResult::GetMenuName() const { return k_menuName; }
bool Menu_ChallengeResult::UpdateGameInBG() { return true; }
bool Menu_ChallengeResult::RenderGameInBG() { return true; }
void Menu_ChallengeResult::OnEnter() {}
void Menu_ChallengeResult::OnQuit() {}
void Menu_ChallengeResult::Init() { Menu_Base::Init(); }
void Menu_ChallengeResult::Update(int dt) { Menu_Base::Update(dt); }
void Menu_ChallengeResult::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
TRACKING_SCREEN Menu_ChallengeResult::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
void Menu_ChallengeResult::Exit() {}
void Menu_ChallengeResult::SetMenuChallengeWon(bool won) { m_wasChallengeWon = won; }
