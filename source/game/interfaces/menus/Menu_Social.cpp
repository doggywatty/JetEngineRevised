// source/game/interfaces/menus/Menu_Social.cpp
//
// The online services are not ported (see Menu_Social.h); what the .so asks of them is answered here by what an offline
// player gets: nobody is logged in, no leaderboard is loading or loaded, there is no contest of the week and no reward of the
// network. Not ported: the login and the invitations (PopupLoginToFacebook, PopupChooseSNS, FriendsMgr), the prizes of the week
// (PopupWeeklyPrizes), the dialogs of the tutorial of the contest, the random users shown when there are no friends
// (RandomUsersMgr), the tracking, the ads' banners and the CRM (GameCrmMgr).
#include "Menu_Social.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "box/BananasAndTokensBox.h"
#include "game/Game.h"
#include "game/Statistics.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceGrid.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"

const jet::String Menu_Social::k_menuName("Social");

// OnlinePlayerData: nobody is logged in to any network (IsLoggedInFacebook @0x3fe270, IsLoggedInQihoo @0x3fe2ec, IsLoggedInGameAPI
// @0x401ca4, IsLoggedInGameCenter @0x3fe368, IsLoggedInToAnonymous @0x3fe1f4), and no login is possible (CanLogin @0x3ff154)
static bool IsLoggedInFacebook() { return false; }
static bool IsLoggedInQihoo() { return false; }
static bool IsLoggedInGameAPI() { return false; }
static bool IsLoggedInGameCenter() { return false; }
static bool IsLoggedInToAnonymous() { return false; }
static bool CanLogin() { return false; }
// LeaderboardMgr: no leaderboard is loading (@0x46b2e0), none shows a connection error (@0x46d7e4) and no user is loaded (@0x46b238)
static bool IsLoadingLeaderboard(int, int) { return false; }
static bool ShouldShowConnectionError(int, int) { return false; }
static int GetNumUsersLoaded(int, int) { return 0; }

static const jet::String &Text(const char *key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)); }

// @0x494974: opens on the leaderboard of the friends
Menu_Social::Menu_Social()
    : Menu_Base(jet::String("SocialMenu")), m_isLoadingLeaderboard(false), m_isWaitingForLogin(false), m_leaderboardRange(Range_Friends), m_leaderboardType(Type_Score),
      m_currentState(0), m_lastState(0), m_hasToTrack(false), m_contestButton(0), m_delayer(0.0f, 100.0f, 1000), m_noWeeklyContestDialogs(false), m_claimedBananas(0),
      m_claimedTokens(0) {
    m_worldGrid = m_friendsScoreGrid = m_friendsContestGrid = m_weeklyGrid = 0;
    m_backButton = m_inviteFriendsButton = m_leaderboardButton = m_friendsScoreButton = m_friendsContestButton = m_friendButton = 0;
    m_weeklyPrizesButton = m_loginFacebookButton = m_facebookConnectButton = m_loginGoogleButton = 0;
    m_loadingIcon = 0;
    m_bg = m_weeklyInfoBgGraph = m_scrollBarWeekly = m_loginBg = m_friendsParent = m_scrollBarFriends = m_scrollBarWorld = m_waitingForInitialize = m_loadingProblemText = 0;
    m_weeklyMissionInfoText = m_noWeeklyMissionText = m_weeklyMissionDurationText = 0;
    m_weeklyTimePBar = 0;
    m_bananasAndTokensBox = 0;
    m_delayer.SetDelay(900);  // @0x494a78
}

// @0x494aac
Menu_Social::~Menu_Social() {
    ClearLeaderboard();
    delete m_bananasAndTokensBox;
}

