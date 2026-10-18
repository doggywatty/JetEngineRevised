// source/game/interfaces/menus/Menu_SendChallengeTaunt.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the "send
// challenge, pick a taunt" screen's static Aurora page ("SendChallenge_Taunt") via Menu_Base's
// generic BasicPage rendering. Does not call GetUIObject() with guessed element names. The costume
// carousel (LoadCostumes/RenderCostumes) is NOT wired yet.
#include "Menu_SendChallengeTaunt.h"

const jet::String Menu_SendChallengeTaunt::k_menuName("SendChallengeTaunt");

Menu_SendChallengeTaunt::Menu_SendChallengeTaunt()
    : Menu_Base(jet::String("SendChallenge_Taunt")), m_sendChallengeButton(0), m_backButton(0), m_selectedCostumePos(0, 0, 0),
      m_selectedCostumeRot(0, 0, 0, 1), m_selectedTauntId(0) {}

Menu_SendChallengeTaunt::~Menu_SendChallengeTaunt() {}

const jet::String &Menu_SendChallengeTaunt::GetMenuName() const { return k_menuName; }
bool Menu_SendChallengeTaunt::UpdateGameInBG() { return true; }
bool Menu_SendChallengeTaunt::RenderGameInBG() { return true; }
void Menu_SendChallengeTaunt::Init() { Menu_Base::Init(); }
void Menu_SendChallengeTaunt::Update(int dt) { Menu_Base::Update(dt); }
void Menu_SendChallengeTaunt::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
void Menu_SendChallengeTaunt::OnEnter() {}
void Menu_SendChallengeTaunt::OnQuit() {}
void Menu_SendChallengeTaunt::SetEquipped(TauntItemBox*) {}
TRACKING_SCREEN Menu_SendChallengeTaunt::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
void Menu_SendChallengeTaunt::LoadCostumes() {}
void Menu_SendChallengeTaunt::FreeCostumes() {}
void Menu_SendChallengeTaunt::UpdateCostumes(int) {}
void Menu_SendChallengeTaunt::RenderCostumes() {}
bool Menu_SendChallengeTaunt::SendChallenge() { return false; }
