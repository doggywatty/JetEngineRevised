// source/game/interfaces/menus/Menu_Shop.cpp
//
// Not ported (see Menu_Shop.h): the sounds, the popups, the tracking, the online ads and the free cash natives, which are
// left out where the .so calls them; the control flow around them is the .so's.
#include "Menu_Shop.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "box/BananasAndTokensBox.h"
#include "box/ShopBonusUpgradeItemBox.h"
#include "box/ShopBuyableItemBox.h"
#include "camera/cameramgr.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/Player.h"
#include "game/entities/Bonus.h"
#include "game/entities/BonusSet.h"
#include "game/entities/Buyable.h"
#include "game/entities/Minion.h"
#include "game/entities/MinionCostume.h"
#include "game/gamemanagers/BuyablesMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/gamestates/GS_MainMenu.h"
#include "game/gamestates/GS_Shop.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceList.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/online/store/Price.h"
#include "game/online/store/Store.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include <algorithm>
#include <cmath>
#include <ctime>

const jet::String Menu_Shop::k_menuName("Shop");

// the layouts of the promotions show where the game allows (COPPAMgr::CanShowPromoLayouts @0x5462fc: always, the manager of
// the kids' privacy rules is not in this build)
static const bool k_canShowPromoLayouts = true;

// the Android natives of the free cash button (IsFreeCashButtonEnabled @0x3e7570, IsFreeCashLoaded @0x5aff6c) and of the
// device list (nativeIsListDeviceCantLoadAlpha @0x3f53cc) ask the ads network and the device, which this build does not have
static bool IsFreeCashButtonEnabled() { return false; }
static bool IsFreeCashLoaded() { return false; }

static const jet::String &Text(const char *key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)); }

// @0x5b028c
Menu_Shop::Menu_Shop()
    : Menu_BaseCostumes(jet::String("Shop_Common")), m_canPressFreeButton(false), m_freeCashShown(false), m_freeCashFlag(false), m_bgFillRect(0), m_backButton(0),
      m_freeBananasButton(0), m_freeBananasLocked(0), m_restoreButton(0), m_newCostumes(0), m_promoCostumes(0), m_limitedCostumes(0), m_counters(0), m_textCategory(0),
      m_textTokenWarning(0), m_textTokenWarningFreeCash(0), m_textLoading(0), m_currentTab(Shop_Bonus), m_previousTab(Shop_None), m_lastRefreshedTab(Shop_None),
      m_previousTabTracking(Shop_None), m_fadeL(0), m_fadeR(0), m_freeCashTimer(0), m_unused264(0), m_list(0), m_scrollBarObject(0), m_waitingBg(0), m_firstBuy(false),
      m_tutorialState(0), m_shieldIndex(0), m_tutorialElementIndex(-1), m_tutorialStarted(false), m_freeCashEnabled(false), m_freeCashNotLoaded(true), m_freeCashChecks(0) {}

// @0x5b0400
Menu_Shop::~Menu_Shop() {
    ClearBuyablesAndUpgradesBoxes();
    delete m_counters;
}

