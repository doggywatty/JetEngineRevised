// source/game/interfaces/menus/subpages/EndScreenLeaderboardPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENLEADERBOARDPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENLEADERBOARDPAGE_H

class EndScreenLeaderboardPage : public EndScreenBasePage {  // sizeof 0x208
public:
    virtual const jet::String &GetUITemplateName();
    EndScreenLeaderboardPage();
    virtual ~EndScreenLeaderboardPage();
    virtual void Update(int);
    virtual void Refresh();
    virtual bool ShouldResetChildren();
private:
    void ClearLeaderboard();
    void PopulateLeaderboard();
    void LoadText();
public:
    static const jet::String k_uiTmplName;
private:
    bool m_refreshOnNextFrame;  // 0x1C8
    InterfaceObject *m_loginBg;  // 0x1CC
    InterfaceText *m_loginTitle;  // 0x1D0
    InterfaceButton *m_loginFacebookButton;  // 0x1D4
    InterfaceObject *m_loginGamecenterGraph;  // 0x1D8
    InterfaceText *m_facebookReward;  // 0x1DC
    bool m_isLoadingLeaderboard;  // 0x1E0
    InterfaceGrid *m_leaderboardGrid;  // 0x1E4
    InterfaceGraph *m_loadingIcon;  // 0x1E8
    std::vector<IGResultsLeaderboardUserBox *,std::allocator<IGResultsLeaderboardUserBox *> > m_leaderboardBoxList;  // 0x1EC
    LeaderboardMgr::LeaderboardRange m_leaderboardRange;  // 0x200
    LeaderboardMgr::LeaderboardType m_leaderboardType;  // 0x204
};

#endif