// @0x494b3c: the elements of the layer
void Menu_Social::Init() {
    Menu_Base::Init();
    m_friendsScoreGrid = GetUIGrid(jet::String("FriendsScoreGrid"));
    m_friendsContestGrid = GetUIGrid(jet::String("FriendsContestGrid"));
    m_worldGrid = GetUIGrid(jet::String("WorldGrid"));
    m_bg = GetUIObject(jet::String("MenuBG"));
    m_backButton = GetUIButton(jet::String("BackButton"));
    m_inviteFriendsButton = GetUIButton(jet::String("InviteFriendsButton"));
    m_loadingIcon = GetUIGraph(jet::String("LoadingIcon"));
    m_friendsScoreButton = GetUIButton(jet::String("B_Friends_score"));
    m_friendsContestButton = GetUIButton(jet::String("B_Friends_contest"));
    m_leaderboardButton = GetUIButton(jet::String("B_Leaderboard"));
    m_contestButton = GetUIButton(jet::String("B_Contest"));
    m_loginBg = GetUIObject(jet::String("LoginBG"));
    m_loginFacebookButton = GetUIButton(jet::String("FacebookButton"));
    m_loginGoogleButton = GetUIButton(jet::String("GoogleButton"));
    m_friendsParent = GetUIObject(jet::String("Friends_parent"));
    m_scrollBarFriends = GetUIObject(jet::String("InterfaceScrollBarFriends"));
    m_scrollBarWorld = GetUIObject(jet::String("InterfaceScrollBarWorld"));
    m_facebookConnectButton = GetUIButton(jet::String("FacebookConnectButton"));
    m_weeklyInfoBgGraph = GetUIObject(jet::String("WeeklyInfoBgGraph"));
    m_scrollBarWeekly = GetUIObject(jet::String("InterfaceScrollBarWeekly"));
    m_weeklyGrid = GetUIGrid(jet::String("WeeklyGrid"));
    m_weeklyMissionInfoText = GetUIText(jet::String("WeeklyMissionInfoText"));
    m_noWeeklyMissionText = GetUIText(jet::String("NoWeeklyMissionText"));
    m_weeklyMissionDurationText = GetUIText(jet::String("WeeklyMissionDurationText"));
    m_weeklyPrizesButton = GetUIButton(jet::String("WeeklyPrizesButton"));
    m_weeklyTimePBar = GetUIPBar(jet::String("WeeklyTimeBar"));
    m_loadingProblemText = GetUIObject(jet::String("LoadingProblemText"));
    m_waitingForInitialize = GetUIObject(jet::String("WaitingForInitializeBg"));
    m_currentState = 0;
    // isTabFriend = true (the tab of the friends is the one the invitations answer to)
    m_bananasAndTokensBox = new BananasAndTokensBox(GetUIObject(jet::String("BananasAndTokensCounterPos")), true);
    m_friendButton = GetUIButton(jet::String("GS_Friend"));
    m_friendButton->SetVisible(false);
}

// @0x495ac0: without the online services the menu says so and goes away; else the tabs, the claimed rewards and the buttons
void Menu_Social::Update(int dt) {
    MenuMgr *mm = MenuMgr::GetInstance();
    if (!Game::GetInstance()->AreOnlineServicesReady()) {
        PopupMgr::GetInstance()->PushInfoPopup(std::string(Text("POPUP_NO_INTERNET").c_str()));
        mm->PopMenu();
        return;
    }
    // AnticheatingManager: a cheating player also leaves the menu (not ported)
    if (m_refreshOnNextFrame) Refresh();
    UpdateClaimedInfo();
    if (m_leaderboardType == Type_Contest) RefreshWeeklyContestDuration();
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        // KeyEventManager::ClearKeyEvents()
        mm->PopMenu();
        return;
    }
    Menu_Base::Update(dt);
    // WeeklyChallengeMgr::Prizes_HasPopupsToShow() / Prizes_TryShowPopups(): there are no prizes
    InterfaceGrid *grid;
    if (m_worldGrid->IsVisible()) grid = m_worldGrid;
    else if (m_friendsScoreGrid->IsVisible()) grid = m_friendsScoreGrid;
    else grid = m_friendsContestGrid->IsVisible() ? m_friendsContestGrid : m_weeklyGrid;
    bool tabsActive = !grid->IsGridDragging();
    m_leaderboardButton->SetActive(tabsActive);
    m_friendsScoreButton->SetActive(tabsActive);
    m_friendsContestButton->SetActive(tabsActive);
    m_contestButton->SetActive(tabsActive);
    m_backButton->SetActive(tabsActive);
    m_delayer.Update(dt);
    if (!m_noWeeklyContestDialogs) HandleWeeklyContestNotifications();
    if (mm->ACTION_GetNumReleasedButtons() > 0) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(0);
        if (b == m_backButton)
            mm->PopMenu();
        else if (b == m_inviteFriendsButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // a player who is not logged in to Facebook is asked to (PopupLoginToFacebook), one who is
            // invites the friends who do not play (FriendsMgr::StartLoadingNonPlayingFriends, Menu_InviteFriends): not ported
            if (!IsLoggedInFacebook() && !IsLoggedInGameAPI() && !IsLoggedInQihoo()) {
            }
        } else if (b == m_friendsScoreButton) {
            // isTabFriend = true
            SoundMgr::GetInstance()->ResumeGroup(SoundMgr::k_sfxGroupLabel, 500);
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            SetLeaderBoardRangeAndType(Range_Friends, Type_Score, true);
            NotifyTracking(0);
        } else if (b == m_friendsContestButton) {
            SetLeaderBoardRangeAndType(Range_Friends, Type_Contest, true);
            NotifyTracking(1);
        } else if (b == m_contestButton) {
            // isTabFriend = false
            SetLeaderBoardRangeAndType(Range_World, Type_Contest, true);
            NotifyTracking(2);
        } else if (b == m_loginFacebookButton || b == m_facebookConnectButton) {
            // OnlinePlayerData::LoginToFacebook() is not ported
            m_refreshOnNextFrame = true;
        } else if (b == m_weeklyPrizesButton) {
            // PopupWeeklyPrizes (the world's) or PopupFriendsWeeklyPrizes are pushed (not ported)
        }
    }
    if (m_isWaitingForLogin) {
        if (CanLogin()) Refresh();
    } else if (m_isLoadingLeaderboard) {
        if (!IsLoadingLeaderboard(m_leaderboardType, m_leaderboardRange)) Refresh();
    }
    if (m_hasToTrack) {
        // GameTrackingMgr::SendVisitScreenEvent(GetPreviousTrackingScreen(), GetCurrentTrackingScreen())
        m_hasToTrack = false;
    }
    // the banner of the ads is shown once the menu has been entered (isBannerVisible, timeMillis1)
}

