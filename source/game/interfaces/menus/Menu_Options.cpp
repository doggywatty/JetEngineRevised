// source/game/interfaces/menus/Menu_Options.cpp
//
// Not ported: the tracking of the screens (GameTrackingMgr::SendVisitScreenEvent), the key events
// (KeyEventManager::ClearKeyEvents) and the debug logs; the control flow around them is the .so's. The connectivity page
// (OptionsConnectionPage) and the redeem code page (RedeemCodePage) are not made: their tabs are not there in this
// build (COPPAMgr::AreSocialFeaturesEnabled is false, and Init hides the redeem code button), so their pointers stay empty.
#include "Menu_Options.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/entities/Minion.h"
#include "game/gamemanagers/COPPAMgr.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceList.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "subpages/OptionsAboutPage.h"
#include "subpages/OptionsHelpPage.h"
#include "subpages/OptionsSettingsPage.h"

const jet::String Menu_Options::k_menuName("Options");
bool isMenuOption = false;
bool isBannerVisible = false;
bool isUpdateOnce = false;
bool g_bBackToMenuOption = false;

// @0x48b164
Menu_Options::Menu_Options()
    : Menu_Base(jet::String("Options_Common")), m_backButton(0), m_ccButton(0), m_subpagesParent(0), m_subpagesScroll(0), m_textCategory(0), m_tab(Options_None),
      m_shownTab(Options_None), m_checkContent(false), m_settingsPage(0), m_connectionPage(0), m_helpPage(0), m_aboutPage(0), m_redeemCodePage(0) {}

// @0x48b27c: the pages are children of the list and go with it (the .so frees them here)
Menu_Options::~Menu_Options() {
    delete m_settingsPage;
    delete m_aboutPage;
}

// @0x48b38c: the buttons (the redeem code one is hidden), the headers of the tabs and the pages in the list
void Menu_Options::Init() {
    Menu_Base::Init();
    m_backButton = GetUIButton(jet::String("BackButton"));
    m_ccButton = GetUIButton(jet::String("CCButton"));
    m_tabButtons.push_back(GetUIButton(jet::String("B_Settings")));
    m_tabButtons.push_back(GetUIButton(jet::String("B_Help")));
    m_tabButtons.push_back(GetUIButton(jet::String("B_About")));
    InterfaceButton *redeem = GetUIButton(jet::String("B_RedeemCode"));
    redeem->SetVisible(false);
    m_tabButtons.push_back(redeem);
    m_tabButtons.push_back(GetUIButton(jet::String("B_Achievements")));
    m_tabHeaderStrings.push_back(jet::String("OPTIONS_TAB_SETTINGS"));
    m_tabHeaderStrings.push_back(jet::String("OPTIONS_TAB_CONNECTIVITY"));
    m_tabHeaderStrings.push_back(jet::String("OPTIONS_TAB_HELP"));
    m_tabHeaderStrings.push_back(jet::String("OPTIONS_TAB_ABOUT"));
    m_tabHeaderStrings.push_back(jet::String("OPTIONS_TAB_REDEEM"));
    m_textCategory = GetUIText(jet::String("Category_Text"));
    m_subpagesParent = GetUIList(jet::String("SubPagesList_Parent"));
    m_subpagesScroll = static_cast<InterfaceScrollBar*>(GetUIObject(jet::String("SubPagesList_ScrollBar")));
    m_settingsPage = new OptionsSettingsPage();
    m_settingsPage->SetParent(m_subpagesParent);
    m_settingsPage->SetVisible(false);
    m_aboutPage = new OptionsAboutPage();
    m_aboutPage->SetParent(m_subpagesParent);
    m_aboutPage->SetVisible(false);
    m_helpPage = new OptionsHelpPage();
    m_helpPage->SetParent(m_subpagesParent);
    m_helpPage->SetVisible(false);
    SetTab(Options_Settings);
}

// @0x48b93c
void Menu_Options::OnEnter() {
    if (m_settingsPage) m_settingsPage->OnEnter();
    m_subpagesParent->ResetScrollOffset();
    if (COPPAMgr::GetInstance()->AreSocialFeaturesEnabled()) {
        // OptionsConnectionPage::CanShowDLCNotify @0x4a2c9c is false: the page is not here
        UpdateTabNotify(Options_Connection, true, false, jet::String("B_Achievements"), jet::String("B_Achievements_alert"));
    }
    Refresh();
    isMenuOption = true;
}

// @0x48b9dc
void Menu_Options::OnQuit() {
    GameLevel::GetInstance()->GetMinion()->ReloadCostumeIfNeeded();
    if (m_settingsPage) m_settingsPage->OnMenuQuit();
    isMenuOption = false;
    if (isBannerVisible) isBannerVisible = false;
}

