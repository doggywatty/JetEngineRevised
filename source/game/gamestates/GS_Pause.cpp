// source/game/gamestates/GS_Pause.cpp
#include "GS_Pause.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/interfaces/menus/Menu_Pause.h"
#include "game/interfaces/menus/MenuMgr.h"

const jet::String GS_Pause::k_stateName("GS_Pause");
// The menu music label: the literal of GS_Pause's static initializer (@0x3bf7b0, "ingame_menu").
const jet::String GS_Pause::k_musicLabel("ingame_menu");

// Verified @0x50397c: sets Game::m_paused true, pushes the pause menu (Menu_Pause, real Aurora
// level name "IngameMenu" despite nothing in that name suggesting "pause"), stops whatever menu music was playing.
GS_Pause::GS_Pause() : m_music(-1) {
    Game::GetInstance()->SetPaused(true);
    MenuMgr::GetInstance()->PushMenu(Menu_Pause::k_menuName);
    Game::GetInstance()->StopMenuMusic();
}

// Verified @0x503a04: unsets Game::m_paused and stops the menu music.
GS_Pause::~GS_Pause() {
    Game::GetInstance()->SetPaused(false);
    Game::GetInstance()->StopMenuMusic();
}

const jet::String &GS_Pause::GetStateName() { return k_stateName; }

// Verified @0x503a84/0x503a88: genuinely empty in the real binary, not a gap.
void GS_Pause::ResumeState() {}
void GS_Pause::SuspendState() {}

// Verified @0x503a8c: if no menu music handle is currently set, start it.
void GS_Pause::UpdateState() {
    if (Game::GetInstance()->GetMenuMusic() < 0) Game::GetInstance()->PlayMenuMusic(k_musicLabel);
}

// Verified @0x503ad4: tail call to GameLevel::GetInstance()->Render(), same pattern as every other
// GameState this session (GS_MainMenu excepted, which has its own override of this exact body).
void GS_Pause::RenderState() { GameLevel::GetInstance()->Render(); }
