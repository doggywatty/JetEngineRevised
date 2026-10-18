// source/game/interfaces/menus/Menu_Goals.cpp
//
// Not ported: the tracking of the screens (GameTrackingMgr::SendVisitScreenEvent,
// TrackingScreenMgr::Update), the blind box popup (PopupBlindBoxPrize of the button of the bapples' tab) and KeyEventManager::ClearKeyEvents; the control
// flow around them is the .so's. The pages of the leaderboard tab (RedeemCodePage: its button lies beyond the right side of
// the screen) and of the bapples' tab (GoalsBappleObjectivesPage: the hunt is never started and Init hides its button) are
// not made, so their pointers stay empty and nothing asks them for anything.
#include "Menu_Goals.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "Menu_Options.h"
#include "game/Player.h"
#include "game/interfaces/PopupMgr.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceList.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/box/BananasAndTokensBox.h"
#include "game/interfaces/menus/subpages/GoalsAchievementsPage.h"
#include "game/interfaces/menus/subpages/GoalsLocationsPage.h"
#include "game/interfaces/menus/subpages/GoalsMissionsPage.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"

const jet::String Menu_Goals::k_menuName("Goals");
bool Menu_Goals::checkSoundClickOpen = false;

// @0x4e77dc: the locations tab is the one the menu opens at, until it is told otherwise
Menu_Goals::Menu_Goals()
    : Menu_Base(jet::String("Goals_Common")), m_backButton(0), m_subpagesParent(0), m_subpagesScroll(0), m_textCategory(0), m_bananasAndTokensBox(0), m_targetState(Goals_Locations),
      m_state(Goals_Missions), m_previousState(Goals_Missions), m_redeemCodePage(0), m_missionsPage(0), m_achievementsPage(0), m_locationsPage(0), m_bapplesPage(0),
      m_openButton(0), m_itemNumber(0), m_showAchievementNotify(false), m_gsAchievementButton(0) {}

// @0x4e78f4: the pages go with the menu
Menu_Goals::~Menu_Goals() {
    delete m_missionsPage;
    delete m_achievementsPage;
    delete m_locationsPage;
    delete m_bananasAndTokensBox;
}

// @0x4e7a48: the buttons of the tabs in the order of the tabs (the one of the bapples is hidden), the texts of their headers,
// the counters, and the page of each tab in the list
void Menu_Goals::Init() {
    Menu_Base::Init();
    m_backButton = GetUIButton(jet::String("Button_back"));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_Missions")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_Achievements")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_HallOfFame")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_Locations")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_Bapples")));
    GetUIButton(jet::String("Button_Bapples"))->SetVisible(false);
    m_tabHeaderStrings.push_back(jet::String("MISSIONS"));
    m_tabHeaderStrings.push_back(jet::String("ACHIEVEMENTS"));
    m_tabHeaderStrings.push_back(jet::String("LEADERBOARD"));
    m_tabHeaderStrings.push_back(jet::String("LOCATIONS_TITLE"));
    m_tabHeaderStrings.push_back(jet::String("BAPPLES_DAILYMENU_DAILY_OBJECTIVE"));
    m_textCategory = GetUIText(jet::String("Category_Text"));
    m_bananasAndTokensBox = new BananasAndTokensBox(GetUIObject(jet::String("BananasAndTokensCounterPos")), true);
    m_subpagesParent = GetUIList(jet::String("SubPagesList_Parent"));
    m_subpagesScroll = static_cast<InterfaceScrollBar*>(GetUIObject(jet::String("SubPagesList_ScrollBar")));
    // the page of the leaderboard tab (RedeemCodePage) is not made
    m_missionsPage = new GoalsMissionsPage();
    m_missionsPage->SetParent(m_subpagesParent);
    m_missionsPage->SetVisible(false);
    m_achievementsPage = new GoalsAchievementsPage();
    m_achievementsPage->SetParent(m_subpagesParent);
    m_achievementsPage->SetVisible(false);
    m_gsAchievementButton = GetUIButton(jet::String("GS_Achivement"));
    m_gsAchievementButton->SetVisible(false);
    m_locationsPage = new GoalsLocationsPage();
    m_locationsPage->SetParent(m_subpagesParent);
    m_locationsPage->SetVisible(false);
    // the page of the bapples' tab (GoalsBappleObjectivesPage) is not made
    m_openButton = GetUIButton(jet::String("OpenButton"));
    m_itemNumber = GetUIText(jet::String("Item_Number"));
    m_openButton->SetVisible(false);
}

// @0x4e8320: the tab the menu was told to open at; the locations are made again
void Menu_Goals::OnEnter() {
    SetState(m_targetState, true);
    UpdateShowAchievementNotify(true);
    m_previousState = Goals_None;
    m_locationsPage->OnEnter();
}

