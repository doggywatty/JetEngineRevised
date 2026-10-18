// source/game/interfaces/menus/Menu_DeathRevive.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_DEATHREVIVE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_DEATHREVIVE_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"
#include "game/online/tracking/tracking.h"

class InterfaceButton;
class InterfaceText;
class InterfaceGraph;
class InterfaceArea;
class InterfacePBar;
class Menu_PortraitMessage;
class Mission;
class OnlineUser;

// The offer shown when the minion dies (GS_EndGame pushes it, the .so's Menu_DeathRevive, sizeof 0x17C in the .so). Its buttons:
// the revive (with the price in bananas or in the tokens, or a revive from the revives owned: Revive_UseButton), the skip (which ends the run
// without a revive: GameLevel::OnMinionNoRevive) and a second skip. The buy incentives (a challenge, the top friend, the next friend to beat,
// the score and a mission of the run) are shown above it when they apply. The first death also shows the tutorial portrait.
// Layout as in the .so, the offsets are the .so's (Menu_Base is 0x124 bytes).
class Menu_DeathRevive : public Menu_Base {
public:
    virtual const jet::String &GetMenuName() const;
    Menu_DeathRevive();
    virtual ~Menu_DeathRevive();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
    // @0x4e6374: the die timer (k_dieTime) starts again; the portrait's Update restarts it too
    void ResetDieTime();
    // @0x4e6838: the incentives of the death (the challenge, the top friend, the score, the mission and the next friend), shown or hidden
    void ComputeBuyIncentiveData();
    // @0x4e6380: the next friend to beat (the online friends) and its avatar area; hidden when there is none
    void RefreshNextFriendToBeat();
    // @0x4e6558: the score and the bananas of the run, and the distance icons
    void RefreshScoreAndBananas();
    // @0x4e5f40: the revive is bought (the price is paid by ExecuteCost, or it is free), the revive count goes down, the menu pops and the
    // minion is revived when the game is on (GS_Gameplay::BeginRevivingSequence)
    void BuyDeathRevive(int, int);
    // @0x4e6558's neighbour: the flag at +0x12D, set by GS_EndGame when the revive menu is pushed
    void SetFlag12D(bool v) { m_flag12D = v; }
    static const jet::String k_menuName;
private:
    bool BuyIncentive_ShowChallengeIncentive();                        // @0x4e69b8
    OnlineUser *BuyIncentive_ShowTopFriend();                          // @0x4e6c40
    bool BuyIncentive_ShowScoreIncentive();                            // @0x4e75fc
    bool BuyIncentive_ShowMissionIncentive();                          // @0x4e72a8
    void BuyIncentive_MissionShowDataFor(Mission*);                    // @0x4e7404
    bool BuyIncentive_showBeatNextFriendIncentive();                   // @0x4e6f98
    static bool s_boughtRevive;   // Menu_DeathRevive::s_BoughtRevive: a revive bought while the menu was not up (Refresh gives it)
    static const int k_dieTime;   // 5000 (the .so's k_dieTime)
    int m_dieTime;                                   // 0x124
    Menu_PortraitMessage *m_tutoPortrait;            // 0x128: the tutorial portrait of the first death (null after the first revive)
    bool m_boughtRevive;                             // 0x12C: a tutorial revive is paid; Refresh and BuyDeathRevive run it
    bool m_flag12D;                                  // 0x12D: SetFlag12D
    InterfaceButton *m_reviveButton;                 // 0x130
    InterfaceButton *m_reviveUseButton;              // 0x134
    InterfaceButton *m_skipButton;                   // 0x138
    InterfaceButton *m_skipButton2;                  // 0x13C
    InterfacePBar *m_interfacePBar;                  // 0x140
    InterfaceText *m_tokensValue;                    // 0x144
    OnlineUser *m_nextFriend;                        // 0x148
    InterfaceArea *m_nextFriendAvatarRect;           // 0x14C
    InterfaceGraph *m_nextFriendBg;                  // 0x150
    InterfaceArea *m_nextFriendArea;                 // 0x154
    int m_nextFriendScore;                           // 0x158
    bool m_revivePaid;                               // 0x15C: BuyDeathRevive ran; OnQuit starts the revive sequence
    InterfaceObject *m_socialParent;                 // 0x160: ReviveIncentiveSocial_parent (the challenge)
    InterfaceObject *m_nextFriendParent;             // 0x164
    InterfaceObject *m_topFriendParent;              // 0x168
    InterfaceObject *m_scoreParent;                  // 0x16C
    InterfaceObject *m_missionParent;                // 0x170
    bool m_incentiveShown;                           // 0x174: one of the incentives is shown
    OnlineUser *m_topFriend;                         // 0x178
};

#endif