// @0x496428: the background is drawn first, over what the menu scene shows
void Menu_Social::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    m_bg->SetVisible(true);
    m_bg->Render(cam, p);
    m_bg->SetVisible(false);
    Menu_Base::Render(cam, p);
}

// @0x49648c: the first state (1) refreshes the menu, the others tell the ads' banner to go
void Menu_Social::SetState(int s) {
    Menu_Base::SetState(s);
    if (s == 1) Refresh();
    // 0: the banner flag is set; 2: it is cleared and the banner is hidden
}

// @0x49651c
void Menu_Social::OnEnter() {
    ResetClaimedInfo();
    m_lastState = 3;
}

// @0x496534: which tab, board and login are shown
void Menu_Social::Refresh() {
    RefreshWeeklyContestNotifications();
    bool loggedIn;
    if (IsLoggedInFacebook() || IsLoggedInQihoo() || IsLoggedInGameAPI() || IsLoggedInGameCenter()) loggedIn = true;
    else loggedIn = m_leaderboardRange == Range_Friends ? false : IsLoggedInToAnonymous();
    m_isLoadingLeaderboard = IsLoadingLeaderboard(m_leaderboardType, m_leaderboardRange);
    bool connectionError = ShouldShowConnectionError(m_leaderboardType, m_leaderboardRange);
    m_loadingIcon->SetVisible(loggedIn ? m_isLoadingLeaderboard : false);
    m_loadingProblemText->SetVisible(loggedIn && m_isLoadingLeaderboard ? connectionError : false);
    m_friendsParent->SetVisible(loggedIn && m_leaderboardRange == Range_Friends);
    bool worldBoard = m_leaderboardType == Type_Score && m_leaderboardRange == Range_World;
    m_scrollBarWorld->SetVisible(worldBoard && loggedIn);
    m_weeklyInfoBgGraph->SetVisible(false);
    m_noWeeklyMissionText->SetVisible(false);
    HandleFacebookRewardDisplay();
    m_refreshOnNextFrame = false;
    ClearLeaderboard();
    if (!m_isLoadingLeaderboard && loggedIn) {
        PopulateLeaderboard();
        if (m_leaderboardRange == Range_World) {
            Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_checkLeaderboardsCount, 1);
            // GameCrmMgr::SocialPointCut()
        }
    }
    m_worldGrid->SetVisible(worldBoard);
    m_leaderboardButton->SetSelected(worldBoard);
    bool friendsScore = m_leaderboardType == Type_Score && m_leaderboardRange == Range_Friends;
    m_friendsScoreGrid->SetVisible(friendsScore);
    m_friendsScoreButton->SetSelected(friendsScore);
    bool friendsContest = m_leaderboardType == Type_Contest && m_leaderboardRange == Range_Friends;
    m_friendsContestGrid->SetVisible(friendsContest);
    m_friendsContestButton->SetSelected(friendsContest);
    bool contest = m_leaderboardType == Type_Contest && m_leaderboardRange == Range_World;
    m_weeklyGrid->SetVisible(contest);
    m_contestButton->SetSelected(contest);
    // the contest tabs mean the player has seen the new contest (WeeklyChallengeMgr::MarkUserHasSeenTheNewContest)
    UpdateContestButton(true);
    const char *title;
    if (m_leaderboardType == Type_Contest) title = m_leaderboardRange == Range_Friends ? "WEEKLYC_FRIENDS_TITLE" : "WEEKLY_CONTEST";
    else title = m_leaderboardRange == Range_World ? "LEADERBOARD" : "FRIENDS_SCORE";
    GetUIText(jet::String("Category_Text"))->SetText(Text(title));
    bool canLogin = CanLogin();
    // the background of the login shows for a Qihoo player who cannot log in or is not logged in
    m_loginBg->SetVisible(IsLoggedInQihoo() ? (!canLogin || !loggedIn) : false);
    m_inviteFriendsButton->SetVisible(false);
    if (!loggedIn) {
        m_loginGoogleButton->SetVisible(loggedIn);
        m_waitingForInitialize->SetVisible(!canLogin);
    }
    m_isWaitingForLogin = !canLogin;
    GetUIObject(jet::String("LoginDefault_Text"))->SetVisible(false);
    GetUIObject(jet::String("LoginReward_Text"))->SetVisible(false);
    m_facebookConnectButton->SetVisible(false);
    if (m_leaderboardType == Type_Contest && m_leaderboardRange == Range_Friends && loggedIn) {
        // WeeklyChallengeMgr::TryRestartingActiveWM()
        RefreshWeeklyContestInfo();
    }
    UpdateClaimedInfo();
}

