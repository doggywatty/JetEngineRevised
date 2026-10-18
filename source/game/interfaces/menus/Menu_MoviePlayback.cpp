// source/game/interfaces/menus/Menu_MoviePlayback.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the movie
// playback overlay's static Aurora page ("MoviePlayback") via Menu_Base's generic BasicPage
// rendering (the skip button / subtitles). Does not call GetUIObject() with guessed element
// names. Real movie playback itself (clara::Movie) is a separate system, not this menu's concern.
#include "Menu_MoviePlayback.h"

const jet::String Menu_MoviePlayback::k_menuName("MoviePlayback");

Menu_MoviePlayback::Menu_MoviePlayback()
    : Menu_Base(jet::String("MoviePlayback")), m_skipButtonArea(0), m_skipButton(0), m_subtitleText(0), m_skipMovie(false) {}

Menu_MoviePlayback::~Menu_MoviePlayback() {}

const jet::String &Menu_MoviePlayback::GetMenuName() const { return k_menuName; }
void Menu_MoviePlayback::Init() { Menu_Base::Init(); }
void Menu_MoviePlayback::Update(int dt) { Menu_Base::Update(dt); }
void Menu_MoviePlayback::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
void Menu_MoviePlayback::OnEnter() {}
void Menu_MoviePlayback::OnQuit() {}
void Menu_MoviePlayback::Refresh() {}
bool Menu_MoviePlayback::UpdateGameInBG() { return true; }
bool Menu_MoviePlayback::RenderGameInBG() { return true; }
void Menu_MoviePlayback::EnableSkipButton(bool) {}
bool Menu_MoviePlayback::ShouldSkipMovie() { return m_skipMovie; }
void Menu_MoviePlayback::ShowSubtitle(const char*) {}
void Menu_MoviePlayback::HideSubtitle() {}
TRACKING_SCREEN Menu_MoviePlayback::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
