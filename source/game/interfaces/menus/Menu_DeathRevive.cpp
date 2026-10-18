// source/game/interfaces/menus/Menu_DeathRevive.cpp
//
// The revive offer of a death (Menu_DeathRevive in the .so). Every function follows the .so's order of calls. Not ported, and why:
//  - the friends' avatars (OnlineUser::RenderAvatar, the photo areas of the social, next friend and top friend incentives) and the
//    OnlineUser::Update calls: the online users are not in the port (no login);
//  - LeaderboardMgr::GetNextFriendToBeat (@0x46d4d4): the leaderboards are online, so no next friend to beat is found;
//  - Challenge (the challenge incentive text, GetCompletedPercentage, @0x55d0f0 and @0x55d060) and the online name of the challenge's
//    friend (ChallengeMgr::OnlineUser_GetName): ChallengeMgr::Challenge_IsActive is false in the port (it needs the login), so the
//    challenge branch never reaches them;
//  - GameTrackingMgr's revive and currency events (TRACKING_* are not in the tree);
//  - KeyEventManager::ClearKeyEvents (the key event manager is empty in the tree) and TouchMgr::GetAllActiveTouches (its result is not used
//    by the .so either).
#include "Menu_DeathRevive.h"
#include "Menu_PortraitMessage.h"
#include "Menu_ResultEndScreen.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/Player.h"
#include "game/Statistics.h"
#include "../../cheats/CheatsMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/challenges/ChallengeMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "localization/formatter.h"
#include "game/entities/Mission.h"
#include "game/gamestates/GS_Gameplay.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/popups/PopupInfo.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/interfaceobjects/InterfaceObject.h"
#include "project.h"
#include "sound/soundmgr.h"
#include <functional>

const jet::String Menu_DeathRevive::k_menuName("DeathRevive");
bool Menu_DeathRevive::s_boughtRevive = false;
const int Menu_DeathRevive::k_dieTime = 5000;

// the babel keys of the incentives' texts
static const char *const k_keepRunningTutoKey = "REVIVE_INCENTIVE_KEEP_RUNNING_TUTO";
static const char *const k_bestScoreTutoKey = "REVIVE_INCENTIVE_BESTSCORE_TUTO";
static const char *const k_friendChallengerKey = "FRIEND_CHALLENGER";

static babel::StringMgr *StrMgr() { return babel::Babel::GetInstance()->GetStringMgr(); }
static babel::Formatter *Fmt() { return babel::Babel::GetInstance()->GetFormatter(); }

Menu_DeathRevive::Menu_DeathRevive()
    : Menu_Base(jet::String("Ingame_DeathRevive")), m_dieTime(0), m_tutoPortrait(0), m_boughtRevive(false), m_flag12D(false), m_reviveButton(0),
      m_reviveUseButton(0), m_skipButton(0), m_skipButton2(0), m_interfacePBar(0), m_tokensValue(0), m_nextFriend(0), m_nextFriendAvatarRect(0),
      m_nextFriendBg(0), m_nextFriendArea(0), m_nextFriendScore(0), m_revivePaid(false), m_socialParent(0), m_nextFriendParent(0),
      m_topFriendParent(0), m_scoreParent(0), m_missionParent(0), m_incentiveShown(false), m_topFriend(0) {}

Menu_DeathRevive::~Menu_DeathRevive() {}

const jet::String &Menu_DeathRevive::GetMenuName() const { return k_menuName; }
bool Menu_DeathRevive::UpdateGameInBG() { return true; }
bool Menu_DeathRevive::RenderGameInBG() { return true; }
TRACKING_SCREEN Menu_DeathRevive::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }

// @0x4e561c: the menu is entered: the purchase flag of the .so's static is kept, the timer restarts, the texts are refreshed, and the
// game knows a revive menu is up
void Menu_DeathRevive::OnEnter() {
    ResetDieTime();
    Refresh();
    Game::GetInstance()->SetInDeathRevive(true);
}

// @0x4e55dc: the menu is left. When a revive was paid, the game's revive sequence starts (the menu pops first)
void Menu_DeathRevive::OnQuit() {
    Game::GetInstance()->SetInDeathRevive(false);
    if (m_revivePaid) {
        GameState::PopState();
        GS_Gameplay::GetInstance()->BeginRevivingSequence();
        m_revivePaid = false;
    }
}

