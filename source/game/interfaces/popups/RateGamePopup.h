// source/game/interfaces/popups/RateGamePopup.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_RATEGAMEPOPUP_H
#define SOURCE_GAME_INTERFACES_POPUPS_RATEGAMEPOPUP_H

class RateGamePopup : public BasePopup {  // sizeof 0x1D0
public:
    RateGamePopup();
    virtual ~RateGamePopup();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
protected:
    void HandleButtons();
    InterfaceButton *m_okButton;  // 0x1C8
    InterfaceButton *m_notNowButton;  // 0x1CC
};

#endif
