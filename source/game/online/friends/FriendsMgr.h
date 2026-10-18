// source/game/online/friends/FriendsMgr.h
#ifndef SOURCE_GAME_ONLINE_FRIENDS_FRIENDSMGR_H
#define SOURCE_GAME_ONLINE_FRIENDS_FRIENDSMGR_H

class FriendsMgr : public Singleton<FriendsMgr>, public GameSaveable<FriendsSaveData> {  // sizeof 0xA8
public:
    FriendsMgr();
    virtual ~FriendsMgr();
    void Init();
    void Update();
    bool IsLoadingFriends();
    int GetNumFriendsLoaded();
    void SetLoadingFriends(bool);
    float GetLoadingProgression();
    OnlineUser *GetOnlineUser(int);
    int GetFriendId(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::map<int,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<int>,std::allocator<std::pair<int const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > &GetAllFriends();
    void CleanOnlineFriends();
    int AddFriend(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ChallengeFriend(int);
    void RefreshChallengedFriends();
    void ResetChallengeCompleted(int);
    bool IsChallengeCompleted(int);
    const Price *GetFriendChallengeCompletedPrize(int);
    void SendSocialPNToUsers(const std::list<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, online::PushNotificationsMgr::PushNot_Types);
    void StartLoadingNonPlayingFriends();
    bool IsLoadingNonPlayingFriends();
    bool HasErrorLoadingNonPlayingFriends();
    float GetLoadingNonPlayingFriendsProgression();
    void CleanNonPlayingOnlineFriends();
    int GetNumNonPlayingFriendsLoaded();
    int GetNumNonPlayingFriendsNotInvited();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetNonPlayingFriend(int);
    void InviteNonPlayingFriend(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    void InviteAllNonPlayingFriends();
    bool IsNonPlayingFriendInvited(int);
    virtual void ResetData(FriendsSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, FriendsSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, FriendsSaveData&);
private:
    void _RefreshChallengedFriends();
    void SendGetGamePNToAllFriends();
    static bool sOnNonPlayingFriendsLoaded(void*, bool, social::OnlineEventData);
    static bool sOnInboxSaved(void*, bool, social::OnlineEventData);
    void SerializeV1(jet::stream::IStreamW&, FriendsSaveData&);
    bool DeserializeV1(jet::stream::IStream&, FriendsSaveData&);
public:
    static const bool k_debug_enableUserAsFriend;
private:
    bool m_loadingFriends;  // 0x30
    float m_loadingProgression;  // 0x34
    bool m_loadingNonPlayingFriends;  // 0x38
    float m_loadingNonPlayingFriendsProgression;  // 0x3C
    std::map<int,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<int>,std::allocator<std::pair<int const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_friendsMap;  // 0x40
    int m_nextFriendIdCount;  // 0x5C
    int m_numFriendsLoaded;  // 0x60
    std::vector<int,std::allocator<int> > m_friendsToChallenge;  // 0x64
    bool m_refreshingChallengedFriends;  // 0x78
    bool m_errorLoadingNonPlayingFriends;  // 0x79
    static const int k_saveGameVersion;
    std::vector<FriendsMgr::NonPlayingFriendData,std::allocator<FriendsMgr::NonPlayingFriendData> > m_nonPlayingFriends;  // 0x7C
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_friendsToInvite;  // 0x90
    bool m_pnSentToFriends;  // 0xA4
    static const int k_signatureV1;
};

#endif