// @0x4e46ac: the buttons, the bars, the texts and the incentive parents of the page
void Menu_DeathRevive::Init() {
    Menu_Base::Init();
    m_reviveButton = GetUIButton(jet::String("ReviveButton"));
    m_reviveUseButton = GetUIButton(jet::String("Revive_UseButton"));
    m_skipButton = GetUIButton(jet::String("SkipButton"));
    m_skipButton2 = GetUIButton(jet::String("SkipButton2"));
    m_interfacePBar = GetUIPBar(jet::String("InterfacePBar"));
    m_interfacePBar->SetVisible(false);
    m_tokensValue = GetUIText(jet::String("Tokens_Value"));
    m_nextFriendBg = GetUIGraph(jet::String("NextFriend_bg"));
    m_nextFriendBg->SetVisible(false);
    m_nextFriendArea = GetUIArea(jet::String("NextFriend_Area"));
    GetUIGraph(jet::String("Tokens_box"))->SetVisible(false);
    m_socialParent = GetUIObject(jet::String("ReviveIncentiveSocial_parent"));
    m_nextFriendParent = GetUIObject(jet::String("ReviveIncentiveNextFriend_parent"));
    m_topFriendParent = GetUIObject(jet::String("ReviveIncentiveTopFriend_parent"));
    m_scoreParent = GetUIObject(jet::String("ReviveIncentiveScore_Parent"));
    m_missionParent = GetUIObject(jet::String("ReviveIncentiveMission_Parent"));
}

// @0x4e5adc (Update, the frame of the menu): the button presses. Skip ends the run without a revive (GameLevel::OnMinionNoRevive, unless
// the tutorial portrait is still up: then the press does nothing); Revive buys the revive (the tutorial's is free and is paid with
// ExecuteCost of the revive price). Then the tutorial portrait is updated and the die timer restarts.
void Menu_DeathRevive::Update(int dt) {
    if (m_refreshOnNextFrame) Refresh();
    Menu_Base::Update(dt);
    // OnlineUser::Update of the next friend and of the top friend (@0x420160): the online users are not in the port
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        // KeyEventManager::ClearKeyEvents (@0x505184): the key event manager is empty in the tree
    }
    for (int i = 0; i < GetNumReleasedButtons(); i++) {
        InterfaceButton *button = GetReleasedButton(i);
        if (button == m_skipButton || button == m_skipButton2) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            if (!m_tutoPortrait) GameLevel::GetInstance()->OnMinionNoRevive();
            return;
        }
        if (button == m_reviveButton || button == m_reviveUseButton) {
            if (!m_tutoPortrait) {
                BuyDeathRevive(0, 0);
            } else {
                // the first death: the revive is the tutorial's. Its popup says so, the price is paid with ExecuteCost, the portrait goes
                // away, the purchase is kept for Refresh, the tutorial is complete, 5 tokens are given and the skip buttons show again.
                PopupMgr::GetInstance()->PushPopup(new PopupInfo(PopupInfo::Info, std::string(StrMgr()->Get(jet::String("REVIVE_TUTO_COMPLETE")).c_str()), 0));
                Player::GetInstance()->ExecuteCost(*Game::GetInstance()->DeathRevive_GetRevivePrice(),
                                                   std::function<void(int, int)>([this](int, int) { BuyDeathRevive(0, 0); }));
                m_boughtRevive = true;
                delete m_tutoPortrait;
                m_tutoPortrait = 0;
                Player::GetInstance()->AddTokens(5);
                Player::GetInstance()->SetReviveTutoComplete(true);
                m_skipButton->SetVisible(true);
                m_skipButton2->SetVisible(true);
            }
            return;
        }
    }
    // the .so checks the press state of the two revive buttons here and then calls TouchMgr::GetAllActiveTouches, whose result is dropped
    if (m_tutoPortrait) {
        m_tutoPortrait->Update(dt);
        ResetDieTime();
    }
}

// @0x4e51bc: the tokens of the player, then the page (and the portrait of the first death)
void Menu_DeathRevive::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    const int tokens = Player::GetInstance()->GetTokens();
    m_tokensValue->SetText(Fmt()->FormatMeasure(tokens));
    Menu_Base::Render(cam, p);
    if (m_tutoPortrait) m_tutoPortrait->Render(cam, p);
}

