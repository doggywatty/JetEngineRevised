// source/game/interfaces/popups/PopupFBInit.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPFBINIT_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPFBINIT_H

class PopupFBInit : public BasePopup {  // sizeof 0x1D8
public:
    PopupFBInit();
    virtual ~PopupFBInit();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
protected:
    void HandleButtons();
    InterfaceGraph *m_windows;  // 0x1C8
    InterfaceButton *m_okButton;  // 0x1CC
    InterfaceButton *m_notNowButton;  // 0x1D0
    InterfaceText *m_tokenValue;  // 0x1D4
};

#endif
