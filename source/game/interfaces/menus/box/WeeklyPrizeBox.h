// source/game/interfaces/menus/box/WeeklyPrizeBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_WEEKLYPRIZEBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_WEEKLYPRIZEBOX_H

class WeeklyPrizeBox : public BasicPage {  // sizeof 0x1D0
public:
    WeeklyPrizeBox(const WeeklyChallengePrizeInfo&, bool);
    virtual ~WeeklyPrizeBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
protected:
    InterfaceText *m_place;  // 0x1C4
    InterfaceGraph *m_tokenIcon;  // 0x1C8
    InterfaceGraph *m_bananasIcon;  // 0x1CC
};

namespace gui {
struct WeeklyPrizeBox {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Size();
    static jet::String &_PrizeText();
    static jet::String &_PrizeType();
    static jet::String &_BananasIcon();
    static jet::String &_TokensIcon();
    static jet::String &_CheckIcon();
};
}

#endif
