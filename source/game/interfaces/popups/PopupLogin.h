// source/game/interfaces/popups/PopupLogin.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPLOGIN_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPLOGIN_H

class PopupLogin : public BasePopup {  // sizeof 0x1E8
public:
    PopupLogin();
    virtual ~PopupLogin();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
    void RefreshLoginPopup();
private:
    InterfaceText *m_titleText;  // 0x1C8
    InterfaceButton *m_closeButton;  // 0x1CC
    InterfaceButton *m_facebookButton;  // 0x1D0
    InterfaceButton *m_gameCenterButton;  // 0x1D4
    InterfaceGraph *m_facebookIcon;  // 0x1D8
    InterfaceGraph *m_gameCenterIcon;  // 0x1DC
    InterfaceGraph *m_facebookConnected;  // 0x1E0
    InterfaceGraph *m_gameCenterConnected;  // 0x1E4
};

#endif