// @0x5b04e0: the elements of the layer the shop works with, the tutorial's state and the counters. What it does not show at
// first is hidden. (The .so also asks for the ad bar's configuration when the online services are up.)
void Menu_Shop::Init() {
    Menu_Base::Init();
    Menu_BaseCostumes::Init(jet::String("Costumes_Grid"), jet::String("Index_Grid"), jet::String("Selected_costume"), jet::String("Costumes_ScrollBar"));
    m_backButton = m_basicPage->GetUIButton(jet::String("Button_back"));
    m_freeBananasButton = m_basicPage->GetUIButton(jet::String("Button_free_bananas"));
    m_freeBananasLocked = m_basicPage->GetUIButton(jet::String("Button_free_bananas_locked"));
    m_restoreButton = m_basicPage->GetUIButton(jet::String("Button_restore"));
    m_restoreButton->SetVisible(false);
    m_tabButtons.push_back(GetUIButton(jet::String("Button_bananas")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_cash")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_costumes")));
    m_tabButtons.push_back(GetUIButton(jet::String("Button_bonus")));
    m_tabButtonsPromo.push_back(GetUIButton(jet::String("Button_promo_bananas")));
    m_tabButtonsPromo.push_back(GetUIButton(jet::String("Button_promo_cash")));
    m_tabButtonsPromo.push_back(GetUIButton(jet::String("Button_promo_costumes")));
    m_tabButtonsPromo.push_back(GetUIButton(jet::String("Button_promo_bonus")));
    m_newCostumes = GetUIButton(jet::String("Button_new_costumes"));
    m_promoCostumes = GetUIButton(jet::String("Button_promo_costumes"));
    m_limitedCostumes = GetUIButton(jet::String("Button_limited_costumes"));
    m_newCostumes->SetVisible(false);
    m_promoCostumes->SetVisible(false);
    m_limitedCostumes->SetVisible(false);
    GetUIObject(jet::String("Tutorial_Buy"))->SetVisible(false);
    GetUIObject(jet::String("Tutorial_Buy2"))->SetVisible(false);
    m_tutorialState = TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_buySomething) ? 5 : 0;
    m_bgFillRect = m_basicPage->GetUIObject(jet::String("bgFillRect"));
    m_bgFillRect->SetVisible(false);
    m_counters = new BananasAndTokensBox(GetUIObject(jet::String("BananasAndTokensCounterPos")), true);
    m_textCategory = m_basicPage->GetUIText(jet::String("Category_Text"));
    m_textLoading = m_basicPage->GetUIText(jet::String("Loading_Text"));
    m_textTokenWarning = m_basicPage->GetUIText(jet::String("TokenWarning_Text"));
    m_textTokenWarningFreeCash = m_basicPage->GetUIText(jet::String("TokenWarning_Text_FreeCash"));
    m_tabHeaders.push_back(jet::String("SHOP_TAB_BANANAS"));
    m_tabHeaders.push_back(jet::String("SHOP_TAB_TOKENS"));
    m_tabHeaders.push_back(jet::String("SHOP_TAB_COSTUMES"));
    m_tabHeaders.push_back(jet::String("SHOP_TAB_BONUS"));
    m_list = GetUIList(jet::String("ItemsList"));
    m_scrollBarObject = GetUIObject(jet::String("Items_ScrollBar"));
    m_list->SetVisible(false);
    m_scrollBarObject->SetVisible(false);
    m_waitingBg = GetUIObject(jet::String("WaitingForInitializeBg"));
    m_waitingBg->SetVisible(false);
    m_fadeL = GetUIGraph(jet::String("fade_L"));
    m_fadeR = GetUIGraph(jet::String("fade_R"));
    m_firstBuy = Player::GetInstance()->GetFirstBuy();
}

// @0x5b1724
void Menu_Shop::InitTutorial() {
    if (!m_tutorialElements.empty()) return;
    m_tutorialElements.push_back(TutorialElement(GetUIObject(jet::String("Tutorial_Buy"))));
    m_tutorialElements.push_back(TutorialElement(GetUIObject(jet::String("Tutorial_Buy2"))));
    m_shieldIndex = GetIndexShield();
}

// @0x5b17a4: the costumes take their places in the scene (when the costumes are there), the list goes back to its top, and
// the shop shows the tab it was left at: the tokens one only when there is a store (else the first), or the costumes one when
// a popup asked for the shop
void Menu_Shop::OnEnter() {
    m_previousTabTracking = Shop_None;
    CameraMgr::GetInstance()->Update(99999);
    if (CostumeMgr::GetInstance()->WasReset())
        InitCostumes(jet::String("BigCostume_feet_pos"), jet::String("BigCostume_next_feet_pos"), jet::String("SmallCostume_feet_pos"),
                     jet::String("SmallCostume_next_feet_pos"));
    m_list->ResetScrollOffset();
    m_previousTab = Shop_None;
    if (m_currentTab == Shop_Tokens) {
        if (!Game::GetInstance()->AreOnlineServicesReady()) {
            m_tabButtons[(size_t)m_currentTab]->SetSelected(false);
            m_tabButtonsPromo[(size_t)m_currentTab]->SetSelected(false);
            m_currentTab = Shop_Bananas;
        } else {
            // the loading time of the store's token packs starts (tracking)
        }
        return;
    }
    // the fades at the sides of the list stay when the device can load alpha textures (nativeIsListDeviceCantLoadAlpha is false)
    if (Game::GoToShop == 2) {
        Game::GoToShop = 0;
        GoToTab(Shop_Costumes, false, false);
        OnEnter();
    }
}

// @0x5b1b50
void Menu_Shop::CheckFreeCash() {
    m_freeCashChecks++;
    const bool enabled = IsFreeCashButtonEnabled();
    if (m_freeCashEnabled != enabled) {
        m_freeCashEnabled = enabled;
        if (m_currentTab != Shop_Bonus) m_refreshOnNextFrame = true;
    }
    m_freeCashNotLoaded = !IsFreeCashLoaded();
    RefreshCostsRealMoneyTextsVisibility();
    m_freeCashTimer = 10000;
}