// @0x4e8358: the back key and button leave; a tab button shows its page; the open button of the bapples' tab opens a prize
void Menu_Goals::Update(int dt) {
    if (m_refreshOnNextFrame) Refresh();
    UpdateShowAchievementNotify(false);
    TutorialMgr::GetInstance()->ShowMuliplierPerkDialog();
    MenuMgr *mm = MenuMgr::GetInstance();
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        g_bBackToMenuOption = true;
        // KeyEventManager::ClearKeyEvents
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
        mm->PopMenu();
        return;
    }
    if (m_subpagesParent->m_enableTouch) {
        const float total = m_subpagesParent->GetTotalChildrenHeight();
        const float h = total - m_subpagesParent->GetSize().y;
        const float v = -m_subpagesParent->GetOffset().y / h;
        m_subpagesScroll->SetValue(v);
    }
    // the buttons are only touched when the list is not being dragged (and the achievements' grid is not)
    bool active;
    if (m_subpagesParent->m_isDragging) active = false;
    else if (m_state != Goals_Achievements) active = true;
    else active = !m_achievementsPage->ShouldResetChildren();
    for (size_t i = 0; i < m_tabButtons.size(); i++) m_tabButtons[i]->SetActive(active);
    m_backButton->SetActive(active);
    Menu_Base::Update(dt);
    const int count = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < count; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_backButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            mm->PopMenu();
            return;
        }
        if (b == m_openButton) {
            checkSoundClickOpen = true;
            // PopupMgr::PushPopup(new PopupBlindBoxPrize(BappleMgr::GetBappleDaily(Player::GetLastUnopenedSilverBlindBox()), the box)): the button is only
            // shown by the bapples' tab, which is not reachable
            continue;
        }
        for (int t = 0; t < 5; t++)
            if (b == m_tabButtons[(size_t)t]) {
                SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);  // and GameTrackingMgr::SendVisitScreenEvent(GetCurrentTrackingScreen(), GetTrackingState(t))
                SetState(t, false);
                return;
            }
    }
}

// @0x4e86b4
void Menu_Goals::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }

// @0x4e86b8: only the page of the tab shows and is brought up to date; the header and the scroll bar follow the page
void Menu_Goals::Refresh() {
    m_refreshOnNextFrame = false;
    m_missionsPage->SetVisible(m_state == Goals_Missions);
    m_achievementsPage->SetVisible(m_state == Goals_Achievements);
    // the page of the leaderboard tab (RedeemCodePage, shown at Goals_HallOfFame) is not made
    m_locationsPage->SetVisible(m_state == Goals_Locations);
    // the page of the bapples' tab (GoalsBappleObjectivesPage, shown at Goals_Bapples) is not made
    m_openButton->SetVisible(false);
    if (m_state == Goals_HallOfFame) {
        // RedeemCodePage::Refresh() (the page is not made)
    } else if (m_state == Goals_Missions) {
        m_missionsPage->Refresh();
    } else if (m_state == Goals_Achievements) {
        m_achievementsPage->Refresh();
    } else if (m_state == Goals_Locations) {
        m_locationsPage->Refresh();
    } else if (m_state == Goals_Bapples) {
        // GoalsBappleObjectivesPage::Refresh() (the page is not made); the blind boxes the bapples have given are counted
        const unsigned boxes = Player::GetInstance()->CountUnopenedSilverBlindBoxes();
        if (boxes) {
            m_openButton->SetVisible(true);
            m_itemNumber->SetText(jet::String::Format("%d", (int)boxes));
        }
    }
    m_textCategory->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(m_tabHeaderStrings[(size_t)m_state]).c_str());
    m_subpagesParent->ResetScrollOffset();
    m_subpagesParent->Reset();
    const float overflow = m_subpagesParent->GetTotalChildrenHeight() - m_subpagesParent->GetSize().y;
    const bool scrolls = overflow > 0;
    m_subpagesScroll->SetVisible(scrolls);
    m_subpagesParent->m_enableTouch = scrolls;
}

// @0x4e890c: the tab to show: its button shows as selected and the pages follow at the next refresh; the missions are looked
// at again when their tab is opened
void Menu_Goals::SetState(int tab, bool keepPrevious) {
    const int shown = m_state;
    m_targetState = tab;
    if (tab != shown) {
        if (!keepPrevious) m_previousState = shown;
    } else if (!keepPrevious) {
        return;
    }
    if (shown <= 4) m_tabButtons[(size_t)shown]->SetSelected(false);
    m_tabButtons[(size_t)tab]->SetSelected(true);
    m_refreshOnNextFrame = true;
    m_state = tab;
    if (tab == Goals_Missions) MissionMgr::GetInstance()->RefreshGameMissions();
    if (m_state == Goals_Locations) {
        if (!TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay))
            PopupMgr::GetInstance()->PushInfoPopup(std::string(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("TUTORIAL_FIRST")).c_str()));
    }
    // TrackingScreenMgr::Update()
}

// @0x4e8a90: the achievements' tab has another button when a prize is to be taken (it shows what is there)
void Menu_Goals::UpdateShowAchievementNotify(bool force) {
    const bool pending = AchievementsMgr::GetInstance()->HasAnyPrizeToClaim();
    if (!force && m_showAchievementNotify == pending) return;
    m_showAchievementNotify = pending;
    const bool selected = m_tabButtons[1]->IsSelected();
    InterfaceButton *plain = GetUIButton(jet::String("Button_Achievements"));
    plain->SetVisible(!m_showAchievementNotify);
    InterfaceButton *claim = GetUIButton(jet::String("Button_Achievements_claim"));
    claim->SetVisible(m_showAchievementNotify);
    m_tabButtons[1] = m_showAchievementNotify ? claim : plain;
    m_tabButtons[1]->SetSelected(selected);
}

// @0x4e8bd8: the screens the tracking knows the tabs by
int Menu_Goals::GetTrackingState(int tab) {
    static const int k_screens[6] = { 113846, 113847, -1, 122086, 127839, -1 };
    return (tab >= 0 && tab <= 5) ? k_screens[tab] : -1;
}

// @0x4e8bfc
bool Menu_Goals::AnyTabWithNotification() { return AchievementsMgr::GetInstance()->HasAnyPrizeToClaim(); }
