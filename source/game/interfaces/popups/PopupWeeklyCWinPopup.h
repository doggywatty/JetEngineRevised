// source/game/interfaces/popups/PopupWeeklyCWinPopup.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPWEEKLYCWINPOPUP_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPWEEKLYCWINPOPUP_H

class PopupWeeklyCWinPopup : public BasePopup {  // sizeof 0x1D4
public:
    PopupWeeklyCWinPopup(const WeeklyChallengePrizeInfo&);
    virtual ~PopupWeeklyCWinPopup();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
private:
    WeeklyChallengePrizeInfo *m_challengePrizeInfo;  // 0x1C8
    WeeklyPrizeBox *m_prizeBox;  // 0x1CC
    InterfaceButton *m_claimButton;  // 0x1D0
};

#endif
