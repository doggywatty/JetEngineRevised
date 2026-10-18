// source/game/interfaces/menus/Menu_MainMenu.cpp
#include "Menu_MainMenu.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "Menu_Goals.h"
#include "Menu_Social.h"
#include "game/interfaces/popups/PopupGoToLocation.h"
#include "Menu_Options.h"
#include "box/BananasAndTokensBox.h"
#include "Menu_BaseCostumes.h"
#include "game/challenges/ChallengeMgr.h"
#include "game/gamemanagers/BackgroundMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamestates/GameState.h"
#include "game/gamestates/GS_MainMenu.h"
#include "game/interfaces/CostumeShopItem.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/popups/PopupInfo.h"
#include "game/interfaces/popups/PopupQuestion.h"
#include "game/online/store/Price.h"
#include "game/online/store/Store.h"
#include "game/Statistics.h"
#include "game/gamemanagers/BappleMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamestates/GS_Shop.h"
#include "game/GameLevel.h"
#include "game/Player.h"
#include "game/Game.h"
#include "camera/cameramgr.h"
#include "scene/camera.h"
#include "game/entities/LevelDef.h"
#include "game/entities/Deco3d.h"
#include "game/entities/Minion.h"
#include "game/entities/MinionCostume.h"
#include "game/interfaces/interfaceobjects/Interface3DTouchInteract.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceList.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "localization/formatter.h"
#include <ctime>

// OnlinePlayerData::CanLogin() @0x3ff154: the online player data is not ported, nobody can log in
static bool OnlinePlayerDataCanLogin() { return false; }

const jet::String Menu_MainMenu::k_menuName("MainMenu");

// the platform's saved "daily_login" time flag (the Java side's nativeIsTimeSet)
static bool s_dailyLoginTimeSet = false;

// the calendar day a time falls on
static int DayOf(long t) { return (int)(t / 86400); }

// @0x46794c
Menu_MainMenu::Menu_MainMenu()
    : Menu_BaseCostumes(k_menuName), m_mainMenuScrollList(0), m_challengeScrollList(0), m_touchesEnabled(true), m_scrollListShowing(true), m_enterToSocial(false),
      m_waiting(false), m_refreshRequested(false), m_equippedIndex(-1), m_enteringPlay(false), m_outroStarted(false), m_startPlay(false),
      m_scrollDelayer(0.0f, 0.0f, 1000), m_scrollShowDelay(200), m_scrollHideDelay(200), m_dailyDays(0), m_dailyDayFirst(0), m_dailyDayInWeek(0), m_dailyCurTime(0),
      m_dailyDone(false), m_dailyFrame(0), m_glass(0), m_getAward(0), m_dailyFrameLimit(0), m_dailyFrameTimer(0) {
    m_scrollDelayer.SetDelay(m_scrollShowDelay);
}

Menu_MainMenu::~Menu_MainMenu() { delete m_counters; }

void Menu_MainMenu::HideAll(const std::vector<InterfaceGraph*> &v) {
    for (size_t i = 0; i < v.size(); i++) v[i]->SetVisible(false);
}