// @0x5b1ba8: the boxes and the costumes go; leaving for the main menu makes the minion wear the costume equipped
void Menu_Shop::OnQuit() {
    ClearBuyablesAndUpgradesBoxes();
    ClearCostumes();
    GameState *crt = GameState::GetCrtState();
    bool toMainMenu = crt->GetStateName().Equals(GS_MainMenu::k_stateName);
    if (!toMainMenu && crt->GetStateName().Equals(GS_Shop::k_stateName)) toMainMenu = GameState::GetPrevState(0)->GetStateName().Equals(GS_MainMenu::k_stateName);
    if (toMainMenu) GameLevel::GetInstance()->GetMinion()->ReloadCostumeIfNeeded();
    m_previousTabTracking = Shop_None;
}

// @0x5b1c80
void Menu_Shop::Update(int dt) {
    // (the .so shows the ad bar when it is downloaded, and the reward of the IGP network when it is given)
    if (m_refreshOnNextFrame) Refresh();
    Menu_Base::Update(dt);
    // SendTrackingLoadingTimes @0x5b566c is the tracking of how long the tokens tab took to load
    if (m_list->m_enableTouch) {
        const float total = m_list->GetTotalChildrenHeight();
        const float h = total - m_list->GetSize().y;
        const float v = -m_list->GetOffset().y / h;
        static_cast<InterfaceScrollBar*>(m_scrollBarObject)->SetValue(v);
    }
    if (m_currentTab == Shop_Tokens) {
        if (!Game::GetInstance()->AreOnlineServicesReady()) GoToTab(Shop_Bananas, false, false);
    } else if (m_currentTab == Shop_Costumes) {
        if (!m_costumes.empty()) UpdateCostumes(dt, false, false);
    }
    if (m_currentTab == Shop_Bonus) {
        if (Store::GetInstance()->IsRestoringGoldenBanana()) {
            m_freeBananasLocked->SetText(Text("LOADING"));
            m_freeBananasLocked->SetVisible(true);
            m_restoreButton->SetVisible(false);
        } else {
            m_freeBananasLocked->SetVisible(false);
            m_restoreButton->SetText(Text("STR_RESTORE"));
        }
    } else {
        m_restoreButton->SetVisible(false);
        m_restoreButton->SetActive(false);
    }
    // the buttons answer when no finger drags the wheel or the list
    const bool enable = m_currentTab == Shop_Costumes ? !(m_draggingGrid || m_draggingIndex) : !m_list->m_isDragging;
    for (size_t i = 0; i < 4; i++) {
        m_tabButtons[i]->SetActive(enable);
        m_tabButtonsPromo[i]->SetActive(enable);
    }
    m_backButton->SetActive(enable);
    m_freeBananasButton->SetActive(false);
    m_freeBananasLocked->SetActive(false);
    m_restoreButton->SetActive(enable);
    m_counters->SetActive(enable);
    UpdateButtons();
    m_freeCashTimer -= dt;
    if (m_freeCashTimer < 0) CheckFreeCash();
    UpdateTutorial();
}

// @0x5b23a0: the back key and the buttons released: back leaves (or goes to the costumes tab when the shop was called with
// the bonus tab from the main menu), the tabs open their tab, and the tutorial lets only its buttons through
void Menu_Shop::UpdateButtons() {
    MenuMgr *mm = MenuMgr::GetInstance();
    int count = mm->ACTION_GetNumReleasedButtons();
    int start = 0;
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        // KeyEventManager::ClearKeyEvents
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
        if (m_currentTab == Shop_Bonus && GS_Shop::checkBonusBack && GS_Shop::checkCallShop) {
            GoToTab(Shop_Costumes, false, false);
            GS_Shop::checkBonusBack = false;
        } else {
            mm->PopMenu();
            GameState::PopState();
            GS_Shop::checkCallShop = false;
            GS_Shop::checkBonusBack = false;
            return;
        }
    }
    for (int i = start; i < count; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (m_tutorialState == 2) {
            if (!m_upgradeBoxes[(size_t)m_shieldIndex]->IsBuyButton(b)) return;
            m_refreshOnNextFrame = true;
            m_tutorialState++;
        } else if (m_tutorialState == 3) {
            if (b != m_basicPage->GetUIButton(jet::String("Button_back"))) return;
            m_refreshOnNextFrame = true;
            m_tutorialState++;
            TutorialFinished();
        } else if (m_tutorialState == 1) {
            return;
        }
        if (b == m_backButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            if (m_currentTab == Shop_Bonus && GS_Shop::checkBonusBack && GS_Shop::checkCallShop) {
                GoToTab(Shop_Costumes, false, false);
                GS_Shop::checkBonusBack = false;
                continue;
            }
            mm->PopMenu();
            GameState::PopState();
            GS_Shop::checkCallShop = false;
            GS_Shop::checkBonusBack = false;
            return;
        }
        if (b == m_freeBananasButton) {
            if (m_canPressFreeButton) {
                m_canPressFreeButton = false;
                return;
            }
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // without the online services the popup says there is no internet (PopupMgr is not ported)
            return;
        }
        if (b == m_restoreButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            Store *store = Store::GetInstance();
            if (!store->IsIAPPurchaseInProgress()) store->RestorePendingTransactions();
            return;
        }
        int t = 0;
        for (; t < 4; t++)
            if (b == m_tabButtons[(size_t)t] || b == m_tabButtonsPromo[(size_t)t]) break;
        // a button that is not one of the tabs (the boxes' ones, which the boxes answer to themselves) leaves the tab as it is:
        // the .so goes to tab 4 here, one past the tabs' buttons, which Refresh would index
        if (t == 4) continue;
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);  // (the .so also sets sound groups of the tabs when costumes are there), and the tracking of the screen
        GoToTab((EShopTabs)t, false, false);
        return;
    }
}

