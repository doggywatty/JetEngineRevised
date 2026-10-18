// source/game/interfaces/popups/PopupPrizeMessage.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPPRIZEMESSAGE_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPPRIZEMESSAGE_H

class PopupPrizeMessage : public BasePopup {  // sizeof 0x208
public:
    PopupPrizeMessage(GameMessageGift*);
    virtual ~PopupPrizeMessage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
private:
    InterfaceButton *m_okButton;  // 0x1C8
    InterfaceText *m_cCMessageText;  // 0x1CC
    InterfaceText *m_weeklyContestInfoText;  // 0x1D0
    InterfaceList *m_prizeList;  // 0x1D4
    int m_messageType;  // 0x1D8
    std::vector<GameMessageGift::MessageGift,std::allocator<GameMessageGift::MessageGift> > m_giftList;  // 0x1DC
    jet::String m_giftMessage;  // 0x1F0
    std::vector<PrizeBox *,std::allocator<PrizeBox *> > m_prizes;  // 0x1F4
};

#endif