// the calendar of the daily login: which days are done, the award button and the language's frames
void Menu_MainMenu::SetupDailyLogin() {
    Player *pl = Player::GetInstance();
    m_dailyCurTime = pl->GetDailyCurTime();
    m_dailyDays = pl->GetDailyDays();
    long now = (long)time(0);
    int dayA = DayOf(m_dailyCurTime), dayNext = DayOf(m_dailyCurTime + 0x15180), dayNow = DayOf(now);
    m_dailyDone = false;
    if (s_dailyLoginTimeSet) {
        if (now - m_dailyCurTime > 0x5460 || m_dailyCurTime - now > 0x5460) m_dailyDone = true;
        s_dailyLoginTimeSet = false;
        if (m_dailyDone) {
            pl->SetDailyCurTime((int)now);
            Game::GetInstance()->Save(false, false);  // @0x4841e8
        }
    }
    if (dayA == dayNow) m_dailyDone = true;
    else if (dayNow != dayNext) m_dailyDays = m_dailyDays < 7 ? 0 : 7;
    m_dailyDayFirst = (m_dailyDays - m_dailyDays % 7) + 1;
    m_dailyDayInWeek = (m_dailyDays - m_dailyDayFirst) + 1;

    // the frame and the "more award" picture by language
    int lang = (int)babel::Babel::GetInstance()->GetSelectedLanguage();
    int moreAward = 0x23, frame = 0;
    switch (lang) {
        case 0x6a61: moreAward = 0x28; frame = 5; break;
        case 0x6573: moreAward = 0x27; frame = 4; break;
        case 0x6272: case 0x7074: moreAward = 0x29; frame = 6; break;
        case 0x6465: moreAward = 0x25; frame = 2; break;
        case 0x6672: moreAward = 0x24; frame = 1; break;
        case 0x6974: moreAward = 0x26; frame = 3; break;
        case 0x7275: moreAward = 0x2c; frame = 9; break;
        case 0x6b6f: moreAward = 0x2a; frame = 7; break;
        case 0x7472: moreAward = 0x2d; frame = 10; break;
        case 0x7a68: moreAward = 0x2b; frame = 8; break;
    }
    m_dailyFrame = GetUIGraph(jet::String("DailyLogFrame"));
    m_dailyFrame->SetFrame(frame);
    GetUIGraph(jet::String("MoreAward"))->SetFrame(moreAward);
    m_glass = GetUIGraph(jet::String("glass"));
    m_glass->SetVisible(false);
    if (m_dailyDone) {
        m_dailyFrame->SetVisible(false);
        return;
    }
    static const char *k_finish[] = { "finish1", "finish2", "finish3", "finish4", "finish5", "finish6", "finish7" };
    static const char *k_day[] = { "day1", "day2", "day3", "day4", "day5", "day6", "day7" };
    static const char *k_dayBar[] = { "day_1", "day_2", "day_3", "day_4", "day_5", "day_6", "day_7" };
    static const char *k_banana[] = { "banana1", "banana2", "banana3", "banana4", "banana5", "banana6", "banana7" };
    static const char *k_xWhite[] = { "x1_white", "x2_white", "x300_white", "x500_white", "x750_white", "x900_white", "x4_white" };
    static const char *k_xGreen[] = { "x1_green", "x2_green", "x300_green", "x500_green", "x750_green", "x900_green", "x4_green" };
    static const char *k_x950W[] = { "x950_1_white", "x950_2_white", "x950_3_white", "x950_4_white", "x950_5_white", "x950_6_white", "x950_7_white" };
    static const char *k_x950G[] = { "x950_1_green", "x950_2_green", "x950_3_green", "x950_4_green", "x950_5_green", "x950_6_green", "x950_7_green" };
    static const char *k_token[] = { "token1", "token2", "token_week" };
    m_finish.clear(); m_banana.clear(); m_xWhite.clear(); m_xGreen.clear(); m_x950White.clear(); m_x950Green.clear(); m_token.clear(); m_dayText.clear(); m_dayBar.clear();
    for (int i = 0; i < 7; i++) {
        m_finish.push_back(GetUIGraph(jet::String(k_finish[i])));
        m_dayText.push_back(GetUIText(jet::String(k_day[i])));
        m_dayBar.push_back(GetUIPBar(jet::String(k_dayBar[i])));
        m_banana.push_back(GetUIGraph(jet::String(k_banana[i])));
        m_xWhite.push_back(GetUIGraph(jet::String(k_xWhite[i])));
        m_xGreen.push_back(GetUIGraph(jet::String(k_xGreen[i])));
        m_x950White.push_back(GetUIGraph(jet::String(k_x950W[i])));
        m_x950Green.push_back(GetUIGraph(jet::String(k_x950G[i])));
    }
    for (int i = 0; i < 3; i++) m_token.push_back(GetUIGraph(jet::String(k_token[i])));
    GetUIPBar(jet::String("day_singer"))->SetVisible(false);
    GetUIPBar(jet::String("day_vacationer"))->SetVisible(false);
    for (size_t i = 0; i + 1 < m_dayBar.size(); i++) m_dayBar[i]->SetVisible(false);
    m_dayBar[6]->SetVisible(false);
    HideAll(m_finish); HideAll(m_banana); HideAll(m_xWhite); HideAll(m_xGreen); HideAll(m_x950White); HideAll(m_x950Green); HideAll(m_token);
    for (size_t i = 0; i < m_finish.size(); i++) m_dailyFrameTimer = 0;
    for (size_t i = 0; i < m_dayText.size(); i++) {
        jet::String fmt = babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("DAYS_TRING"));
        m_dayText[i]->SetText(jet::String::Format(fmt.c_str(), (int)i + m_dailyDayFirst));
    }
    m_getAward = GetUIButton(jet::String("GetAward"));
    m_getAward->SetVisible(false);
    m_glass->SetVisible(true);
    for (int i = 0; i + m_dailyDayFirst <= m_dailyDays; i++) {
        m_finish[i]->SetVisible(true);
        m_dayBar[i]->SetValue(1.0f);
    }
    m_getAward->SetVisible(true);
    for (size_t i = 0; i + 1 < m_dayBar.size(); i++) m_dayBar[i]->SetVisible(true);
    if (m_dailyDayFirst == 1 || m_dailyDayFirst == 8) {
        const bool got = m_dailyDayFirst == 1 ? pl->IsGetFirstCostume() != 0 : pl->IsGetSecondCostume() != 0;
        if (!got) {
            m_dayBar[6] = GetUIPBar(jet::String(m_dailyDayFirst == 1 ? "day_singer" : "day_vacationer"));
            m_dayBar[6]->SetVisible(true);
            m_xGreen[6]->SetVisible(true);
        } else {
            m_token[2]->SetVisible(true);
            m_dayBar[6] = GetUIPBar(jet::String("day_7"));
            m_dayBar[6]->SetVisible(true);
            m_xWhite[6]->SetVisible(true);
        }
        m_token[0]->SetVisible(true);
        m_token[1]->SetVisible(true);
        for (int i = 2; i < 6; i++) m_banana[i]->SetVisible(true);
        for (int i = 0; i < 6; i++) (i < m_dailyDayInWeek ? m_xWhite[i] : m_xGreen[i])->SetVisible(true);
    } else {
        m_dayBar[6] = GetUIPBar(jet::String("day_7"));
        m_dayBar[6]->SetVisible(true);
        for (size_t i = 0; i < m_banana.size(); i++) m_banana[i]->SetVisible(true);
        for (int i = 0; i < 7; i++) (i < m_dailyDayInWeek ? m_x950White[i] : m_x950Green[i])->SetVisible(true);
    }
}