// @0x4e5668: the price of the revive, the revives owned, the buttons and the texts of the page. The first death shows the tutorial portrait
// (the revive is free the first time: the tutorial). A revive bought while the menu was not up is given here.
void Menu_DeathRevive::Refresh() {
    m_refreshOnNextFrame = false;
    Game *game = Game::GetInstance();
    Player *player = Player::GetInstance();
    const int price = (int)game->DeathRevive_GetRevivePrice()->GetPriceAsDouble();
    const int bananas = player->GetBananas();
    const bool tutoComplete = player->GetReviveTutoComplete();
    bool portraitAllowed = true;
    if (price > bananas) {
        if (s_boughtRevive) {
            player->AddTokens(price);
            s_boughtRevive = false;
            BuyDeathRevive(0, 0);
            portraitAllowed = true;
        } else {
            portraitAllowed = false;
        }
    }
    ResetDieTime();
    const int revives = player->GetReviveQuantity();
    const bool useRevive = game->DeathRevive_GetRevivePriceType() == 2 && revives > 0;
    m_reviveUseButton->SetVisible(useRevive);
    m_reviveButton->SetVisible(!useRevive);
    GetUIObject(jet::String("Revive_Quantity"))->SetVisible(useRevive);
    GetUIText(jet::String("Revive_Number"))->SetText(jet::String::Format("%d", revives));
    const int priceInt = (int)game->DeathRevive_GetRevivePrice()->GetPriceAsDouble();
    const jet::String measure = Fmt()->FormatCurrency(priceInt);
    m_reviveButton->SetText(jet::String::Format(StrMgr()->Get(jet::String("REVIVE_DESC")).c_str(), measure.c_str()));
    RefreshNextFriendToBeat();
    RefreshScoreAndBananas();
    if (!tutoComplete && !m_tutoPortrait && portraitAllowed) {
        // the first death: the tutorial portrait points at the revive; the skips are hidden until the tutorial is done
        m_skipButton->SetVisible(false);
        m_skipButton2->SetVisible(false);
        m_tutoPortrait = new Menu_PortraitMessage();
        m_tutoPortrait->Init();
        m_tutoPortrait->HidePortraitMessage();
        const math::vec2<float> pos = m_interfacePBar->GetPos();
        const math::vec2<float> size = m_interfacePBar->GetSize();
        m_tutoPortrait->SelfShowTutoGraph(6, math::vec2<float>(pos.x + size.x, pos.y), false);
        for (InterfaceText *title : {GetUIText(jet::String("ReviveIncentiveSocial_TitleText")), GetUIText(jet::String("ReviveIncentiveNextFriend_TitleText")),
                                     GetUIText(jet::String("ReviveIncentiveTopFriend_TitleText")), GetUIText(jet::String("ReviveIncentiveScore_TitleText")),
                                     GetUIText(jet::String("ReviveIncentiveMission_TitleText"))})
            title->SetText(StrMgr()->Get(jet::String(k_keepRunningTutoKey)));
        // the best score incentive is the one the tutorial shows when no other incentive is up
        const bool anyVisible = m_socialParent->IsVisible() || m_topFriendParent->IsVisible() || m_nextFriendParent->IsVisible() ||
                                m_scoreParent->IsVisible() || m_missionParent->IsVisible();
        if (!anyVisible) {
            GetUIText(jet::String("ReviveIncentiveScore_DescText"))->SetText(StrMgr()->Get(jet::String(k_bestScoreTutoKey)));
            m_scoreParent->SetVisible(true);
        }
    }
    // the titles of the incentives (the same text in the tutorial and in the tail of the .so's Refresh)
    for (InterfaceText *title : {GetUIText(jet::String("ReviveIncentiveSocial_TitleText")), GetUIText(jet::String("ReviveIncentiveNextFriend_TitleText")),
                                 GetUIText(jet::String("ReviveIncentiveTopFriend_TitleText")), GetUIText(jet::String("ReviveIncentiveScore_TitleText")),
                                 GetUIText(jet::String("ReviveIncentiveMission_TitleText"))})
        title->SetText(StrMgr()->Get(jet::String(k_keepRunningTutoKey)));
    // a revive bought from the tutorial path (OnQuit of the tutorial's popup) is run from here
    if (m_boughtRevive) BuyDeathRevive(0, 0);
}

// @0x4e6374: the die timer restarts
void Menu_DeathRevive::ResetDieTime() { m_dieTime = 4000; }

