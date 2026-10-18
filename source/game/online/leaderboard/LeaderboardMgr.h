// source/game/online/leaderboard/LeaderboardMgr.h
#ifndef SOURCE_GAME_ONLINE_LEADERBOARD_LEADERBOARDMGR_H
#define SOURCE_GAME_ONLINE_LEADERBOARD_LEADERBOARDMGR_H

class LeaderboardMgr : public Singleton<LeaderboardMgr> {  // sizeof 0x23C
public:
    LeaderboardMgr();
    virtual ~LeaderboardMgr();
    void Init();
    void Update(int);
    void StartLoadingLeaderboard(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    bool IsLoadingLeaderboard(LeaderboardMgr::LeaderboardType);
    bool IsLoadingLeaderboard(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    bool HasLoadError(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    int GetNumUsersLoaded(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    float GetLoadingProgression(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    OnlineUser *GetOnlineUser(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    bool RegisterPlayerScore(LeaderboardMgr::LeaderboardType, int, bool);
    int GetLeaderboardIdx(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    OnlineUser *GetOnlineFriendsArroundPlayer(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    void CleanLeaderboards();
    void SetLeaderboardName(LeaderboardMgr::LeaderboardType, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsUserInLeaderboard(OnlineUser*, LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    bool IsPlayer(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    void RefreshPlayerScore(LeaderboardMgr::LeaderboardType, int, bool);
    void RefreshPlayerScore(LeaderboardMgr::LeaderboardType, bool);
    void RefreshPlayerScore(bool);
    bool IsRankInLeaderboard(int, LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    int GetUserPos(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    int GetUserScore(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, int);
    void RemovePosts(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsRemovingPosts();
private:
    void FinishLoadingLeaderboard(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    void CleanLeaderboard(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    void LoadNextRange(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    void StoreItems(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    void AddUserToLeaderboard(LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, const char*, bool, int);
    void SetPlayer(int, LeaderboardMgr::LeaderboardType, LeaderboardMgr::LeaderboardRange);
    std::vector<LeaderboardMgr::LeaderboardUserData,std::allocator<LeaderboardMgr::LeaderboardUserData> > m_onlineUserList[3][2];  // 0x4
    bool m_loadingLeaderboard[3][2];  // 0x7C
    bool m_savingLeaderboard[3];  // 0x82
    bool m_removingPosts[3];  // 0x85
    float m_loadingProgression[3][2];  // 0x88
    LeaderboardMgr::LoadingStep m_loadingStep[3][2];  // 0xA0
    bool m_loadingError[3][2];  // 0xB8
    bool m_savingError[3];  // 0xBE
    int m_numLoadingRetries[3][2];  // 0xC4
    int m_loadingRetryTimer[3][2];  // 0xDC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_leaderboardPostScript[3];  // 0xF4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_leaderboardName[3];  // 0x154
    social::Leaderboard *m_leaderboardWorld[3];  // 0x1B4
    social::FriendsLeaderboard *m_leaderboardFriends[3];  // 0x1C0
    social::LeaderboardRangeHandle m_leaderboardHandle[3][2];  // 0x1CC
    std::vector<social::Leaderboard *,std::allocator<social::Leaderboard *> > m_leaderboardToDelete;  // 0x214
    std::vector<social::FriendsLeaderboard *,std::allocator<social::FriendsLeaderboard *> > m_friendsLeaderboardToDelete;  // 0x228
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_leaderboardPostScript_distance;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_leaderboardPostScript_points;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_leaderboardPostScript_weekly;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_leaderboardName_distance;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_leaderboardName_points;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_gamecenterLeaderboardCategory;
    static const int k_maxUsersInWorldRange;
    static const int k_maxUsersInPage;
    static const int k_maxUsersAroundPlayer;
    static const int k_initialRetryTimer;
    static const int k_maxRetryTime;
};

#endif