// @0x467cf4: find the elements, hide what a quiet day does not show, set up the daily login calendar.
// Not ported yet: the welcome screen.
void Menu_MainMenu::Init() {
    Menu_Base::Init();
    m_dailyFrameLimit = 50;
    m_dailyFrameTimer = 0;
    m_dailyAnimating = false;
    Menu_BaseCostumes::Init(jet::String("Costumes_Grid"), jet::String("Index_Grid"), jet::String("Selected_costume"), jet::String("Costumes_ScrollBar"));
    m_smallCostumeBG = GetUIGraph(jet::String("SmallCostume_BG"));
    m_smallCostumeBG->SetVisible(false);
    m_shopNotify = GetUIGraph(jet::String("Shop_NotifyIcon"));
    m_shopNotify->SetVisible(false);
    m_friendsNotify = GetUIGraph(jet::String("Friends_NotifyIcon"));
    m_friendsNotify->SetVisible(false);
    m_challengeScrollList = GetUIList(jet::String("ChallengeScroll_list"));
    m_mainMenuScrollList = GetUIList(jet::String("Scroll_list"));
    m_shopButton = GetUIButton(jet::String("shopCostume_Button"));
    m_goalsButton = GetUIButton(jet::String("goals_Button"));
    m_optionsButton = GetUIButton(jet::String("options_Button"));
    m_cheatsButton = GetUIButton(jet::String("EnableCheats"));
    m_glGamesButton = GetUIButton(jet::String("GLgames_Button"));
    m_cmccExitButton = GetUIButton(jet::String("CMCCexit_Button"));
    m_moreGameButton = GetUIButton(jet::String("Moregame_Button"));
    InterfaceGraph *limitBG = GetUIGraph(jet::String("daily_limit_time_bg"));
    GetUIText(jet::String("daily_limit_time"));
    math::vec2<float> p = limitBG->GetPos(), s = limitBG->GetSize();
    m_cmccExitButton->SetPos(math::vec2<float>(p.x + 5.0f, p.y + s.y));
    m_cmccExitButton->SetVisible(false);
    m_moreGameButton->SetVisible(false);
    m_cancelButton = GetUIButton(jet::String("CancelButton"));
    m_locationsButton = GetUIButton(jet::String("Locations_Button"));
    m_friendsButton = GetUIButton(jet::String("Friends_Button"));
    m_menuButton = GetUIButton(jet::String("Menu_Button"));
    m_arrowR = GetUIButton(jet::String("Arrow_R"));
    m_arrowL = GetUIButton(jet::String("Arrow_L"));
    m_multiplierButton = GetUIButton(jet::String("Multiplier_button"));
    m_bapplesButton = GetUIButton(jet::String("Bapples_button"));
    m_bapplesButton->SetVisible(false);
    m_multiplierBonus = GetUIGraph(jet::String("Multiplier_bonus"));
    m_multiplierBonusValue = GetUIText(jet::String("Multiplier_bonus_value"));
    m_menuBG = GetUIObject(jet::String("Menu_BG"));
    m_menuBGOffset = m_menuBG->GetOffset();
    m_counters = new BananasAndTokensBox(GetUIObject(jet::String("BananasAndTokensCounterPos")), true);
    InterfaceButton *dbgLast = GetUIButton(jet::String("CostumeDebugLast")), *dbgNext = GetUIButton(jet::String("CostumeDebugNext"));
    m_waitingBg = GetUIObject(jet::String("WaitingBg"));
    dbgLast->SetVisible(false);
    dbgNext->SetVisible(false);
    m_touchesEnabled = false;
    ShowMainMenuScrollList(true);
    InterfaceButton *login = GetUIButton(jet::String("GS_Login")), *plus = GetUIButton(jet::String("GS_Plus"));
    login->SetVisible(false);
    plus->SetVisible(false);
    SetupDailyLogin();
    Player *pl = Player::GetInstance();
    if (pl->IsMigicCostume() == 0) {
        pl->SetIsMigicCostume(2);
        pl->SetMagicCostumeTime((int)time(0));
    }
}

// @0xd0b8ed: set while the main menu is the one shown (OnEnter / OnQuit)
static bool s_mainMenuActive = false;

// the time left as the promotion countdowns write it: "hh:mm:ss", with the days in front when there are some (the same
// text RefreshCostumePromo, _dailylogin and _magician build, @0x4892a4 @0x489678 @0x489d90)
static jet::String FormatTimeLeft(int seconds) {
    const int sec = seconds % 60, min = (seconds / 60) % 60, hr = (seconds / 3600) % 24, days = seconds / 86400;
    if (days == 0) return jet::String::Format("%02d:%02d:%02d", hr, min, sec);
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    return jet::String::Format("%d %s %02d:%02d:%02d", days, sm->Get(jet::String(days == 1 ? "STORE_DAY_LEFT" : "STORE_DAYS_LEFT")).c_str(), hr, min, sec);
}

// @0x4861a4: the lab as it was left: the counters, the notices, the costumes of the wheel in the place of the lab's minion.
// Not ported: the bapple hunt (never unlocked here), the minion's state label and PopupGoToLocation (when the background is not
// the last active location),
// COPPAMgr and the cheat detection popup, Game::SaveSessionData, the IAP token list, the version update check.
void Menu_MainMenu::OnEnter() {
    GetUIGraph(jet::String("daily_limit_time_bg"))->SetVisible(false);
    s_mainMenuActive = true;
    m_cheatsButton->SetVisible(true);
    // the missions the player has are looked at again (@0x486234): the ones that count over runs may be done by now
    MissionMgr::GetInstance()->RefreshGameMissions();
    GetUIText(jet::String("Best_score_value"))->SetText(babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(Statistics::GetInstance()->GetBestScore()));
    // BappleMgr::IsBappleHuntUnlocked is never true here: the button shows the daily menu's text (the hunt's day counter otherwise)
    m_bapplesButton->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("BAPPLES_DAILYMENU_BAPPLES")));
    m_challengeScrollList->SetVisible(false);
    UpdateChallengeScrollVisibility();
    RefreshCostumePromo();
    RefreshCostumePromo_dailylogin();
    RefreshMarks();
    // the camera goes where the level put it, with the design's horizontal fov, before the costumes are placed by it
    CameraMgr::GetInstance()->Update(99999);
    jet::scene::Camera *cam = CameraMgr::GetInstance()->GetSceneCamera();
    cam->SetHorizontalFov(Game::GetInstance()->GetHFovFor(cam->GetFov(), true));
    InitCostumes(jet::String("BigCostume_feet_pos"), jet::String("BigCostume_next_feet_pos"), jet::String("SmallCostume_feet_pos"),
                 jet::String("SmallCostume_next_feet_pos"));
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    minion->RegisterForRender(false);
    minion->RegisterForUpdate(false);
    minion->GetDeco()->RegisterForUpdate(false);
    minion->GetDeco()->RegisterForRender(false);
    if (!m_dailyDone)
        if (MinionCostume *c = GetCurrentCostume()) {
            c->RegisterForRender(false);
            c->RegisterForUpdate(false);
        }
    // @0x486434
    Player::GetInstance()->ComputeLocationUnlock();
    EnableTouches(true);
}

// @0x486954
void Menu_MainMenu::OnQuit() {
    s_mainMenuActive = false;
    HidePlusButton();
    ClearCostumes();
}

// @0x486994: the location the player runs in is the one of the background (what the cancel of the download of a location does)
void Menu_MainMenu::OnLocationDownloadContentCancelCallback() { Player::GetInstance()->SetCurrentLocation(BackgroundMgr::GetInstance()->GetLocationOfCurrentBackground()); }

// @0x4869d8: the exit popup's "yes" leaves the game (Game::QuitGame @0x52b9f0 is not ported: the host closes the window)
void Menu_MainMenu::OnPopupYes(void*) {}
// @0x4869f8
void Menu_MainMenu::OnPopupNo(void*) {}

