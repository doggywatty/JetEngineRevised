// libs/social_framework/include/social/userosiris.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USEROSIRIS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USEROSIRIS_H

namespace social {
class UserOsiris : public social::User {  // sizeof 0xDB0
public:
    virtual ~UserOsiris();
    void InitAnonymous();
    void AddCredential(social::UserSNS*, social::UserOsiris::AccountLinkType);
    void RemoveCredential(social::UserSNS*);
    virtual void Login(bool);
    void LoginToAnonymous();
    virtual bool LogoutImpl();
    gaia::BaseServiceManager::Credentials GetCredentials();
    bool IsLoggedInTo(sociallib::ClientSNSEnum);
    bool IsLoggedInToSNS();
    social::UserSNS *GetCredential(sociallib::ClientSNSEnum);
    social::UserSNS *GetMainCredential();
    social::UserSNS *GetLoggedInCredential();
    virtual social::Profile *GetProfile();
    virtual social::Friends *GetFriends();
    virtual social::Avatar *GetAvatar();
    social::BinaryData *GetData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual social::Wall *GetWall();
    virtual social::GameProfile *GetGameProfile();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetSimpleUid();
    virtual social::Inbox *GetInbox();
    social::InboxHermesSecure *GetSecureInbox();
    social::RequestBox *GetRequestBox();
    virtual social::Achievements *GetAchievements();
    virtual social::Activity *GetActivity();
    virtual social::StandardProfile *GetStandardProfile();
    static gaia::BaseServiceManager::Credentials GetCredentialsFromUid(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static sociallib::ClientSNSEnum GetSNSFromUid(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSimpleUidfromUid(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void LoginCancel();
    bool IsSameUser(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void CheckLoginConflicts(social::UserSNS*);
    bool CheckLoginConflictsFast(social::UserSNS*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetLinkedCredentialUid(sociallib::ClientSNSEnum);
    bool IsImportingProfile();
    bool IsImportingFriends();
    const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetCredentialList();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetAccountId();
protected:
    UserOsiris(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    UserOsiris();
private:
    social::Batcher *GetBatch();
    void RequestProfile();
    void RequestCredentialList();
    void RequestAlias();
    void ImportGameCenter();
    void loginOsiris();
    void LinkUser();
    void LinkCredential(social::UserSNS*, social::UserOsiris::AccountLinkType);
    static void sOnOsirisLogin(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    void OnOsirisLogin(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    static void sOnAccountLinked(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    void OnAccountLinked(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    static void sOnProfileImported(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnfriendsImported(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    void OnfriendsImported(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    void ImportProfile(social::UserSNS*);
    static bool sOnProfileLoaded(void*, bool, social::OnlineEventData);
    static void sOnCredentialList(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnAliasLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void nullCallback(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnCredentialLinked(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static bool sOnremoveCredentialLoggedOut(void*, bool, social::OnlineEventData);
    static bool sOnremoveCredentialLoggedIn(void*, bool, social::OnlineEventData);
    static void sOnCheckIfAccountExists(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnCheckLoginConflicts(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static bool sOnGameCenterFriendsLoaded(void*, bool, social::OnlineEventData);
    static void sOnGameCenterListConnections(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnGameCenterListRequests(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnGameCenterConnectionAdded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnGameCenterProfileUpdated(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::map<enum sociallib::ClientSNSEnum,social::UserSNS *,std::less<enum sociallib::ClientSNSEnum>,std::allocator<std::pair<enum sociallib::ClientSNSEnum const ,social::UserSNS *> > > m_profiles;  // 0x70
    social::UserSNS *m_mainProfile;  // 0x8C
    std::list<social::UserSNS *,std::allocator<social::UserSNS *> > m_secondaryProfiles;  // 0x90
    std::list<social::UserSNS *,std::allocator<social::UserSNS *> > m_secondaryProfilesToRelogin;  // 0xA8
    std::list<social::UserSNS *,std::allocator<social::UserSNS *> > m_linkedProfiles;  // 0xC0
    std::list<social::UserSNS *,std::allocator<social::UserSNS *> > m_unlinkedProfiles;  // 0xD8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_simpleUid;  // 0xF0
    int m_importedFriends;  // 0x110
    int m_linkedProfilesCount;  // 0x114
    social::ProfileOsiris m_profile;  // 0x118
    social::FriendsOsiris m_friends;  // 0x20C
    social::AvatarOsiris m_avatar;  // 0x33C
    social::WallOsiris m_wall;  // 0x45C
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::BinaryDataSeshat *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::BinaryDataSeshat *> > > m_binaryData;  // 0x5A8
    social::InboxHermes m_inbox;  // 0x5C4
    social::InboxHermesSecure m_secureBox;  // 0x6FC
    social::GameProfile m_gameProfile;  // 0x814
    social::RequestBox m_requestBox;  // 0xA88
    social::UserSNS *m_linkingCredential;  // 0xB9C
    social::StandardProfile m_standardProfile;  // 0xBA0
    bool m_aborted;  // 0xCB0
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_credentialList;  // 0xCB4
    social::UserSNS *m_checkingSnsUser;  // 0xCC8
    int m_importingProfiles;  // 0xCCC
    int m_importingFriends;  // 0xCD0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_accountId;  // 0xCD4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_aliasResponse;  // 0xCF4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_alias;  // 0xD14
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_dummyResponses;  // 0xD34
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_credentialCheckResponses;  // 0xD48
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_credentialListResponses;  // 0xD5C
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_checkLoginConflictsResponses;  // 0xD70
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_requestListResponses;  // 0xD84
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_requestListConnections;  // 0xD98
};
}

#endif
