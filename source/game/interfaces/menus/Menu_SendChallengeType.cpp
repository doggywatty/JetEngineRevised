// source/game/interfaces/menus/Menu_SendChallengeType.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the "send
// challenge, pick a type" screen's static Aurora page ("SendChallenge_Type") via Menu_Base's
// generic BasicPage rendering. Does not call GetUIObject() with guessed element names.
#include "Menu_SendChallengeType.h"

const jet::String Menu_SendChallengeType::k_menuName("SendChallengeType");

Menu_SendChallengeType::Menu_SendChallengeType()
    : Menu_Base(jet::String("SendChallenge_Type")), m_selectText(0), m_scoreButton(0), m_distanceButton(0), m_bananasButton(0), m_backButton(0),
      m_tauntObj(0) {}

Menu_SendChallengeType::~Menu_SendChallengeType() {}

const jet::String &Menu_SendChallengeType::GetMenuName() const { return k_menuName; }
bool Menu_SendChallengeType::UpdateGameInBG() { return true; }
bool Menu_SendChallengeType::RenderGameInBG() { return true; }
void Menu_SendChallengeType::Init() { Menu_Base::Init(); }
void Menu_SendChallengeType::Update(int dt) { Menu_Base::Update(dt); }
void Menu_SendChallengeType::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
void Menu_SendChallengeType::OnEnter() {}
void Menu_SendChallengeType::OnQuit() {}
bool Menu_SendChallengeType::CheckTauntButtonPress(InterfaceButton*) { return false; }