// @0x5b2978: the fill behind, the wheel in the costumes tab, the "loading" of the tabs that wait for the store, the page
void Menu_Shop::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    m_bgFillRect->SetVisible(true);
    m_bgFillRect->Render(cam, painter);
    m_bgFillRect->SetVisible(false);
    if (m_currentTab == Shop_Costumes) RenderCostumes(cam, painter, false);
    if ((m_currentTab & ~2) == 1 && Store::GetInstance()->IsIAPStoreUpdating() && Game::GetInstance()->AreOnlineServicesReady()) {
        m_textLoading->SetText(Text("LOADING"));
        m_textLoading->SetVisible(true);
    } else {
        m_textLoading->SetVisible(false);
    }
    Menu_Base::Render(cam, painter);
    // DebugRenderer::Render
}

// @0x5b2af8
void Menu_Shop::SetState(int s) {
    Menu_Base::SetState(s);
    if (s == 1) Refresh();
}

// @0x5b2b24: without the costumes of the data only the bananas tab can be shown (forced); the tokens tab needs the store, or
// the shop goes back to the tab it came from (after the popup "no internet")
void Menu_Shop::GoToTab(EShopTabs tab, bool force, bool refreshOnNextFrame) {
    if (tab > Shop_Tokens && !CostumeMgr::GetInstance()->WasReset()) {
        force = true;
        tab = Shop_Bananas;
    }
    if (m_currentTab == tab) {
        if (!force) return;
        m_lastRefreshedTab = Shop_None;
    } else {
        m_previousTab = m_currentTab;
        m_tabButtons[(size_t)m_currentTab]->SetSelected(false);
        m_tabButtonsPromo[(size_t)m_currentTab]->SetSelected(false);
    }
    m_previousTabTracking = m_currentTab;
    m_currentTab = tab;
    if (refreshOnNextFrame) m_refreshOnNextFrame = true;
    else Refresh();
    if (m_currentTab == Shop_Tokens) {
        // the loading time of the tokens starts (tracking)
        if (Game::GetInstance()->AreOnlineServicesReady()) {
            Store *store = Store::GetInstance();
            if (store->IsAnyIAPTokenItemAvailable() && !store->IsIAPStoreUpdating()) {
                // the .so refreshes the IAP token list and asks STORE_NO_PACKS_AVAILABLE when there are none
            }
        }
        // PopupMgr::PushInfoPopup(POPUP_NO_INTERNET) is not ported
        GoToTab(m_previousTab == Shop_None ? Shop_Bananas : m_previousTab, false, false);
    }
}