// @0x496d0c: the mission of the contest, or the text that there is none (the contest comes from WeeklyChallengeMgr, EventsMgr
// and the project's WeeklyMission entity: there is none)
void Menu_Social::RefreshWeeklyContestInfo() { m_noWeeklyMissionText->SetVisible(true); }

// @0x496e2c: the time the contest has left; without a contest the menu is refreshed
void Menu_Social::RefreshWeeklyContestDuration() {
    // WeeklyChallengeMgr::TryRestartingActiveWM()
    Refresh();
}

// @0x497024: the boxes of the board are thrown away
void Menu_Social::ClearLeaderboard() {
    for (size_t i = 0; i < m_userBoxList.size(); i++) {
        m_userBoxList[i]->SetParent(0);
        delete m_userBoxList[i];
    }
    m_userBoxList.clear();
}

// @0x497084: a box for each user of the board in the grid of the tab (LeaderboardMgr has none loaded)
void Menu_Social::PopulateLeaderboard() {
    InterfaceGrid *grid;
    if (m_leaderboardType == Type_Score) grid = m_leaderboardRange == Range_World ? m_worldGrid : m_friendsScoreGrid;
    else if (m_leaderboardType == Type_Contest) grid = m_leaderboardRange == Range_World ? m_weeklyGrid : m_friendsContestGrid;
    else grid = 0;
    int users = GetNumUsersLoaded(m_leaderboardType, m_leaderboardRange);
    // a LeaderboardUserBox for each of them, and the player's one is the main child of the grid; with the player's last run and
    // no friends, random users (RandomUsersMgr) fill the board
    if (grid) grid->SetRowCount(users);
}

// @0x497424
void Menu_Social::SetLeaderBoardRangeAndType(int range, int type, bool force) {
    if (type == m_leaderboardType && range == m_leaderboardRange && !force) return;
    m_leaderboardType = type;
    m_leaderboardRange = range;
    ResetClaimedInfo();
    Refresh();
}

// @0x497470
void Menu_Social::PushMenuWithLeaderBoardRangeAndType(int range, int type) {
    MenuMgr *mm = MenuMgr::GetInstance();
    Menu_Base *current = mm->GetCurrentPlayingMenu();
    if (!current || !current->GetMenuName().Equals(k_menuName)) mm->PushMenu(k_menuName);
    static_cast<Menu_Social*>(mm->GetMenu(k_menuName))->SetLeaderBoardRangeAndType(range, type, false);
}

// @0x4974e4, @0x4974ec
int Menu_Social::GetCurrentTrackingScreen() const { return GetTrackingState(m_currentState); }
int Menu_Social::GetPreviousTrackingScreen() const { return GetTrackingState(m_lastState); }