// @0x488fc8: the notifications of the device (the Android native SimplifiedPN_IsNotificationPermissionEnabled and
// GameSettings::SetLocalNotificationsEnabled are not ported), the notices, the promotion, and the costumes after a popup
void Menu_MainMenu::Refresh() {
    m_refreshRequested = false;
    RefreshMarks();
    RefreshCostumePromo();
    Player::GetInstance()->ComputeLocationUnlock();  // @0x489054
    if (m_refreshAfterPopup) {
        RefreshCostumes();
        m_refreshAfterPopup = false;
    }
}

// @0x489098: the red marks: the friends' one asks the weekly contest (WeeklyChallengeMgr and TutorialMgr's weekly dialogs are
// not ported, so it stays hidden), the options' and the goals' ones ask their menus
void Menu_MainMenu::RefreshMarks() {
    MenuMgr *mm = MenuMgr::GetInstance();
    Menu_Base *options = mm->GetMenu(Menu_Options::k_menuName);
    GetUIObject(jet::String("Options_NotifyIcon"))->SetVisible(static_cast<Menu_Options*>(options)->AnyTabWithNotification());
    Menu_Base *goals = mm->GetMenu(Menu_Goals::k_menuName);
    GetUIObject(jet::String("Goals_NotifyIcon"))->SetVisible(static_cast<Menu_Goals*>(goals)->AnyTabWithNotification());
}

// @0x4892a4: the countdown of the promotion of the costumes
void Menu_MainMenu::RefreshCostumePromo() {
    bool show = false;
    if (!IsScrollingCostumes()) {
        Store *store = Store::GetInstance();
        jet::String costumes("Costumes");
        const bool promo = store->HasPromotionOnCategory(costumes);
        const bool iap = store->HasIAPPromotionOnCategory(costumes);
        if (iap || promo) {
            show = true;
            InterfaceText *text = GetUIText(jet::String("Time_Costumes"));
            const int left = store->GetDynamicPricingPromotionRemainingTime();
            if (left > 0) {
                jet::String timeStr = FormatTimeLeft(left);
                const char *key = "SHOP_PROMO_ENDS_IN";
                if (!m_costumes.empty() && m_currentCostume >= 0) {
                    MinionCostume *c = m_costumes[(size_t)m_currentCostume];
                    const bool pricePromo = c->GetPrice()->HasPricePromotion();
                    const bool timeLimited = c->IsTimeLimited();
                    if (!pricePromo && !timeLimited) show = false;
                    else if (CostumeMgr::GetInstance()->WasBought(*c)) show = false;
                    else if (timeLimited && !pricePromo) key = "AVAILABLE_FOR";
                }
                text->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)).c_str(), timeStr.c_str()));
            } else
                show = false;
        }
    }
    GetUIGraph(jet::String("Time_Costumes_BG"))->SetVisible(show);
}

// @0x489678: the countdown of the awarded costumes: 24 hours of the day's award (the award is 1), then the 2 hours of its
// expiry (the award is 2)
void Menu_MainMenu::RefreshCostumePromo_dailylogin() {
    Player *pl = Player::GetInstance();
    bool first = false, second = false;
    if (!IsScrollingCostumes()) {
        first = pl->IsGetFirstCostume() == 1;
        second = pl->IsGetSecondCostume() == 1;
    }
    bool show = false;
    if (first || second) {
        InterfaceText *text = GetUIText(jet::String("daily_limit_time"));
        int left = 0;
        if (first) left = pl->GetAwardCostumeTime_1() + 0x15180 - (int)time(0);
        if (second) left = pl->GetAwardCostumeTime_2() + 0x15180 - (int)time(0);
        if (left > 0) {
            jet::String timeStr = FormatTimeLeft(left);
            if (!m_costumes.empty() && m_currentCostume >= 0) {
                const int id = m_costumes[(size_t)m_currentCostume]->GetCostumeId();
                show = id == 0x12 ? first : id == 0x10 ? second : false;
            } else
                show = true;
            text->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("DAILYLOG_TIME_LIMIT")).c_str(), timeStr.c_str()));
        }
    }
    GetUIGraph(jet::String("daily_limit_time_bg"))->SetVisible(show);
    if (show) return;
    if (!IsScrollingCostumes()) {
        first = pl->IsGetFirstCostume() == 2;
        second = pl->IsGetSecondCostume() == 2;
    }
    if (!(first || second)) return;
    InterfaceText *text = GetUIText(jet::String("Time_Costumes"));
    int left = 0;
    if (first) left = pl->GetAwardCostumeTime_1() + 0x1c20 - (int)time(0);
    if (second) left = pl->GetAwardCostumeTime_2() + 0x1c20 - (int)time(0);
    bool showBG = false;
    if (left > 0) {
        jet::String timeStr = FormatTimeLeft(left);
        if (!m_costumes.empty() && m_currentCostume >= 0) {
            const int id = m_costumes[(size_t)m_currentCostume]->GetCostumeId();
            showBG = id == 0x12 ? first : id == 0x10 ? second : false;
        } else
            showBG = true;
        text->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("AVAILABLE_FOR")).c_str(), timeStr.c_str()));
    }
    GetUIGraph(jet::String("Time_Costumes_BG"))->SetVisible(showBG);
}

// @0x489d90: the countdown of the magician's costume (3 days)
void Menu_MainMenu::RefreshCostumePromo_magician() {
    bool show = false;
    if (!IsScrollingCostumes() && Player::GetInstance()->IsMigicCostume() == 2) {
        InterfaceText *text = GetUIText(jet::String("Time_Costumes"));
        const int left = Player::GetInstance()->GetMagicCostumeTime() + 0x3f480 - (int)time(0);
        if (left > 0) {
            jet::String timeStr = FormatTimeLeft(left);
            show = (!m_costumes.empty() && m_currentCostume >= 0) ? m_costumes[(size_t)m_currentCostume]->GetCostumeId() == 0x19 : true;
            text->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("AVAILABLE_FOR")).c_str(), timeStr.c_str()));
        }
    }
    GetUIGraph(jet::String("Time_Costumes_BG"))->SetVisible(show);
}

