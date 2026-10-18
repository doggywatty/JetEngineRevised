// source/game/interfaces/menus/subpages/OptionsConnectionPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSCONNECTIONPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSCONNECTIONPAGE_H

class OptionsConnectionPage : public BasicPage {  // sizeof 0x220
public:
    OptionsConnectionPage();
    virtual ~OptionsConnectionPage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Refresh();
    static bool CanShowDLCNotify();
private:
    void LoadUserNames();
    static bool sOnSNSName(void*, bool, sociallib::ClientSNSEnum);
    InterfaceButton *m_facebookButton;  // 0x1C4
    InterfaceObject *m_gameCenterConnected;  // 0x1C8
    InterfaceObject *m_gameCenterDisconnected;  // 0x1CC
    InterfaceButton *m_downloadDlcButton;  // 0x1D0
    InterfaceButton *m_cancelDlcButton;  // 0x1D4
    InterfacePBar *m_dlcProgressBar;  // 0x1D8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_facebookCredential;  // 0x1DC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_gamecenterCredential;  // 0x1FC
    bool m_refreshOnNextFrame;  // 0x21C
};

#endif