// @0x5b2d80: the tab shows: its button selected (the ones of the bananas and tokens tabs are not shown, the costumes and
// the bonus ones take their places), its header, the promotions, and its content: the boxes of the packs or upgrades in the
// list, or the wheel of the costumes
void Menu_Shop::Refresh() {
    m_refreshOnNextFrame = false;
    m_tabButtons[(size_t)m_currentTab]->SetSelected(true);
    m_tabButtonsPromo[(size_t)m_currentTab]->SetSelected(true);
    m_textCategory->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(m_tabHeaders[(size_t)m_currentTab]));
    RefreshPromoStatus();
    m_tabButtons[1]->SetVisible(false);
    m_tabButtons[0]->SetVisible(false);
    m_tabButtons[2]->SetPos(m_tabButtons[0]->GetPos());
    m_tabButtons[3]->SetPos(m_tabButtons[1]->GetPos());
    const math::vec2<float> scroll = m_list->m_scrollTarget;
    switch (m_currentTab) {
        case Shop_Bananas:
            SetupBuyablesTab(jet::String("Bananas"));
            break;
        case Shop_Tokens:
            SetupBuyablesTab(jet::String("Tokens"));
            break;
        case Shop_Costumes:
            m_list->SetVisible(false);
            m_scrollBarObject->SetVisible(false);
            if (!m_costumes.empty()) {
                const int current = m_currentCostume;
                m_currentCostume = -1;
                SetCurrentCostume(current);
            }
            break;
        case Shop_Bonus:
            SetupUpgradesTab();
            HandleTutorial();
            break;
        default:
            break;
    }
    m_freeBananasButton->SetVisible(false);
    m_freeBananasLocked->SetVisible(false);
    m_freeBananasLocked->SetActive(false);
    RefreshCostsRealMoneyTextsVisibility();
    if (m_currentTab != Shop_Costumes) {
        GetUIGraph(jet::String("Daily_Time_Costumes_BG"))->SetVisible(false);
        if (m_lastRefreshedTab == m_currentTab) {
            m_list->m_scrollTarget = scroll;
            m_list->UpdateChildOffsets();
        } else if (m_lastRefreshedTab != Shop_None) {
            m_list->ResetScrollOffset();
        }
        m_list->Reset();
        const float scrollable = m_list->GetTotalChildrenHeight() - m_list->GetSize().y;
        const bool scrolls = scrollable > 0;
        m_scrollBarObject->SetVisible(scrolls);
        m_list->m_enableTouch = scrolls;
    } else if (m_lastRefreshedTab != Shop_Costumes || m_refreshAfterPopup) {
        RefreshCostumes();
    }
    m_lastRefreshedTab = m_currentTab;
    if (m_tutorialState == 0) SetupTutorial();
}

// @0x5b3160: the costume that is offered for a time (a redeem code's): its time left, and it goes when the time is over.
// Nothing calls it in this version of the .so.
void Menu_Shop::RefreshRedeemCostume() {
    if (m_currentTab != Shop_Costumes) return;
    MinionCostume *c = m_costumes[(size_t)m_currentCostume];
    const short id = (short)c->GetCostumeId();
    Player *player = Player::GetInstance();
    const int until = player->FindRedeemCostume(id);
    InterfaceText *text = GetUIText(jet::String("Daily_Time_Costumes"));
    if (until <= 0) return;
    int left = until - (int)time(0);
    if (left <= 0) {
        player->RemoveRedeemCostume(id);
        return;
    }
    const int sec = left % 60, min = (left / 60) % 60, hour = (left / 3600) % 24, days = left / 86400;
    jet::String t;
    if (days == 0) {
        t = jet::String::Format("%02d:%02d:%02d", hour, min, sec);
    } else if (days == 1) {
        t = jet::String::Format("%d%s %02d:%02d:%02d", 1, Text("STORE_DAY_LEFT").c_str(), hour, min, sec);
    } else {
        t = jet::String::Format("%d%s %02d:%02d:%02d", days, Text("STORE_DAYS_LEFT").c_str(), hour, min, sec);
        if (days > 10) {
            player->RemoveRedeemCostume(id);
            return;
        }
    }
    GetUIGraph(jet::String("Daily_Time_Costumes_BG"))->SetVisible(true);
    text->SetText(jet::String::Format(Text("DAILYLOG_TIME_LIMIT").c_str(), t.c_str()));
}

// the time left as the shop says it: hours, minutes and seconds, with the days before when there are
static jet::String TimeLeftText(int left) {
    const int sec = left % 60, min = (left / 60) % 60, hour = (left / 3600) % 24, days = left / 86400;
    if (days == 0) return jet::String::Format("%02d:%02d:%02d", hour, min, sec);
    return jet::String::Format("%d %s %02d:%02d:%02d", days, Text(days == 1 ? "STORE_DAY_LEFT" : "STORE_DAYS_LEFT").c_str(), hour, min, sec);
}