// @0x46f70c (0x48a3b8): the award of the day. The 7th and the 14th are a costume (a popup tells), once; the other days
// bananas, the more the day of the week is far; after the 14th every day is the same. The calendar shows it
void Menu_MainMenu::GetDailyAward() {
    Player *pl = Player::GetInstance();
    m_dailyDays++;
    m_dailyCurTime = (int)time(0);
    // Date::SetToNow / SetTime of the calendar's date (the tracking's)
    const int day = m_dailyDays % 7;
    int bananas = 0x1f40;
    switch (day) {
        case 0:
            if (m_dailyDays == 7 && pl->IsGetFirstCostume() == 0) {
                // CostumeMgr::GetCostumeById(0x18)
                pl->SetGetFirstCostume(1);
                // PopupMgr::PushPopup(PopupYCJgame(1)) (not ported)
                pl->SetAwardCostumeTime_1(m_dailyCurTime);
                bananas = 0;
            } else if (m_dailyDays == 14 && pl->IsGetSecondCostume() == 0) {
                // CostumeMgr::GetCostumeById(0x13)
                pl->SetGetSecondCostume(1);
                // PopupMgr::PushPopup(PopupYCJgame(1)) (not ported)
                pl->SetAwardCostumeTime_2(m_dailyCurTime);
                bananas = 0;
            }
            break;
        case 1: bananas = 1000; break;
        case 2: bananas = 1500; break;
        case 3: bananas = 2000; break;
        case 4: bananas = 3000; break;
        case 5: bananas = 4000; break;
        case 6: bananas = 6000; break;
    }
    if (bananas) pl->AddBananas(bananas);
    pl->SetDailyCurTime(m_dailyCurTime);
    pl->SetDailyDays(m_dailyDays);
    Game::GetInstance()->Save(false, false);
    m_dailyAnimating = true;
    m_finish[(size_t)m_dailyDayInWeek]->SetVisible(true);
    if (m_dailyDays > 14) {
        m_x950White[(size_t)m_dailyDayInWeek]->SetVisible(true);
        m_x950Green[(size_t)m_dailyDayInWeek]->SetVisible(false);
    } else if (m_dailyDays == 14 || m_dailyDays == 7) {
        if (pl->IsGetFirstCostume()) {
            m_xWhite[(size_t)m_dailyDayInWeek]->SetVisible(true);
            m_xGreen[(size_t)m_dailyDayInWeek]->SetVisible(false);
        }
        if (pl->IsGetSecondCostume()) {
            m_xWhite[(size_t)m_dailyDayInWeek]->SetVisible(true);
            m_xGreen[(size_t)m_dailyDayInWeek]->SetVisible(false);
        }
    } else {
        m_xWhite[(size_t)m_dailyDayInWeek]->SetVisible(true);
        m_xGreen[(size_t)m_dailyDayInWeek]->SetVisible(false);
    }
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);
}

// @0x4869fc (first part): the awarded costumes' timers. A costume given for a day (1) is taken away a day later and gives
// two hours to buy it (2), then it is gone (-1); the magician's costume lasts three days
void Menu_MainMenu::UpdateAwardsTimers() {
    Player *pl = Player::GetInstance();
    if (MinionCostume *c = GetCurrentCostume()) {
        c->RegisterForRender(true);
        c->RegisterForUpdate(true);
    }
    m_glass->SetVisible(false);
    const int now = (int)time(0);
    if (pl->IsGetSecondCostume() == 1 && now - pl->GetAwardCostumeTime_2() > 0x15180) {
        CostumeMgr::GetInstance()->RemoveCostumeById(0x13);
        pl->SetGetSecondCostume(2);
        pl->SetAwardCostumeTime_2(now);
        Game::GetInstance()->Save(false, false);
        ShowMainMenuScrollList(false);
        m_item->Expand(true, true);
        ReloadCostumes();
        OnEnter();
        PopupMgr::GetInstance()->PushPopup(new PopupInfo(PopupInfo::Info, std::string(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("COSTUME__EXPIRED")).c_str()), 100));
        Game::GoToShop = 0;
    }
    if (pl->IsGetFirstCostume() == 1 && now - pl->GetAwardCostumeTime_1() > 0x15180) {
        CostumeMgr::GetInstance()->RemoveCostumeById(0x18);
        pl->SetGetFirstCostume(2);
        pl->SetAwardCostumeTime_1(now);
        Game::GetInstance()->Save(false, false);
        ShowMainMenuScrollList(false);
        m_item->Expand(true, true);
        ReloadCostumes();
        OnEnter();
        PopupMgr::GetInstance()->PushPopup(new PopupInfo(PopupInfo::Info, std::string(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("COSTUME__EXPIRED")).c_str()), 100));
        Game::GoToShop = 0;
    }
    if (pl->IsGetSecondCostume() == 2 && now - pl->GetAwardCostumeTime_2() > 0x1c20) {
        pl->SetGetSecondCostume(-1);
        Game::GetInstance()->Save(false, false);
    }
    if (pl->IsGetFirstCostume() == 2 && now - pl->GetAwardCostumeTime_1() > 0x1c20) {
        pl->SetGetFirstCostume(-1);
        Game::GetInstance()->Save(false, false);
    }
    if (pl->IsMigicCostume() == 2 && now - pl->GetMagicCostumeTime() > 0x3f480) {
        pl->SetIsMigicCostume(-1);
        Game::GetInstance()->Save(false, false);
    }
    // the popup of the expired costume asked for the shop: it opens on its costumes
    if (Game::GoToShop == 1) {
        Game::GoToShop = 2;
        GS_Shop::checkCallShop = true;
        StopMainMenuSfx();
        GameState::PushState(new GS_Shop(), true);
    }
}

