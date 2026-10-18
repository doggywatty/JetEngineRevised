// source/game/interfaces/menus/Menu_ResultEndScreen.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the end-of-run
// result screen's static Aurora page ("Ingame_ResultScreen") via Menu_Base's generic BasicPage
// rendering. Does not call GetUIObject() with guessed element names. The scrollable sub-pages
// (tip/leaderboard/locations/missions/bapples) and claimed-reward bookkeeping are NOT wired yet.
// Note: the real Menu_ResultEndScreen::Render() is just `b Menu_Base::Render` (a tail call, no
// override logic at all) -- confirms this minimal pattern matches the original exactly for this
// one method, not just as an approximation.
#include "Menu_ResultEndScreen.h"
#include "../interfaceobjects/Interface3DCostumePos.h"
#include "game/Game.h"
#include "game/Statistics.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "localization/formatter.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"

const jet::String Menu_ResultEndScreen::k_menuName("ResultEndScreen");
const int Menu_ResultEndScreen::k_autoScrollingPageTimer = 5000;
const int Menu_ResultEndScreen::k_pageOffsetDelayTime = 300;

Menu_ResultEndScreen::Menu_ResultEndScreen()
    : Menu_Base(jet::String("Ingame_ResultScreen")), m_exitOption(EExitOptionsDef::Exit_None), m_subpageInfoTitle(0), m_curSubPageId(0), m_targetPageId(0),
      m_playAgainButton(0), m_nextNextButton(0), m_prevPageButton(0), m_nextPageButton(0), m_shareButton(0), m_backToMenuButton(0), m_shopButton(0),
      m_textBananas(0), m_subpagesParentArea(0), m_subpagesClip(0, 0, 0, 0), m_bananasAndTokensBox(0), m_isFirstShow(true), m_isAutoScrollingEnabled(false),
      m_autoScrollinPageTimer(0), m_isNextPageEnabled(false), m_pageOffsetDelayer(0.0f, 1.0f, 0), m_claimedBananas(0), m_claimedTokens(0) {}

Menu_ResultEndScreen::~Menu_ResultEndScreen() {}

const jet::String &Menu_ResultEndScreen::GetMenuName() const { return k_menuName; }
bool Menu_ResultEndScreen::UpdateGameInBG() { return true; }
bool Menu_ResultEndScreen::RenderGameInBG() { return true; }
void Menu_ResultEndScreen::Init() { Menu_Base::Init(); }
void Menu_ResultEndScreen::Update(int dt) { Menu_Base::Update(dt); }
void Menu_ResultEndScreen::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
// @0x490d04 (the .so's OnEnter ends with a call to the virtual Refresh, slot 0x24 of the address point)
void Menu_ResultEndScreen::OnEnter() { Refresh(); }
void Menu_ResultEndScreen::OnQuit() {}

// @0x490db8: the run's values of the result page. Each text is the babel string (or the measure) of the current run's statistics
// (GetRanGameDistance, GetCurrentRunBananas, GetScore, GetScoreMultiplier), formatted by the game's babel formatter.
// Not ported in this function: the social share visibility (@0x490dbc, the object at +0x168 with the social flag), the page list loop
// (+0x134..0x138), the new-record score block (GameLevel+0x66F, its writer is not located, see re/TODO.md), and HandleTutorial (@0x49257c).
void Menu_ResultEndScreen::Refresh() {
    babel::Babel *babel = babel::Babel::GetInstance();
    babel::Formatter *fmt = babel->GetFormatter();
    Statistics *st = Statistics::GetInstance();

    // "Common_Distance_Value" = Format(babel "DISTANCE_VALUE", FormatMeasure(run distance))
    const jet::String distanceKey("DISTANCE_VALUE");
    const jet::String distanceFormat = babel->GetStringMgr()->Get(distanceKey);
    const jet::String distance = fmt->FormatMeasure(st->GetRanGameDistance());
    GetUIText(jet::String("Common_Distance_Value"))->SetText(jet::String::Format(distanceFormat.c_str(), distance.c_str()));

    // "Common_Multiplier_Value" = "x" + FormatMeasure(score multiplier)
    const jet::String multiplier = fmt->FormatMeasure(st->GetScoreMultiplier());
    GetUIText(jet::String("Common_Multiplier_Value"))->SetText(jet::String("x") + multiplier);

    // "Common_Score_Value" = FormatMeasure(score)
    GetUIText(jet::String("Common_Score_Value"))->SetText(fmt->FormatMeasure(st->GetScore()));

    // "Common_Bananas_Value" = FormatMeasure(bananas of the run)
    GetUIText(jet::String("Common_Bananas_Value"))->SetText(fmt->FormatMeasure(st->GetCurrentRunBananas()));

    RefreshCostumes();
    Subpages_Refresh();
    UpdateClaimedInfo();
}
void Menu_ResultEndScreen::SetFirstShow() { m_isFirstShow = true; }
// @0x492688
void Menu_ResultEndScreen::RefreshCostumes() {
    GetUI3DCostumePos(jet::String("Common_photo"))->SetAnimixerState(Interface3DCostumePos::k_Minion_Costume_State_Name_Idle);
}
void Menu_ResultEndScreen::AddClaimedPriceAmount(const Price&) {}
TRACKING_SCREEN Menu_ResultEndScreen::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
void Menu_ResultEndScreen::Exit(safe_enum<Menu_ResultEndScreen::EExitOptionsDef, Menu_ResultEndScreen::EExitOptionsDef::type> o) { m_exitOption = o; }
void Menu_ResultEndScreen::Subpages_SetTargetPage(int p) { m_targetPageId = p; }
void Menu_ResultEndScreen::Subpages_Update(int) {}
void Menu_ResultEndScreen::Subpages_SetOffset(const math::vec2<float>&) {}
void Menu_ResultEndScreen::Subpages_OnEnterInit() {}
void Menu_ResultEndScreen::Subpages_Refresh() {}
void Menu_ResultEndScreen::Subpages_CheckShowPrevNextButtons() {}
void Menu_ResultEndScreen::Subpages_UpdateAutoScrolling(int) {}
void Menu_ResultEndScreen::Subpages_UpdateNextButtonPress(bool) {}
void Menu_ResultEndScreen::Subpages_SetOffsetDelayer(float, float) {}
void Menu_ResultEndScreen::UpdateClaimedInfo() {}
