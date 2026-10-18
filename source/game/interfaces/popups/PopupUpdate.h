// source/game/interfaces/popups/PopupUpdate.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPUPDATE_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPUPDATE_H

class PopupUpdate : public BasePopup {  // sizeof 0x1DC
public:
    PopupUpdate(bool);
    virtual ~PopupUpdate();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
private:
    bool m_mandatory;  // 0x1C8
    InterfaceButton *m_okButton;  // 0x1CC
    InterfaceButton *m_cancelButton;  // 0x1D0
    InterfaceText *m_descText;  // 0x1D4
    InterfaceObject *m_loadingIcon;  // 0x1D8
};

#endif