// @0x48ba50: the back key and button leave; a tab button shows its page
void Menu_Options::Update(int dt) {
    COPPAMgr *coppa = COPPAMgr::GetInstance();
    if (!isUpdateOnce && !coppa->AreSocialFeaturesEnabled()) {
        // without the social features the connectivity tab goes, and the tabs that stay are put in a row again
        isUpdateOnce = true;
        m_tabButtons.pop_back();
        GetUIButton(jet::String("B_Achievements_alert"))->SetVisible(false);
        GetUIButton(jet::String("B_Achievements"))->SetVisible(false);
        math::vec2<float> pos = GetUIButton(jet::String("B_Achievements"))->GetPos();
        GetUIButton(jet::String("B_Settings"))->SetPos(pos);
        m_tabButtons.push_back(GetUIButton(jet::String("B_Settings")));
        m_tabButtons.push_back(GetUIButton(jet::String("B_Help")));
        m_tabButtons.push_back(GetUIButton(jet::String("B_About")));
        InterfaceButton *redeem = GetUIButton(jet::String("B_RedeemCode"));
        redeem->SetVisible(false);
        m_tabButtons.push_back(redeem);
    }
    if (m_refreshOnNextFrame) Refresh();
    MenuMgr *mm = MenuMgr::GetInstance();
    const int tabCount = coppa->AreSocialFeaturesEnabled() ? 5 : 4;
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
        const bool notDragging = !m_subpagesParent->m_isDragging;
        for (int i = 0; i < tabCount; i++) m_tabButtons[(size_t)i]->SetActive(notDragging);
        m_backButton->SetActive(notDragging);
    }
    Menu_Base::Update(dt);
    const int count = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < count; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_backButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            mm->PopMenu();
            break;
        }
        for (int t = 0; t < tabCount; t++)
            if (b == m_tabButtons[(size_t)t]) {
                SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);  // and GameTrackingMgr::SendVisitScreenEvent(GetCurrentTrackingScreen(), GetTrackingState(t))
                SetTab(t);
                return;
            }
    }
    if (coppa->AreSocialFeaturesEnabled())
        UpdateTabNotify(Options_Connection, false, false, jet::String("B_Achievements"), jet::String("B_Achievements_alert"));
    // the banner of the ads shows over the options when the online services are up
    if (Game::GetInstance()->AreOnlineServicesReady()) {
        if (!isBannerVisible && isMenuOption) isBannerVisible = true;
    } else {
        isBannerVisible = false;
    }
}

// @0x48bf60
void Menu_Options::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }

// @0x48bf64: only the page of the tab shows; the header and the scroll bar follow the page
void Menu_Options::Refresh() {
    m_refreshOnNextFrame = false;
    m_subpagesParent->m_flick = (m_tab == Options_About);
    // the redeem code page (shown at Options_RedeemCode) is not made
    m_settingsPage->SetVisible(m_tab == Options_Settings);
    m_aboutPage->SetVisible(m_tab == Options_About);
    // the connectivity page (shown at Options_Connection with the social features) is not made
    m_helpPage->SetVisible(m_tab == Options_Help);
    if (m_tab == Options_Settings) {
        m_settingsPage->Refresh();
    } else if (m_tab == Options_About) {
        m_aboutPage->Refresh();
    } else if (m_tab == Options_Help) {
        m_helpPage->OnEnter();
        m_helpPage->Refresh();
    }
    m_checkContent = false;
    m_textCategory->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(m_tabHeaderStrings[(size_t)m_tab]).c_str());
    if (m_tab != m_shownTab) m_subpagesParent->ResetScrollOffset();
    m_subpagesParent->Reset();
    m_backButton->Reset();
    m_ccButton->Reset();
    const float overflow = m_subpagesParent->GetTotalChildrenHeight() - m_subpagesParent->GetSize().y;
    const bool scrolls = overflow > 0;
    m_subpagesScroll->SetVisible(scrolls);
    m_subpagesParent->m_enableTouch = scrolls;
    m_shownTab = m_tab;
}

// @0x48c1f0: the button of the tab shows as selected, the page follows at the next refresh
void Menu_Options::SetTab(int tab) {
    if (tab == m_tab) return;
    if (m_tab <= 4) m_tabButtons[(size_t)m_tab]->SetSelected(false);
    m_tabButtons[(size_t)tab]->SetSelected(true);
    m_refreshOnNextFrame = true;
    m_tab = tab;
}

// @0x48c234: the screens the tracking knows the tabs by
int Menu_Options::GetTrackingState(int tab) {
    static const int k_screens[6] = { 125461, 125461, 125461, 125461, 113842, 125461 };
    return (tab >= 0 && tab <= 5) ? k_screens[tab] : -1;
}

// @0x48c258: the button of the tab is the one of name0, or the one of name1 (with an alert on it) when there is news
void Menu_Options::UpdateTabNotify(int tab, bool force, bool notify, const jet::String &name0, const jet::String &name1) {
    if (!force) {
        InterfaceButton *cur = m_tabButtons[(size_t)tab];
        if (cur == GetUIButton(notify ? name1 : name0)) return;
    }
    const bool selected = m_tabButtons[(size_t)tab]->IsSelected();
    GetUIButton(name0)->SetVisible(!notify);
    InterfaceButton *alert = GetUIButton(name1);
    alert->SetVisible(notify);
    m_tabButtons[(size_t)tab] = notify ? alert : GetUIButton(name0);
    m_tabButtons[(size_t)tab]->SetSelected(selected);
}

// @0x48c320
void Menu_Options::RequestCheckForContent() {
    if (COPPAMgr::GetInstance()->AreSocialFeaturesEnabled()) SetTab(Options_Connection);
    m_checkContent = true;
}

// @0x48c368
bool Menu_Options::CanAllowDlcErrorMessages() {
    if (m_tab == Options_Settings) return !Game::GetInstance()->IsDlcOnDemandFinished();
    if (m_tab == Options_Connection) return false;  // the page is not here (its pending download of the content)
    return false;
}