// @0x4e6838: the incentives of the death. Each builder shows its part and reports whether it applies; the parents follow, and the statistic
// of the incentive shown (or not shown) is counted for the run.
void Menu_DeathRevive::ComputeBuyIncentiveData() {
    m_topFriend = 0;
    bool challenge = false, score = false, mission = false, beatNext = false;
    if (Statistics::GetInstance()->GetBestScore() > 0) {
        challenge = BuyIncentive_ShowChallengeIncentive();
        m_topFriend = BuyIncentive_ShowTopFriend();
        score = BuyIncentive_ShowScoreIncentive();
        mission = BuyIncentive_ShowMissionIncentive();
        beatNext = BuyIncentive_showBeatNextFriendIncentive();
    }
    m_socialParent->SetVisible(challenge);
    m_topFriendParent->SetVisible(m_topFriend != 0);
    m_nextFriendParent->SetVisible(beatNext);
    m_scoreParent->SetVisible(score);
    m_missionParent->SetVisible(mission);
    m_incentiveShown = challenge || m_topFriend != 0 || beatNext || score || mission;
    Statistics::GetInstance()->IncreaseLocationStat(-1,
                                                    m_incentiveShown ? StatisticsSaveData::k_statScope_tracking_reviveIncentive_shown
                                                                     : StatisticsSaveData::k_statScope_tracking_reviveIncentive_notShown,
                                                    1);
}

// @0x4e6380: the next friend to beat, with its score and avatar area. The leaderboards are online, so the port finds no next friend (see the
// top of this file); the area stays hidden.
void Menu_DeathRevive::RefreshNextFriendToBeat() {
    m_nextFriend = 0;
    m_nextFriendArea->SetVisible(m_nextFriend != 0);
    if (!m_nextFriend) return;
    GetUIText(jet::String("NextFriend_Score"))->SetText(Fmt()->FormatMeasure(m_nextFriendScore));
    m_nextFriendAvatarRect = GetUIArea(jet::String("NextFriend_AvatarRect"));
}

// @0x4e6558: the score and the bananas of the run; the distance icon is the best one when the run's distance beats the greatest distance
void Menu_DeathRevive::RefreshScoreAndBananas() {
    Statistics *st = Statistics::GetInstance();
    const bool bestDistance = st->GetIntValue(-1, StatisticsSaveData::k_statScope_distance, 0) > st->GetDistanceRecord();
    GetUIObject(jet::String("Distance_icon"))->SetVisible(!bestDistance);
    GetUIObject(jet::String("Distance_icon_best"))->SetVisible(bestDistance);
    GetUIText(jet::String("currDistance_Value"))->SetText(Fmt()->FormatMeasure(st->GetScore()));
    GetUIText(jet::String("Bananas_Value"))->SetText(Fmt()->FormatMeasure(st->GetCurrentRunBananas()));
}

// @0x4e5f40: the revive. A revive that is not free is paid first (Player::ExecuteCost; the callback runs this again when the money is there).
// The revive count of the revives owned goes down when they are the price. The menu pops, and the revive sequence starts when the menu is left
// (OnQuit). Not ported (the tracking events): GameTrackingMgr::SendSinglePlayerRaceReviveEvent and SendCurrencySpentEvent.
void Menu_DeathRevive::BuyDeathRevive(int, int) {
    Game *game = Game::GetInstance();
    const Price *price = game->DeathRevive_GetRevivePrice();
    if (!m_boughtRevive) {
        if (!game->DeathRevive_IsFreeRevivePrice(price)) {
            if (!Player::GetInstance()->ExecuteCost(*price, std::function<void(int, int)>([this](int, int) { BuyDeathRevive(0, 0); })))
                return;
        }
    }
    m_boughtRevive = false;
    if (game->DeathRevive_GetRevivePriceType() == 2) {
        Player *player = Player::GetInstance();
        if (player->GetReviveQuantity() > 0) player->AddReviveQuantity(-1);
    }
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_usedClawSaver, 1);
    MenuMgr::GetInstance()->PopMenu();
    m_revivePaid = true;
}

// @0x4e69b8: the challenge's incentive. The port has no challenges (they need the login): Challenge_IsActive is false, which is the
// .so's first test, so the text of the challenge is not reached.
bool Menu_DeathRevive::BuyIncentive_ShowChallengeIncentive() {
    if (!ChallengeMgr::GetInstance()->Challenge_IsActive()) return false;
    return false;
}

// @0x4e6c40: the top friend, the online friend just above the player (LeaderboardMgr::GetOnlineFriendsArroundPlayer @0x46cd04, then the
// friend's score through LeaderboardMgr::GetUserScore and the texts REVIVE_INCENTIVE_TOPFRIEND_SCORE / FRIEND_CHALLENGER). The leaderboards are
// online and are not in the port (LeaderboardMgr.h does not compile), so no top friend is found here: the parent stays hidden.
OnlineUser *Menu_DeathRevive::BuyIncentive_ShowTopFriend() { return 0; }

