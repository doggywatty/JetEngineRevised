// source/game/interfaces/menus/box/ShopBuyableItemBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_SHOPBUYABLEITEMBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_SHOPBUYABLEITEMBOX_H

#include "core/pstring.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"

class Buyable;
class Price;
class InterfaceArea;
class InterfaceButton;
class InterfaceGraph;
class InterfaceText;

// An item of the bananas and tokens tabs ("Shop_IAP" layer, 0x1f8 bytes in the .so): a pack of bananas (bought with
// tokens) or of tokens (a purchase in the real game), with what it gives, its price and its promotions. Not ported: the
// sounds (SoundMgr), the tracking of the purchase and the rate game event of a purchase.
class ShopBuyableItemBox : public BasicPage {
public:
    ShopBuyableItemBox(Buyable*, int);
    virtual ~ShopBuyableItemBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
private:
    bool HasPriceChanged();
    void UpdateDisplay();
    void BuyBannanasPack(int, int);
    Buyable *m_buyableItem;      // 0x1bc
    InterfaceObject *m_bg;       // 0x1c0: "Item_box"
    InterfaceArea *m_iconArea;   // 0x1c4
    InterfaceText *m_name;       // 0x1c8
    InterfaceButton *m_buyButton;  // 0x1cc: the one that pays
    InterfaceButton *m_offButton;  // 0x1d0: "BuyButton_off"
    InterfaceGraph *m_waitIcon;  // 0x1d4
    InterfaceObject *m_promo;    // 0x1d8: "ButtonPromo"
    Price *m_itemPrice;          // 0x1dc
    bool m_hasPricePromo;        // 0x1e0
    bool m_hasAmountPromo;       // 0x1e1
    int m_prevAmount;            // 0x1e4
    double m_prevPrice;          // 0x1e8
    jet::String m_itemDescription;  // 0x1f0
};

#endif