// @0x4869fc: the main menu's frame. The calendar of the day's award first, the back key (the exit popup, or the end of the
// wait for the friends), the list of the buttons that slides in and out with its clip, the arrows, the multiplier, and, as the
// costumes are on, either the exit of the costumes and the minion to the cannon after the play, or the buttons released.
// Not ported: COPPAMgr::Update, PopupGoToLocation (the background is not the last active location), the bapple hunt's button
// text (never unlocked here), Player::ComputeLocationUnlock, the weekly contest's data, OnlinePlayerData::CanLogin (the online
// services are never ready here), the IGP reward, the sounds.
void Menu_MainMenu::Update(int dt) {
    GetUIGraph(jet::String("daily_limit_time_bg"))->SetVisible(false);
    MenuMgr *mm = MenuMgr::GetInstance();
    PopupMgr *popups = PopupMgr::GetInstance();
    if (m_dailyDone) {
        UpdateAwardsTimers();
    } else if (m_dailyAnimating) {
        m_dayBar[(size_t)m_dailyDayInWeek]->SetValue((float)m_dailyFrameTimer / (float)m_dailyFrameLimit);
        if (++m_dailyFrameTimer >= m_dailyFrameLimit) {
            m_dailyDone = true;
            m_dailyFrame->SetVisible(false);
            Player *pl = Player::GetInstance();
            if (pl->IsGetFirstCostume() == 1 || pl->IsGetSecondCostume() == 1) {
                OnNextEnterSelectEquippedCostume();
                OnEnter();
                ShowMainMenuScrollList(false);
                m_item->Expand(true, true);
            }
        }
    }
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        if (m_waiting) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            m_waiting = false;
            EnableTouches(true);
            m_cancelButton->SetActive(false);
            return;
        }
        // KeyEventManager::ClearKeyEvents
        jet::String text = babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("EXIT_POPUP"));
        HidePlusButton();
        popups->PushPopup(new PopupQuestion(text, this, &Menu_MainMenu::OnPopupYes, &Menu_MainMenu::OnPopupNo));
        return;
    }
    if (!m_startPlay && m_refreshRequested) Refresh();
    // @0x487020: the background is not the location the player was in: PopupGoToLocation(last active location, false, cancel callback)
    // when no popup is up, and the last active location becomes the background's
    if (!popups->GetCurrentPopup() && BackgroundMgr::GetInstance()->GetLocationOfCurrentBackground() != Player::GetInstance()->GetCurrentLocation()) {
        popups->PushPopup(new PopupGoToLocation(Player::GetInstance()->GetCurrentLocation(), false, [this] { OnLocationDownloadContentCancelCallback(); }));
        Player::GetInstance()->SetCurrentLocation(BackgroundMgr::GetInstance()->GetLocationOfCurrentBackground());
    }
    // the list of the buttons slides: the clip is the menu's background where it is now
    const bool sliding = m_scrollDelayer.Update(dt);
    m_mainMenuScrollList->SetActive(!sliding);
    const math::vec2<float> slide(0.0f, m_scrollDelayer.GetCurrentValue());
    m_mainMenuScrollList->SetOffset(slide);
    m_menuBG->SetOffset(math::vec2<float>(slide.x + m_menuBGOffset.x, slide.y + m_menuBGOffset.y));
    const math::vec2<float> size = m_menuBG->GetSize();
    const math::vec2<float> pos = m_menuBG->GetPos();
    const float left = pos.x - slide.x - m_menuBGOffset.x, top = pos.y - slide.y - m_menuBGOffset.y;
    const float right = size.x + pos.x - slide.x, bottom = size.y + pos.y - slide.y;
    const math::rect<float> clip(left, top, right, bottom);
    m_mainMenuScrollList->SetClip(clip);
    m_menuBG->SetClip(clip);
    SetCostumeGUIVisible(!m_scrollListShowing);
    if (IsScrollingCostumes()) ShowMainMenuScrollList(false);
    EnableTouches(IsScrollingCostumes() ? false : m_enteringPlay ? false : !m_outroStarted ? !m_waiting : m_enteringPlay);
    m_arrowL->SetVisible(CanScrollCostumes(-1));
    m_arrowR->SetVisible(CanScrollCostumes(1));
    // the locations the missions have opened are unlocked (@0x487314); the hunt is never unlocked, the button's text is the daily menu's
    Player::GetInstance()->ComputeLocationUnlock();
    BappleMgr::GetInstance()->ComputeBappleHuntUnlock();
    m_bapplesButton->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("BAPPLES_DAILYMENU_BAPPLES")));
    // the bonus the current costume adds to the multiplier
    if (!m_currentWasBought && m_currentCostume >= 0 && !m_costumes.empty()) {
        const int factor = m_costumes[(size_t)m_currentCostume]->GetExtraDespicablePointsFactorForGlobal();
        m_multiplierBonus->SetVisible(factor != 0);
        if (factor != 0) m_multiplierBonusValue->SetText(jet::String::Format("+%d", factor));
    } else
        m_multiplierBonus->SetVisible(false);
    RefreshCostumePromo();
    RefreshCostumePromo_dailylogin();
    RefreshCostumePromo_magician();
    if (m_enteringPlay || m_startPlay) {
        UpdateEnteringGamePlay(dt);
        return;
    }
    GetUIText(jet::String("Multiplier_value"))->SetText(jet::String::Format("x%d", Statistics::GetInstance()->GetBaseScoreMultiplier()));
    m_waitingBg->SetVisible(m_waiting);
    if (m_waiting) {
        Set3DTouchesDisableTimer(100);
        if (!Game::GetInstance()->AreOnlineServicesReady()) {
            m_waiting = false;
            popups->PushInfoPopup(std::string(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("POPUP_NO_INTERNET")).c_str()));
            return;
        }
        // OnlinePlayerData::CanLogin: StopMainMenuSfx, MenuMgr::PushMenu(Menu_Social::k_menuName) and the end of the wait (the online
        // services are never ready here, Menu_Social is not ported)
    } else if (!(m_challengeScrollList->IsActive() && m_challengeScrollList->IsDragButtonWasPressed())) {
        if (m_dailyDone) UpdateCostumes(dt, true, m_outroStarted);
    }
    // the weekly challenge's data is read (WeeklyChallengeMgr is not ported); the shop's notice shows when the store has promotions
    m_shopNotify->SetVisible(Store::GetInstance()->HasAnyPromoInShop() || Store::GetInstance()->HasAnyPricePromotionOnCategory(jet::String("Tokens")));
    UpdateChallengeScrollVisibility();
    Menu_Base::Update(dt);
    UpdateReleasedButtons(dt);
    // the IGP reward (isIGPReward: tokens and a popup for what the Gameloft games' portal gave) is not ported
}

