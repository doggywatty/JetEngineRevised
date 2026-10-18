// source/game/interfaces/menus/Menu_Social.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_SOCIAL_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_SOCIAL_H

#include <vector>
#include "Menu_Base.h"
#include "core/delayer.h"
#include "core/pstring.h"

class BananasAndTokensBox;
class InterfaceButton;
class InterfaceGraph;
class InterfaceGrid;
class InterfaceObject;
class InterfacePBar;
class InterfaceText;
class BasicPage;
class Price;

// The social menu: the leaderboards of the world and of the friends, the contest of the week, and the login to the social
// network that shows them. The tabs are the range (world, friends) and the type (score, contest) of the board.
// The online services are not in this build, so the menu is ported with what they would answer left out: nobody is logged in
// (OnlinePlayerData), no board is loading or loaded (LeaderboardMgr, LeaderboardUserBox), there is no contest of the week
// (WeeklyChallengeMgr, EventsMgr, WeeklyMission) and no reward of the network (FacebookAtLaunchMgr); the login, the invitation
// of friends and the prizes of the week (PopupLoginToFacebook, PopupChooseSNS, PopupWeeklyPrizes, FriendsMgr), the tracking,
// the sounds and the banners of the ads are not ported either. The control flow around them is the .so's.
class Menu_Social : public Menu_Base {  // sizeof 0x1EC
public:
    // the range and the type of a leaderboard (LeaderboardMgr::LeaderboardRange / LeaderboardType)
    enum LeaderboardRange { Range_World = 0, Range_Friends = 1 };
    enum LeaderboardType { Type_Score = 0, Type_Contest = 1 };
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    Menu_Social();
    virtual ~Menu_Social();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void SetState(int);
    virtual void Refresh();
    virtual bool RenderGameInBG() { return false; }  // @0x482754
    virtual void OnEnter();
    virtual bool ShowBanner() const { return true; }  // @0x48275c
    virtual int GetCurrentTrackingScreen() const;
    virtual int GetPreviousTrackingScreen() const;
    void OnLeaderboardBox(int) {}  // @0x497420
    void SetLeaderBoardRangeAndType(int range, int type, bool force);  // @0x497424
    // opens the menu (when it is not the one on top) on the leaderboard
    static void PushMenuWithLeaderBoardRangeAndType(int range, int type);  // @0x497470
    static const jet::String k_menuName;
private:
    void ClearLeaderboard();  // @0x497024
    void PopulateLeaderboard();  // @0x497084
    void RefreshWeeklyContestInfo();  // @0x496d0c
    void RefreshWeeklyContestDuration();  // @0x496e2c
    void RefreshWeeklyContestNotifications();  // @0x49798c
    void HandleWeeklyContestNotifications();  // @0x4979d4
    void HandleFacebookRewardDisplay();  // @0x4977a4
    void UpdateContestButton(bool);  // @0x497530
    void ResetClaimedInfo();  // @0x497b5c
    void AddClaimedPriceAmount(const Price&);  // @0x497b6c
    void UpdateClaimedInfo();  // @0x497bc4
    int GetTrackingState(int state) const;  // @0x4974f4
    void NotifyTracking(int state);  // @0x497518
    InterfaceGrid *m_worldGrid;                    // 0x124
    InterfaceGrid *m_friendsScoreGrid;             // 0x128
    InterfaceGrid *m_friendsContestGrid;           // 0x12C
    InterfaceButton *m_backButton;                 // 0x130
    InterfaceButton *m_inviteFriendsButton;        // 0x134
    InterfaceGraph *m_loadingIcon;                 // 0x138
    InterfaceButton *m_leaderboardButton;          // 0x13C: B_Leaderboard
    InterfaceButton *m_friendsScoreButton;         // 0x140: B_Friends_score
    InterfaceButton *m_friendsContestButton;       // 0x144: B_Friends_contest, or its alert one
    InterfaceObject *m_bg;                         // 0x148
    InterfaceButton *m_friendButton;               // 0x14C: GS_Friend
    InterfaceObject *m_weeklyInfoBgGraph;          // 0x150
    InterfaceObject *m_scrollBarWeekly;            // 0x154
    InterfaceGrid *m_weeklyGrid;                   // 0x158
    InterfaceText *m_weeklyMissionInfoText;        // 0x15C
    InterfaceText *m_noWeeklyMissionText;          // 0x160
    InterfaceText *m_weeklyMissionDurationText;    // 0x164
    InterfaceButton *m_weeklyPrizesButton;         // 0x168
    InterfacePBar *m_weeklyTimePBar;               // 0x16C
    InterfaceObject *m_loginBg;                    // 0x170
    InterfaceButton *m_loginFacebookButton;        // 0x174
    InterfaceButton *m_facebookConnectButton;      // 0x17C
    InterfaceButton *m_loginGoogleButton;          // 0x180
    InterfaceObject *m_friendsParent;              // 0x184
    InterfaceObject *m_scrollBarFriends;           // 0x190
    InterfaceObject *m_scrollBarWorld;             // 0x194
    InterfaceObject *m_waitingForInitialize;       // 0x198
    InterfaceObject *m_loadingProblemText;         // 0x19C
    BananasAndTokensBox *m_bananasAndTokensBox;    // 0x1A0
    std::vector<BasicPage*> m_userBoxList;  // 0x1A4: the boxes (LeaderboardUserBox, not ported) of the board
    bool m_isLoadingLeaderboard;                   // 0x1B0
    bool m_isWaitingForLogin;                      // 0x1B1: the login is not possible yet
    int m_leaderboardRange;                        // 0x1B4
    int m_leaderboardType;                         // 0x1B8
    int m_currentState;                            // 0x1BC
    int m_lastState;                               // 0x1C0
    bool m_hasToTrack;                             // 0x1C4
    InterfaceButton *m_contestButton;              // 0x1C8: B_Contest, or its alert one
    jet::core::Delayer<float, jet::core::SmoothStep> m_delayer;  // 0x1CC
    bool m_noWeeklyContestDialogs;                 // 0x1E0
    int m_claimedBananas;                          // 0x1E4
    int m_claimedTokens;                           // 0x1E8
};

#endif
