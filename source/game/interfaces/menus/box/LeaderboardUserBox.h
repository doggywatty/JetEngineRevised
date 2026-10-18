// source/game/interfaces/menus/box/LeaderboardUserBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_LEADERBOARDUSERBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_LEADERBOARDUSERBOX_H

class LeaderboardUserBox : public BasicPage {  // sizeof 0x1FC
public:
    LeaderboardUserBox(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int, bool);
    virtual ~LeaderboardUserBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetState(int);
    int GetState();
    bool SendChallenge();
protected:
    void Refresh();
    int m_state;  // 0x1C4
    bool m_isPlayer;  // 0x1C8
    InterfaceText *m_name;  // 0x1CC
    InterfaceText *m_rank;  // 0x1D0
    InterfaceText *m_points;  // 0x1D4
    InterfaceButton *m_sendChallengeButton;  // 0x1D8
    InterfaceButton *m_collectPrizeButton;  // 0x1DC
    InterfaceObject *m_photoArea;  // 0x1E0
    InterfaceObject *m_challanged;  // 0x1E4
    InterfaceObject *m_bgFriend;  // 0x1E8
    InterfaceObject *m_bgPlayer;  // 0x1EC
    int m_leaderboardUserIdx;  // 0x1F0
    LeaderboardMgr::LeaderboardType m_leaderboardType;  // 0x1F4
    LeaderboardMgr::LeaderboardRange m_leaderboardRange;  // 0x1F8
};

#endif
