// source/game/interfaces/popups/PopupCloudConflict.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPCLOUDCONFLICT_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPCLOUDCONFLICT_H

class PopupCloudConflict : public BasePopup {  // sizeof 0x338
public:
    PopupCloudConflict(GameMessageCCRestore*);
    PopupCloudConflict(PopupCloudConflict::PopupCloudConflictType, social::UserSNS*);
    PopupCloudConflict(PopupCloudConflict::PopupCloudConflictType, social::UserSNS*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~PopupCloudConflict();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
    void Cancel();
    void KeepLocal();
    void KeepRemote();
private:
    static void sOnRemoteUserLoggedIn(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnRemoteUserAuthorized(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static bool sOnUserNames(void*, bool, sociallib::ClientSNSEnum);
    static void sDoKeepLocal(void*);
    static void sDoKeepRemote(void*);
    static void sDoCancel(void*);
    void LoadUserNames();
    InterfaceText *m_titleText;  // 0x1C8
    InterfaceObject *m_content;  // 0x1CC
    InterfaceText *m_remoteScoreText;  // 0x1D0
    InterfaceText *m_remoteBananasText;  // 0x1D4
    InterfaceText *m_remoteTokensText;  // 0x1D8
    InterfaceText *m_remoteTitleText;  // 0x1DC
    InterfaceObject *m_remoteScoreLabel;  // 0x1E0
    InterfaceObject *m_remoteBananasIcon;  // 0x1E4
    InterfaceObject *m_remoteTokensIcon;  // 0x1E8
    InterfaceText *m_remoteMultiplierText;  // 0x1EC
    InterfaceText *m_localScoreText;  // 0x1F0
    InterfaceText *m_localBananasText;  // 0x1F4
    InterfaceText *m_localTokensText;  // 0x1F8
    InterfaceText *m_localTitleText;  // 0x1FC
    InterfaceText *m_localMultiplierText;  // 0x200
    InterfaceText *m_mergeTitle;  // 0x204
    InterfaceButton *m_keepRemoteButton;  // 0x208
    InterfaceButton *m_keepLocalButton;  // 0x20C
    InterfaceButton *m_closeButton;  // 0x210
    InterfaceObject *m_loadingIcon;  // 0x214
    InterfaceButton *m_mergeButton;  // 0x218
    PopupCloudConflict::PopupCloudConflictType m_type;  // 0x21C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_remoteUid;  // 0x220
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_remoteToken;  // 0x240
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_localUid;  // 0x260
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_localToken;  // 0x280
    PopupCloudConflict::State m_state;  // 0x2A0
    safe_enum<CloudSaveGameMgr::ReturnValue,enum CloudSaveGameMgr::ReturnValue::type> m_cloudStatus;  // 0x2A4
    social::UserSNS *m_remoteUser;  // 0x2A8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_localSnsId;  // 0x2AC
    bool m_remoteUserLoggedIn;  // 0x2CC
    bool m_remoteUserLoginError;  // 0x2CD
    bool m_noRemoteData;  // 0x2CE
    bool m_userNamesLoaded;  // 0x2CF
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_localUserName;  // 0x2D0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_remoteUserName;  // 0x2F0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_cCRestoreSaveKey;  // 0x310
    bool m_cCRestoreMandatory;  // 0x330
    PopupCloudConflict::ActionAfterSave m_actionAfterSave;  // 0x334
};

namespace gui {
struct PopupCloudConflict {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_bgFillRect();
    static jet::String &_Deco1();
    static jet::String &_Deco2();
    static jet::String &_BG();
    static jet::String &_Content();
    static jet::String &_RemoteTitle();
    static jet::String &_RemoteButton();
    static jet::String &_RemoteBananasIcon();
    static jet::String &_RemoteTokensIcon();
    static jet::String &_RemoteMultiplierIcon();
    static jet::String &_RemoteScoreLabel();
    static jet::String &_RemoteScoreText();
    static jet::String &_RemoteBananasText();
    static jet::String &_RemoteTokensText();
    static jet::String &_RemoteMultiplier();
    static jet::String &_LocalTitle();
    static jet::String &_LocalButton();
    static jet::String &_LocalBananasIcon();
    static jet::String &_LocalScoreLabel();
    static jet::String &_LocalScoreText();
    static jet::String &_LocalBananasText();
    static jet::String &_LocalTokensText();
    static jet::String &_LocalMultiplier();
    static jet::String &_TitleText();
    static jet::String &_CloseButton();
    static jet::String &_MergeTitle();
    static jet::String &_MergeButton();
    static jet::String &_LoadingIcon();
};
}

#endif