// @0x4e75fc: the score incentive: the best score is near (the difference is small, or the run has reached the score's part of the best one)
bool Menu_DeathRevive::BuyIncentive_ShowScoreIncentive() {
    Statistics *st = Statistics::GetInstance();
    const long long best = st->GetBestScore();
    if (best <= 0) return false;
    const long long score = st->GetScore();
    const long long diff = best - score;
    if (diff <= 0) return false;
    const DeathReviveIncentive &cfg = Game::GetInstance()->GetDeathReviveIncentive();
    bool show = CheatsMgr::GetInstance()->IsDRIScoreEntryActive();
    if (!show) {
        const float ratio = (float)score / (float)best;
        show = ratio >= cfg.scorePtg || diff <= cfg.scoreMaxDiff;
    }
    if (!show) return false;
    const jet::String measure = Fmt()->FormatMeasure((int)diff);
    GetUIText(jet::String("ReviveIncentiveScore_DescText"))
        ->SetText(jet::String::Format(StrMgr()->Get(jet::String("REVIVE_INCENTIVE_BESTSCORE")).c_str(), measure.c_str()));
    return true;
}

// @0x4e72a8: the mission incentive: the active mission of the current run that is the furthest done (and not completed)
bool Menu_DeathRevive::BuyIncentive_ShowMissionIncentive() {
    MissionMgr *missions = MissionMgr::GetInstance();
    Mission *best = 0;
    const unsigned count = missions->GetActiveMissionCount();
    for (unsigned i = 0; i < count; i++) {
        const MissionMgrSaveData::ActiveMissionInfo &info = missions->GetActiveMissionInfo(i);
        if (info.state != 1) continue;
        Mission *m = static_cast<Mission*>(clara::Project::GetInstance()->FindEntityByName(info.id));
        if (!m || !m->GetScope().Equals(Mission::k_missionScopeCurrentRun) || m->IsCompleted()) continue;
        if (!best || best->GetCompletedPercentage() < m->GetCompletedPercentage()) best = m;
    }
    if (!best) return false;
    bool show = CheatsMgr::GetInstance()->IsDRIMissionEntryActive();
    if (!show) show = best->GetCompletedPercentage() >= Game::GetInstance()->GetDeathReviveIncentive().missionPtg;
    if (!show) return false;
    BuyIncentive_MissionShowDataFor(best);
    return true;
}

// @0x4e7404: the mission's description, its giver's bar at the mission's completed part (the other bars are hidden)
void Menu_DeathRevive::BuyIncentive_MissionShowDataFor(Mission *m) {
    GetUIText(jet::String("ReviveIncentiveMission_DescText"))->SetText(m->GetDescription());
    GetUIPBar(jet::String("ReviveIncentiveMission_NefarioPBar"))->SetVisible(false);
    GetUIPBar(jet::String("ReviveIncentiveMission_GruPBar"))->SetVisible(false);
    GetUIPBar(jet::String("ReviveIncentiveMission_GirlsPBar"))->SetVisible(false);
    InterfacePBar *bar = 0;
    const jet::String &giver = m->GetMissionGiver();
    if (giver.Equals(Mission::k_missionGiver_Girls)) bar = GetUIPBar(jet::String("ReviveIncentiveMission_GirlsPBar"));
    if (giver.Equals(Mission::k_missionGiver_Gru)) bar = GetUIPBar(jet::String("ReviveIncentiveMission_GruPBar"));
    if (giver.Equals(Mission::k_missionGiver_Nefario)) bar = GetUIPBar(jet::String("ReviveIncentiveMission_NefarioPBar"));
    if (!bar) return;
    bar->SetVisible(true);
    bar->SetValue(m->GetCompletedPercentage());
    bar->Update(1);
}

// @0x4e6f98: the next friend to beat's incentive: the next friend's score is near the player's (the ratio of the scores is NextFriendPtg). The
// next friend is an online friend (see RefreshNextFriendToBeat), so none is found in the port and this incentive does not apply: its texts
// (REVIVE_INCENTIVE_FRIEND_SCORE, FRIEND_CHALLENGER with the friend's name) are only reached with a next friend and are not in the port.
bool Menu_DeathRevive::BuyIncentive_showBeatNextFriendIncentive() {
    RefreshNextFriendToBeat();
    if (!m_nextFriend) return false;
    return false;
}