// @0x5b3468: for each tab, whether it has a promotion (the tab and its promo button switch) and, for the tab that is shown,
// the time the promotion lasts: those of the store, and the offers of the costumes (the first and second of the day, 2 hours
// or a day, and the magic one, 3 days), each on the costume it is for
void Menu_Shop::RefreshPromoStatus() {
    GetUIGraph(jet::String("Daily_Time_Costumes_BG"))->SetVisible(false);
    bool hasMagic = false;
    for (MinionCostume *c : m_costumes)
        if (c->GetCostumeId() == 0x19) hasMagic = true;
    Player *player = Player::GetInstance();
    const bool magic = hasMagic && player->IsMigicCostume() == 2;
    const bool first2 = player->IsGetFirstCostume() == 2, second2 = player->IsGetSecondCostume() == 2;
    const bool first1 = player->IsGetFirstCostume() == 1, second1 = player->IsGetSecondCostume() == 1;
    const bool noCostumeData = !CostumeMgr::GetInstance()->WasReset();
    const bool a44 = first1 || second2, any2 = second2 || first2, any1 = second1 || first1;
    Store *store = Store::GetInstance();
    static const char *k_categories[4] = { "Bananas", "Tokens", "Costumes", "Upgrades" };
    for (int i = 0; i < 4; i++) {
        const jet::String category(k_categories[i]);
        int fp = store->HasPromotionOnCategory(category) ? 1 : 0;
        if (store->HasIAPPromotionOnCategory(category)) fp |= 2;
        if (i == 1) {
            if (store->HasAnyPricePromotionOnCategory(category)) fp |= 2;
        } else if (i == 2 && m_currentCostume >= 0) {
            m_newCostumes->SetVisible(false);
            m_promoCostumes->SetVisible(false);
            m_limitedCostumes->SetVisible(false);
            if (first2 || fp != 0 || a44 || second1 || magic) {
                bool pricePromo = false, limited = false;
                for (MinionCostume *c : m_costumes) {
                    const bool promo = c->GetPrice()->HasPricePromotion();
                    if (promo) pricePromo = true;
                    if (c->IsTimeLimited() && !promo) limited = true;
                    if (first2 && c->GetCostumeId() == 0x12) limited = true;
                    if (second2 && c->GetCostumeId() == 0x10) limited = true;
                    if (magic && c->GetCostumeId() == 0x19) limited = true;
                }
                m_tabButtonsPromo[2] = pricePromo ? m_promoCostumes : limited ? m_limitedCostumes : m_newCostumes;
                fp = 1;
            } else {
                fp = a44;
            }
        }
        if (i > 1 && noCostumeData) {
            m_tabButtons[(size_t)i]->SetVisible(false);
            m_tabButtonsPromo[(size_t)i]->SetVisible(false);
            continue;
        }
        m_tabButtons[(size_t)i]->SetVisible(fp ? !k_canShowPromoLayouts : true);
        m_tabButtonsPromo[(size_t)i]->SetVisible(fp ? k_canShowPromoLayouts : false);
        if (i != m_currentTab) continue;
        if (fp != 0) {
            InterfaceText *text = GetUIText(jet::String(i == 2 ? "Time_Costumes" : "Time_Items"));
            int left = 0;
            if (m_currentTab == Shop_Tokens) {
                left = store->GetIAPPromotionRemainingTime();
            } else if (m_currentTab == Shop_Bonus) {
                if (fp & 2) left = store->GetIAPPromotionRemainingTime();
                if (fp & 1) {
                    const int dynamic = store->GetDynamicPricingPromotionRemainingTime();
                    left = (fp & 2) ? std::min(left, dynamic) : dynamic;
                }
            } else {
                left = store->GetDynamicPricingPromotionRemainingTime();
            }
            const bool costumeShown = m_currentTab == Shop_Costumes && !m_costumes.empty() && m_currentCostume >= 0;
            jet::String label;
            jet::String time;
            if (left > 0) {
                time = TimeLeftText(left);
                label = jet::String("SHOP_PROMO_ENDS_IN");
                if (costumeShown) {
                    MinionCostume *c = m_costumes[(size_t)m_currentCostume];
                    const bool promo = c->GetPrice()->HasPricePromotion(), limited = c->IsTimeLimited();
                    if (!promo && !limited) fp = 0;
                    else if (limited && !promo) label = jet::String("AVAILABLE_FOR");
                }
            } else if (any2) {
                if (first2) left = player->GetAwardCostumeTime_1() + 7200 - (int)::time(0);
                if (second2) left = player->GetAwardCostumeTime_2() + 7200 - (int)::time(0);
                time = TimeLeftText(left);
                if (costumeShown) {
                    MinionCostume *c = m_costumes[(size_t)m_currentCostume];
                    const bool ok = (first2 && c->GetCostumeId() == 0x12) || (second2 && c->GetCostumeId() == 0x10);
                    if (!ok) fp = 0;
                }
                label = jet::String("AVAILABLE_FOR");
            } else if (any1) {
                if (first1) left = player->GetAwardCostumeTime_1() + 86400 - (int)::time(0);
                if (second1) left = player->GetAwardCostumeTime_2() + 86400 - (int)::time(0);
                time = TimeLeftText(left);
                if (costumeShown) {
                    MinionCostume *c = m_costumes[(size_t)m_currentCostume];
                    const bool ok = (first1 && c->GetCostumeId() == 0x12) || (second1 && c->GetCostumeId() == 0x10);
                    if (!ok) fp = 0;
                }
                // the first offer of the day has its own text, over its own background
                GetUIText(jet::String("Daily_Time_Costumes"))->SetText(jet::String::Format(Text("DAILYLOG_TIME_LIMIT").c_str(), time.c_str()));
                GetUIGraph(jet::String("Daily_Time_Costumes_BG"))->SetVisible(fp ? m_currentTab == Shop_Costumes : false);
                fp = 0;
                label = jet::String();
            } else if (!magic) {
                fp = 0;
            } else {
                left = player->GetMagicCostumeTime() + 259200 - (int)::time(0);
                time = TimeLeftText(left);
                if (costumeShown && m_costumes[(size_t)m_currentCostume]->GetCostumeId() != 0x19) fp = 0;
                label = jet::String("AVAILABLE_FOR");
            }
            if (!label.empty()) {
                jet::String shown = Text(label.c_str());
                if (m_currentTab == Shop_Tokens) {
                    const jet::String description = store->GetIAPPromotionDescription();
                    if (!description.empty()) shown = description;
                }
                text->SetText(jet::String::Format(shown.c_str(), time.c_str()));
            }
        }
        GetUIGraph(jet::String("Time_Costumes_BG"))->SetVisible(fp ? (m_currentTab == Shop_Costumes ? k_canShowPromoLayouts : false) : false);
        GetUIGraph(jet::String("Time_Items_BG"))
            ->SetVisible(fp && m_currentTab != Shop_Costumes && store->HasIAPPromotionOnCategory(category) ? k_canShowPromoLayouts : false);
    }
}