// @0x4869fc (the released buttons): each button of the frame, the first one that is answered ends the loop
void Menu_MainMenu::UpdateReleasedButtons(int) {
    MenuMgr *mm = MenuMgr::GetInstance();
    InterfaceButton *scrollDrag = m_mainMenuScrollList->GetDragButton(), *challengeDrag = m_challengeScrollList->GetDragButton();
    for (int i = 0; i < GetNumReleasedButtons(); i++) {
        InterfaceButton *b = GetReleasedButton(i);
        Set3DTouchesDisableTimer(100);
        if (m_waiting) {
            m_cancelButton->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("CANCEL")));
            if (b != m_cancelButton) continue;
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            m_waiting = false;
            m_cancelButton->SetActive(false);
            break;
        }
        if (!m_dailyDone) {
            if (b == m_getAward && !m_dailyAnimating) GetDailyAward();
            continue;
        }
        if (b == m_menuButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            ShowMainMenuScrollList(!m_scrollListShowing);
            break;
        }
        // a button of the challenge list that was pressed is the list's
        if (m_challengeScrollList->IsActive() && m_challengeScrollList->IsDragButtonWasPressed()) break;
        if (b == m_arrowR) {
            ScrollCostumes(1);
            break;
        }
        if (b == m_arrowL) {
            ScrollCostumes(-1);
            break;
        }
        if (b == m_shopButton) {
            GS_Shop::checkCallShop = true;
            StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            GameState::PushState(new GS_Shop(), true);
            break;
        }
        if (b == m_goalsButton || b == m_multiplierButton) {
            StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            static_cast<Menu_Goals*>(mm->GetMenu(Menu_Goals::k_menuName))->SetEnterState(Menu_Goals::Goals_Missions);
            mm->PushMenu(Menu_Goals::k_menuName);
            break;
        }
        if (b == m_bapplesButton) {
            StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // BappleMgr::IsBappleHuntUnlocked is never true here: PopupMgr::PushPopup(PopupBapplesInfo(false)) (not ported); otherwise
            // Menu_Goals::SetEnterState(4) and the push of the goals
            break;
        }
        if (b == m_friendsButton) {
            StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // @0x487b1c: the social menu opens when the online services are up and the player can log in; else the menu waits
            // for the login. OnlinePlayerData::CanLogin() is not ported: nobody can
            if (Game::GetInstance()->AreOnlineServicesReady() && OnlinePlayerDataCanLogin()) {
                mm->PushMenu(Menu_Social::k_menuName);
                break;
            }
            m_waiting = true;
            m_cancelButton->SetActive(true);
            break;
        }
        if (b == m_locationsButton) {
            StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // @0x487b7c: the goals open at their locations tab
            static_cast<Menu_Goals*>(mm->GetMenu(Menu_Goals::k_menuName))->SetEnterState(Menu_Goals::Goals_Locations);
            mm->PushMenu(Menu_Goals::k_menuName);
            break;
        }
        if (b == m_optionsButton) {
            if (m_scrollListShowing) StopMainMenuSfx();
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            mm->PushMenu(Menu_Options::k_menuName);
            break;
        }
        if (b == m_cheatsButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);  // and CheatsMgr::UI_Show() (@0x59554c, not ported)
            break;
        }
        if (b == scrollDrag) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            m_challengeScrollList->CloseIfDragListOpen();
            break;
        }
        if (b == challengeDrag) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            m_mainMenuScrollList->CloseIfDragListOpen();
            break;
        }
        if (b == m_glGamesButton) {
            // MenuMgr::DisableTouchesFor(k_disableTouchesTime), pressIGP and the Android native nativelaunchIGP (not ported)
            break;
        }
    }
}

// @0x4869fc (the play): the costumes leave, the arrows slide out, the costume of the equipped goes to the middle, and the
// minion comes back for the cannon's movie
void Menu_MainMenu::UpdateEnteringGamePlay(int dt) {
    m_waitingBg->SetVisible(false);
    if (m_startPlay) return;
    UpdateCostumes(dt, true, m_outroStarted);
    m_scrollBar->SetOpacity(0.0f);
    m_item->SetTransition(false);
    m_item->Expand(false, false);
    const float slide = m_arrowL->GetPos().x * 0.01f * (float)dt;
    math::vec2<float> o = m_arrowL->GetOffset();
    m_arrowL->SetOffset(math::vec2<float>(o.x - slide, o.y));
    o = m_arrowR->GetOffset();
    m_arrowR->SetOffset(math::vec2<float>(o.x + slide, o.y));
    const float t = Game::GetInstance()->GetUpdateTimeSeconds();
    for (size_t i = 0; i < m_smallCostumes.size(); i++) {
        math::vec3<float> p = m_smallCostumes[i]->GetPosition();
        p.z -= 2.0f * MinionCostume::k_defaultMoveSpeed * t;
        m_smallCostumes[i]->SetPosition(p);
    }
    math::vec2<float> screen(0.0f, 0.0f);
    CameraMgr::GetInstance()->GetSceneCamera()->Project(screen, m_smallCostumes[(size_t)m_currentCostume]->GetPosition());
    const bool far = screen.y >= Game::GetInstance()->GetDesignWidthScale();
    if (m_costumes[(size_t)m_equippedIndex]->HasReachedTargetPos() && far) {
        MenuMgr *mm = MenuMgr::GetInstance();
        mm->PopMenu();
        Minion *minion = GameLevel::GetInstance()->GetMinion();
        minion->RegisterForRender(true);
        minion->RegisterForUpdate(true);
        minion->GetDeco()->RegisterForUpdate(true);
        minion->GetDeco()->RegisterForRender(true);
        mm->Movie_StartMovie(LevelDef::GetInstance()->GetMovieFromParam(jet::String("Ref_UIMovie_MMToCannon")));
        m_startPlay = true;
    }
}

// @0x4887a0: the list of the challenges of the friends shows when one is active (the online friends' challenges: only the
// list's visibility is ported, ChallengeMgr has no online user here, and Challenge / FriendsMgr / the 3D costume of the
// friend are not ported)
void Menu_MainMenu::UpdateChallengeScrollVisibility() {
    ChallengeMgr *cm = ChallengeMgr::GetInstance();
    cm->Challenge_IsActive();
    if (m_challengeScrollList->IsVisible()) return;
    _ClearChallengeData();
    // ChallengeMgr::CheckStartCurrentChallenge (@0x591160) is not ported: no challenge is ever active here
    const bool active = cm->Challenge_IsActive();
    // the list shows for an active challenge of a named user (ChallengeMgr::OnlineUser_GetName is not ported)
    m_challengeScrollList->SetVisible(active);
}