// @0x4974f4: the screen of the tracking of a tab (-1 for none)
int Menu_Social::GetTrackingState(int state) const {
    static const int k_screens[4] = { 0x1bcaf, 0x1bcaf, 0x1bcb0, -1 };
    return state >= 0 && state <= 3 ? k_screens[state] : -1;
}

// @0x497518: the tab the player went to is tracked on the next update
void Menu_Social::NotifyTracking(int state) {
    m_lastState = m_currentState;
    m_currentState = state;
    m_hasToTrack = true;
}

// @0x497530: the buttons of the contest tabs are the ones that tell of a new contest (the alert ones) when there is one;
// without a contest (WeeklyChallengeMgr, EventsMgr) they are the plain ones
void Menu_Social::UpdateContestButton(bool force) {
    if (!force) {
        InterfaceButton *plain = GetUIButton(jet::String("B_Contest"));
        InterfaceButton *plainFriends = GetUIButton(jet::String("B_Friends_contest"));
        if (m_contestButton == plain && m_friendsContestButton == plainFriends) return;
    }
    bool alert = false;
    bool selected = m_contestButton->IsSelected();
    InterfaceButton *contest = GetUIButton(jet::String("B_Contest"));
    contest->SetVisible(!alert);
    InterfaceButton *contestAlert = GetUIButton(jet::String("B_Contest_Alert"));
    contestAlert->SetVisible(alert);
    m_contestButton = alert ? contestAlert : contest;
    m_contestButton->SetSelected(selected);
    bool loggedIn = IsLoggedInFacebook() || IsLoggedInGameCenter() || IsLoggedInGameAPI() || IsLoggedInQihoo();
    InterfaceButton *friendsContest = GetUIButton(jet::String("B_Friends_contest"));
    friendsContest->SetVisible(!alert || !loggedIn);
    InterfaceButton *friendsContestAlert = GetUIButton(jet::String("B_Friends_contest_Alert"));
    friendsContestAlert->SetVisible(loggedIn && alert);
    bool friendsSelected = m_friendsContestButton->IsSelected();
    m_friendsContestButton = loggedIn && alert ? friendsContestAlert : friendsContest;
    m_friendsContestButton->SetSelected(friendsSelected);
}

// @0x4977a4: the reward of the network for the first login is shown until the player got it (nothing is given offline)
void Menu_Social::HandleFacebookRewardDisplay() {
    bool gotReward = false;  // FacebookAtLaunchMgr::HasGotReward(4)
    jet::String reward = babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(0);  // FacebookAtLaunchMgr::GetFacebookReward()
    GetUIObject(jet::String("Reward_text"))->SetVisible(!gotReward);
    GetUIObject(jet::String("Token"))->SetVisible(!gotReward);
    GetUIText(jet::String("Token_Value"))->SetVisible(!gotReward);
    GetUIText(jet::String("Token_Value"))->SetText(reward);
}

// @0x49798c: the contest's dialogs of the tutorial open the board of the contest of the friends (they are not in this build)
void Menu_Social::RefreshWeeklyContestNotifications() {}

// @0x4979d4: the dialogs that explain the contest (TutorialMgr::AreWeeklyContestDialogs: none here)
void Menu_Social::HandleWeeklyContestNotifications() {}

// @0x497b5c
void Menu_Social::ResetClaimedInfo() {
    m_claimedBananas = 0;
    m_claimedTokens = 0;
}

// @0x497b6c
void Menu_Social::AddClaimedPriceAmount(const Price &p) {
    if (p.GetPriceType() == Price::Tokens) m_claimedTokens += p.GetAmount();
    if (p.GetPriceType() == Price::Bananas) m_claimedBananas += p.GetAmount();
}

// @0x497bc4: the bananas and tokens the prizes of the week gave, while there are any
void Menu_Social::UpdateClaimedInfo() {
    GetUIObject(jet::String("claimed_bananas"))->SetVisible(m_claimedBananas > 0);
    babel::Formatter *f = babel::Babel::GetInstance()->GetFormatter();
    if (m_claimedBananas > 0) GetUIText(jet::String("claimed_banana_count"))->SetText(f->FormatMeasure(m_claimedBananas));
    GetUIObject(jet::String("claimed_tokens"))->SetVisible(m_claimedTokens > 0);
    if (m_claimedTokens > 0) GetUIText(jet::String("claimed_tokens_count"))->SetText(f->FormatMeasure(m_claimedTokens));
}
