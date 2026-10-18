// libs/social_framework/include/social/snsmanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_SNSMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_SNSMANAGER_H

namespace social {
class SNSManager : public social::SSingleton<social::SNSManager>, public social::SimpleEventDispatcher<enum sociallib::ClientSNSEnum> {  // sizeof 0xB4
public:
    SNSManager();
    virtual ~SNSManager();
    void SetDependencyTag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::TSNSData);
    social::SNSDataCache *GetSNSDataCache(sociallib::ClientSNSEnum);
    void Update();
    bool CanUpdateGame();
    void Share(sociallib::ClientSNSEnum, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, social::SNSManager::ShareInfo::SNSShareInfoType);
    void Share(sociallib::ClientSNSEnum, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::SNSManager::ShareInfo::SNSShareInfoType);
    int GetFriendData(sociallib::ClientSNSEnum, const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    void GetUid(sociallib::ClientSNSEnum);
    void GetName(sociallib::ClientSNSEnum);
    void Login(sociallib::ClientSNSEnum, bool);
    bool IsLoggedIn(sociallib::ClientSNSEnum);
    void Logout(sociallib::ClientSNSEnum);
    void GetFriendsUidList(sociallib::ClientSNSEnum, sociallib::FriendTypeEnum);
    int GetFriendsData(sociallib::ClientSNSEnum, const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, bool);
    void GetFriendNames(sociallib::ClientSNSEnum, std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    void SendMsg(sociallib::ClientSNSEnum, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    void SendMsg(sociallib::ClientSNSEnum, const social::SNSManager::Message&);
    bool IsSNSSupported(sociallib::ClientSNSEnum);
    bool IsBusy();
    void InitSNS(sociallib::ClientSNSEnum);
    bool IsSNSInitialized(sociallib::ClientSNSEnum);
    void RegisterForApplePushNotifications(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void CancelPendingRequests();
    void PostAchievement(sociallib::ClientSNSEnum, int);
    void RegisterEventListener(social::SNSEvent, sociallib::ClientSNSEnum, bool (void*, bool, sociallib::ClientSNSEnum)*, void*);
    void Dispatch(social::SNSEvent, bool, sociallib::ClientSNSEnum);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret(sociallib::ClientSNSEnum);
    void GetAvatar(sociallib::ClientSNSEnum);
    void SendScore(sociallib::ClientSNSEnum, int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    void FindDependencies(social::SNSManager::ShareInfo&);
    void FindDependenciesInString(social::SNSManager::ShareInfo&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void StartSharing(social::SNSManager::ShareInfo&);
    bool ResolveShare(social::SNSManager::ShareInfo&);
    void ResolveDependency(social::SNSManager::ShareInfo&, social::TSNSData);
    void PostToWall(social::SNSManager::ShareInfo&);
    void PostToFriendWall(social::SNSManager::ShareInfo&);
    void SendNotification(social::SNSManager::ShareInfo&);
    void SendInvitationToFriends(social::SNSManager::ShareInfo&);
    void FinishCurrentShare(bool);
    void OnFacebookGetName();
    void OnGetUidForRegister(bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::ServerSystem);
    static void sOnScoreSent(bool, void*);
    std::deque<social::SNSManager::ShareInfo,std::allocator<social::SNSManager::ShareInfo> > m_shares;  // 0x24
    std::map<enum sociallib::ClientSNSEnum,social::SNSDataCache *,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,social::SNSDataCache *> > > m_snsCaches;  // 0x40
    std::map<enum social::TSNSData,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<enum social::TSNSData>,std::allocator<std::pair<enum social::TSNSData const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_dependencyTags;  // 0x5C
    bool m_currentFriendRequestIdOsPlaying;  // 0x78
    std::map<enum sociallib::ClientSNSEnum,std::list<unsigned int,std::allocator<unsigned int> >,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,std::list<unsigned int,std::allocator<unsigned int> > > > > m_snsRequestCounters;  // 0x7C
    std::map<enum sociallib::ClientSNSEnum,std::list<std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > >,std::allocator<std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > > > >,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,std::list<std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > >,std::allocator<std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > > > > > > > m_snsRequestDataCache;  // 0x98
};
}

#endif
