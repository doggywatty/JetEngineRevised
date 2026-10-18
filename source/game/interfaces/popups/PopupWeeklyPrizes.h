// source/game/interfaces/popups/PopupWeeklyPrizes.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPWEEKLYPRIZES_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPWEEKLYPRIZES_H

class PopupWeeklyPrizes : public BasePopup {  // sizeof 0x1E8
public:
    PopupWeeklyPrizes();
    virtual ~PopupWeeklyPrizes();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    void ClearPrizesArray();
    InterfaceList *m_prizeList;  // 0x1C8
    InterfaceList *m_prizeConsolationList;  // 0x1CC
    InterfaceButton *m_closeButton;  // 0x1D0
    std::vector<WeeklyPrizeBox *,std::allocator<WeeklyPrizeBox *> > m_prizes;  // 0x1D4
};

#endif