// @0x5b48fc: the list gets a box for each buyable of the category, the cheaper first (the ones that are on sale or in-app
// ones, even when they are not active). The .so adds the ad item of the online store when it has one
void Menu_Shop::SetupBuyablesTab(const jet::String &category) {
    ClearBuyablesAndUpgradesBoxes();
    m_list->SetVisible(true);
    std::vector<Buyable*> buyables = BuyablesMgr::GetInstance()->GetBuyablesSortedByPrice(category);
    bool anyPromo = false;
    for (Buyable *b : buyables) {
        Price *price = b->GetPrice();
        if (!price) continue;
        if (price->IsActive() || price->GetPriceType() == Price::IAP)
            if (price->HasPricePromotion() || price->HasAmountPromotion()) anyPromo = true;
    }
    m_firstBuy = Player::GetInstance()->GetFirstBuy();
    // the ad item (ShopADItemBox) of the first purchase, and the one of the ad network, need the online ads
    Price *first = 0;
    float firstPerUnit = 0;
    for (Buyable *b : buyables) {
        Price *price = b->GetPrice();
        if (!price->IsActive() && price->GetPriceType() != Price::IAP) continue;
        if (!first) {
            first = price;
            firstPerUnit = (float)(price->GetPriceAsDouble() / price->GetAmount());
        }
        if (price->GetRegularAmount() == 0) price->SetRegularAmount(price->GetAmount());
        int percent = -1;
        if (price != first && !anyPromo) {
            float perUnit = (float)(price->GetPriceAsDouble() / price->GetAmount());
            float more = firstPerUnit / perUnit + -1.0f;
            int v = (int)std::floor(0.5f + more * 100.0f);
            const int remainder = v % 5;
            v -= remainder;
            if ((float)remainder >= 2.5f) v += 5;
            percent = v;
        }
        ShopBuyableItemBox *box = new ShopBuyableItemBox(b, percent);
        box->SetParent(m_list);
        m_buyableBoxes.push_back(box);
    }
}

// @0x5b4dc8: a box for each bonus of the set (but the ones whose next price is not active)
void Menu_Shop::SetupUpgradesTab() {
    ClearBuyablesAndUpgradesBoxes();
    m_list->SetVisible(true);
    for (Bonus *bonus : BonusSet::GetInstance()->GetBonusesArray()) {
        Price *next = bonus->GetPrice(bonus->GetCurrentLevel() + 1);
        if (next && !next->IsActive() && next->GetPriceType() != Price::IAP) continue;
        ShopBonusUpgradeItemBox *box = new ShopBonusUpgradeItemBox(bonus);
        box->SetParent(m_list);
        m_upgradeBoxes.push_back(box);
    }
}

// @0x5b4f60
void Menu_Shop::ClearBuyablesAndUpgradesBoxes() {
    for (ShopBuyableItemBox *b : m_buyableBoxes) {
        b->SetParent(0);
        delete b;
    }
    m_buyableBoxes.clear();
    for (ShopBonusUpgradeItemBox *b : m_upgradeBoxes) {
        b->SetParent(0);
        delete b;
    }
    m_upgradeBoxes.clear();
}

// @0x5b5050: the warning of the tokens tab (that the free cash is there or not)
void Menu_Shop::RefreshCostsRealMoneyTextsVisibility() {
    const bool free = m_freeBananasButton->IsVisible() ? true : m_freeBananasLocked->IsVisible();
    m_textTokenWarning->SetVisible(m_currentTab == Shop_Tokens ? !free : false);
    m_textTokenWarningFreeCash->SetVisible(m_currentTab == Shop_Tokens ? free : false);
}

