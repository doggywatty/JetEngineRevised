// source/game/interfaces/popups/PopupLoginToFacebook.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPLOGINTOFACEBOOK_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPLOGINTOFACEBOOK_H

class PopupLoginToFacebook : public BasePopup {  // sizeof 0x1D0
public:
    PopupLoginToFacebook();
    virtual ~PopupLoginToFacebook();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
private:
    InterfaceButton *m_cancelButton;  // 0x1C8
    InterfaceButton *m_loginButton;  // 0x1CC
};

#endif
