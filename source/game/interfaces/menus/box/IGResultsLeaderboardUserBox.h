// source/game/interfaces/menus/box/IGResultsLeaderboardUserBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_IGRESULTSLEADERBOARDUSERBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_IGRESULTSLEADERBOARDUSERBOX_H

class IGResultsLeaderboardUserBox : public BasicPage {  // sizeof 0x1F8
public:
    IGResultsLeaderboardUserBox(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    virtual ~IGResultsLeaderboardUserBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetState(int);
    int GetState();
    bool SendChallenge();
protected:
    void Refresh();
    int m_state;  // 0x1C4
    InterfaceText *m_name;  // 0x1C8
    InterfaceText *m_rank;  // 0x1CC
    InterfaceText *m_points;  // 0x1D0
    InterfaceButton *m_sendChallengeButton;  // 0x1D4
    InterfaceButton *m_collectPrizeButton;  // 0x1D8
    InterfaceObject *m_photoArea;  // 0x1DC
    InterfaceObject *m_challenged;  // 0x1E0
    InterfaceObject *m_bgFriend;  // 0x1E4
    InterfaceObject *m_bgPlayer;  // 0x1E8
    int m_leaderboardUserIdx;  // 0x1EC
    LeaderboardMgr::LeaderboardType m_leaderboardType;  // 0x1F0
    LeaderboardMgr::LeaderboardRange m_leaderboardRange;  // 0x1F4
};

#endif