// @0x488d10
void Menu_MainMenu::_ClearChallengeData() {}

void Menu_MainMenu::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    m_smallCostumeBG->SetVisible(true);
    m_smallCostumeBG->Render(cam, p);
    m_smallCostumeBG->SetVisible(false);
    RenderCostumes(cam, p, true);
    Menu_Base::Render(cam, p);
    if (m_counters) m_counters->Render(cam, p);
}

// @0x4883bc
void Menu_MainMenu::Reset() {
    m_startPlay = false;
    m_enteringPlay = false;
    m_outroStarted = false;
    m_arrowL->SetOffset(math::vec2<float>(0, 0));
    m_arrowR->SetOffset(math::vec2<float>(0, 0));
}

// @0x48840c: event 1 (the minion tapped to play) starts the game when the costume the player has can be loaded, or asks
// the options for the content; 3 opens the shop
void Menu_MainMenu::ExecuteTouchInteractEvent(int event) {
    if (event == 1) {
        MinionCostume *equipped = CostumeMgr::GetInstance()->GetEquippedCostumeRef();
        if (equipped->CanLoadModel() && equipped->IsShopElement()) {
            GameLevel::GetInstance()->GetMinion()->ReloadCostumeIfNeeded();
            StartEnteringGamePlay();
            return;
        }
        // Game +0x26c = 0, MenuMgr::PushMenu(Menu_Options::k_menuName) and Menu_Options::RequestCheckForContent (@0x48c320, not ported yet)
        MenuMgr::GetInstance()->PushMenu(Menu_Options::k_menuName);
    } else if (event == 3) {
        StopMainMenuSfx();
        GameState::PushState(new GS_Shop(), true);
    }
}

// @0x4885dc: the play: the costume of the equipped is found in the wheel (the first one is equipped when it is none) and the
// one the wheel stands at gives it its place; the small ones can no more walk to their places
void Menu_MainMenu::StartEnteringGamePlay() {
    m_equippedIndex = -1;
    for (size_t i = 0; i < m_costumes.size(); i++)
        if (m_costumes[i]->IsCurrentlyEquipped()) {
            m_equippedIndex = (int)i;
            break;
        }
    if (m_equippedIndex == -1) {
        CostumeMgr::GetInstance()->SetEquippedCostume(m_costumes[0], false);
        m_equippedIndex = 0;
        m_currentCostume = 0;
    }
    if (m_equippedIndex != m_currentCostume) {
        MinionCostume *current = m_costumes[(size_t)m_currentCostume], *equipped = m_costumes[(size_t)m_equippedIndex];
        if (!current || !equipped) return;  // "StartEnteringGamePlay NULL - call dev!"
        const math::vec3<float> at = current->GetTargetPosition();
        float spacing = m_bigSpacing;
        if (m_currentCostume >= m_equippedIndex) spacing = -spacing;
        current->SetTargetPosition(math::vec3<float>(at.x, at.y + spacing, at.z));
        equipped->SetTargetPosition(math::vec3<float>(at.x, m_bigPos.y, at.z));
        spacing = m_bigSpacing;
        if (m_currentCostume < m_equippedIndex) spacing = -spacing;
        equipped->SetPosition(math::vec3<float>(at.x, spacing + m_bigPos.y, at.z));
    }
    for (size_t i = 0; i < m_smallCostumes.size(); i++) m_smallCostumes[i]->SetEnableReachTarget(false);
    m_item->SetTransition(false);
    OnNextEnterSelectEquippedCostume();
    m_enteringPlay = true;
}

// @0x48850c
void Menu_MainMenu::TouchInteract_StartingOutroAnim(int) {
    EnableTouches(false);
    m_outroStarted = true;
}

void Menu_MainMenu::TouchInteract_Reset3dEventInterface(int event) {
    if (event != 1) return;
    GetUIObject(jet::String("TAPPLAY3DModel"))->Reset();
}

// @0x488754: the level's Ref_UIMovie_MMInit movie
void Menu_MainMenu::StartMainMenuIntroMovie() {
    MenuMgr::GetInstance()->Movie_StartMovie(LevelDef::GetInstance()->GetMovieFromParam(jet::String("Ref_UIMovie_MMInit")));
}

// @0x488d38: the buttons and the lists' items take or leave touches
void Menu_MainMenu::EnableTouches(bool on) {
    m_touchesEnabled = on;
    InterfaceButton *bs[] = { m_shopButton, m_goalsButton, m_optionsButton, m_cheatsButton, m_glGamesButton, m_locationsButton, m_menuButton, m_bapplesButton, m_arrowR, m_arrowL, m_multiplierButton };
    for (size_t i = 0; i < sizeof(bs) / sizeof(bs[0]); i++) bs[i]->SetActive(on);
    for (unsigned i = 0; i < m_mainMenuScrollList->GetChildCount(); i++) m_mainMenuScrollList->GetChild((int)i)->SetActive(on);
    for (unsigned i = 0; i < m_challengeScrollList->GetChildCount(); i++) m_challengeScrollList->GetChild((int)i)->SetActive(on);
}

// @0x488ed0: the sounds of the main menu state (its ambience and the sfx) stop
void Menu_MainMenu::StopMainMenuSfx() {
    GameState *cur = GameState::GetCrtState();
    if (cur && cur->GetStateName().Equals(GS_MainMenu::k_stateName)) static_cast<GS_MainMenu*>(cur)->StopMainMenuSfx();
}

// @0x488f1c: the list of the buttons slides in (0) or out (up by the height of the background); the menu button's animations go with it
void Menu_MainMenu::ShowMainMenuScrollList(bool show) {
    m_scrollListShowing = show;
    if (!show) {
        math::vec2<float> sz = m_menuBG->GetSize();
        m_scrollDelayer.SetTargetValue(-sz.y);
        m_scrollDelayer.SetDelay(m_scrollHideDelay);
        m_menuButton->SetIdleAnim(0x80);
        m_menuButton->SetPressedAnim(0x81);
    } else {
        m_scrollDelayer.SetTargetValue(0.0f);
        m_scrollDelayer.SetDelay(m_scrollShowDelay);
        m_menuButton->SetIdleAnim(0xa2);
        m_menuButton->SetPressedAnim(0xa1);
    }
}
