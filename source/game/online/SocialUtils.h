// source/game/online/SocialUtils.h
#ifndef SOURCE_GAME_ONLINE_SOCIALUTILS_H
#define SOURCE_GAME_ONLINE_SOCIALUTILS_H

class SocialUtils {  // sizeof 0x3C
public:
    SocialUtils();
    ~SocialUtils();
    void LoginToSNS(sociallib::ClientSNSEnum);
    void LogoutFromSNS(sociallib::ClientSNSEnum);
    void PerformSNSLogin();
    static bool sOnSocialLoggedIn(void*, bool, social::OnlineEventData);
    void LoginAndStealSNS();
    void LoginAndMergeSNS();
    void ReverseLoginAndStealAnonymous();
    void ReverseLoginAndMerge();
    void CancelSNSLogin();
    void StartAutoLogin(const std::vector<enum sociallib::ClientSNSEnum,std::allocator<enum sociallib::ClientSNSEnum> >&);
    bool IsAutoLogginIn();
    bool IsBusy();
    bool HasPendingSnsToLogin();
private:
    void LoginToNextAutoLoginSNS();
    void LoginToOsiris();
    void CancelAutoLogin();
    void PerformSocialLogin(social::UserOsiris::AccountLinkType);
    static bool sOnSNSLoggedIn(void*, bool, social::OnlineEventData);
    static bool sOnSNSLoggedOut(void*, bool, social::OnlineEventData);
    static void CancelLoginSNS(void*);
    static void CancelLoginOsiris(void*);
    static bool sOnProfile(void*, bool, social::OnlineEventData);
    static bool sOnLoginConflictsChecked(void*, bool, social::OnlineEventData);
    static bool sOnSocialLogoutForReverseLogin(void*, bool, social::OnlineEventData);
    static bool sOnCredentialAdded(void*, bool, social::OnlineEventData);
    static bool _sOnSocialLoggedIn(void*, bool, social::OnlineEventData);
    SocialUtils::Status m_status;  // 0x0
    sociallib::ClientSNSEnum m_connectingSns;  // 0x4
    social::UserSNS *m_currentUser;  // 0x8
    social::UserSNS *m_trackUser;  // 0xC
    std::vector<enum sociallib::ClientSNSEnum,std::allocator<enum sociallib::ClientSNSEnum> > m_autoLoginSnsList;  // 0x10
    bool m_autoLogginIn;  // 0x24
    bool m_refreshOnlineDataAfterLogout;  // 0x25
    bool m_isReverseLoginAndSteal;  // 0x26
    bool m_isReverseLoginAndMerge;  // 0x27
    std::vector<enum sociallib::ClientSNSEnum,std::allocator<enum sociallib::ClientSNSEnum> > m_pendingSNSToLogin;  // 0x28
};

#endif
