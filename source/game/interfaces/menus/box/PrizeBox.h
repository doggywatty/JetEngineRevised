// source/game/interfaces/menus/box/PrizeBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_PRIZEBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_PRIZEBOX_H

class PrizeBox : public BasicPage {  // sizeof 0x1D0
public:
    PrizeBox(PrizeBox::PrizeType, const jet::String&);
    virtual ~PrizeBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
protected:
    InterfaceText *m_amount;  // 0x1C4
    InterfaceGraph *m_tokenIcon;  // 0x1C8
    InterfaceGraph *m_bananasIcon;  // 0x1CC
};

namespace gui {
struct PrizeBox {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Size();
    static jet::String &_AmountText();
    static jet::String &_TokensIcon();
    static jet::String &_BananasIcon();
};
}

#endif