// @0x5b50d4: while the tutorial waits for its button, nothing else answers
void Menu_Shop::EnableTouchesTutorial(bool enable) {
    m_freeBananasButton->SetActive(false);
    m_freeBananasLocked->SetActive(false);
    m_counters->SetActive(enable);
    for (InterfaceButton *b : m_tabButtons) b->SetActive(enable);
    for (InterfaceButton *b : m_tabButtonsPromo) b->SetActive(enable);
    for (size_t i = 0; i < m_upgradeBoxes.size(); i++)
        if ((int)i != m_shieldIndex) m_upgradeBoxes[i]->EnableTouches(false);
}

// @0x5b51ac: the tutorial of buying something starts for a player that did not do it, who has bananas to buy with
void Menu_Shop::SetupTutorial() {
    const bool notDone = !TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_buySomething);
    const int bananas = Player::GetInstance()->GetBananas();
    if (bananas > TutorialMgr::k_minBananas && notDone && !m_tutorialStarted) {
        m_tutorialStarted = true;
        m_refreshOnNextFrame = true;
        m_tutorialState = 1;
        GoToTab(Shop_Bonus, false, false);
    }
}

// @0x5b5364
void Menu_Shop::EnableChildrenWithButtons(bool enable) {
    for (ShopBuyableItemBox *b : m_buyableBoxes) b->SetActive(enable);
    for (ShopBonusUpgradeItemBox *b : m_upgradeBoxes) b->SetActive(enable);
    m_counters->SetActive(enable);
}

// @0x5b5340
int Menu_Shop::GetTrackingState(EShopTabs tab) {
    static const int k_states[5] = { 0x1bcab, 0x1bcaa, 0x1bcac, 0x1bcad, -1 };
    return (unsigned)tab <= 4 ? k_states[tab] : -1;
}

// @0x5b53f0
Menu_Shop::TutorialElement::TutorialElement(InterfaceObject *o) : m_object(o) { o->SetVisible(false); }
// @0x5b5418
void Menu_Shop::TutorialElement::LaunchTutorial() { m_object->SetVisible(true); }
// @0x5b5434
void Menu_Shop::TutorialElement::Hide() { m_object->SetVisible(false); }

// @0x5b5450
void Menu_Shop::TutorialFinished() {
    TutorialMgr::GetInstance()->SetTutorialFinished(ETutorialId::k_buySomething);
    m_tutorialStarted = false;
    m_tutorialElements.back().Hide();
}

// @0x5b54c4: what the tutorial does as it goes from state to state (1 starts it, 2 points at the buy button, 3 at the back
// button, 4 ends it)
void Menu_Shop::HandleTutorial() {
    switch (m_tutorialState) {
        case 1:
            InitTutorial();
            m_refreshOnNextFrame = true;
            m_tutorialState = 2;
            m_tutorialElementIndex = -1;
            break;
        case 2:
            m_tutorialElementIndex = 0;
            m_tutorialElements[0].LaunchTutorial();
            break;
        case 3:
            m_upgradeBoxes[(size_t)m_shieldIndex]->EnableTouches(false);
            m_tutorialElementIndex = 1;
            m_tutorialElements[0].Hide();
            m_tutorialElements[(size_t)m_tutorialElementIndex].LaunchTutorial();
            break;
        case 4:
            TutorialFinished();
            break;
        default:
            break;
    }
}

// @0x5b5564: in the middle of the tutorial the list does not scroll and the shield's box comes first
void Menu_Shop::UpdateTutorial() {
    if (m_tutorialState == 4 || m_tutorialState == 0 || m_tutorialState == 1 || m_tutorialState == 5) return;
    EnableTouchesTutorial(false);
    m_list->m_enableTouch = false;
    SetShieldFirst();
}

// @0x5b55b0: the index of the box of the shield (game item type 7)
int Menu_Shop::GetIndexShield() {
    int i = 0;
    for (ShopBonusUpgradeItemBox *b : m_upgradeBoxes) {
        if (b->GetGameItemType() == 7) break;
        i++;
    }
    return i;
}

// @0x5b55f0: the list is scrolled to the shield's box, and the tutorial's arrow goes to it
void Menu_Shop::SetShieldFirst() {
    const int index = GetIndexShield();
    const float height = m_upgradeBoxes[0]->GetSize().y;
    m_list->m_scrollTarget = math::vec2<float>(0, -height * (float)index);
    const math::vec2<float> pos = m_upgradeBoxes[(size_t)index]->GetTutorialPos();
    GetUIObject(jet::String("Tutorial_Buy"))->SetPos(pos);
}
