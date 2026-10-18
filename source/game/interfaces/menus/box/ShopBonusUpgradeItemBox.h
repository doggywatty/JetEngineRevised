// source/game/interfaces/menus/box/ShopBonusUpgradeItemBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_SHOPBONUSUPGRADEITEMBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_SHOPBONUSUPGRADEITEMBOX_H

#include "core/pstring.h"
#include "game/entities/gameitem/GameItemType.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"

class Bonus;
class InterfaceButton;
class Interface3DElement;

// An item of the bonus tab ("Shop_Item_Upgrade" layer, 0x1f8 bytes in the .so): a vehicle or power-up with the level it
// is at, what its next level gives and the price of it, which the buy button pays. Touching the item opens its
// description. Not ported: the sounds (SoundMgr), the tracking of the purchase, the blind box item (BlindBox entities,
// its prize popup) and the real-money purchase's own flow (it is delivered at once, BonusUpgradeMgr::HandleDeliverIAPItem).
class ShopBonusUpgradeItemBox : public BasicPage {
public:
    explicit ShopBonusUpgradeItemBox(Bonus*);
    virtual ~ShopBonusUpgradeItemBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    void EnableExpandedInfo(bool);                // @0x4da378: the description opened (the item is taller) or closed
    void EnableTouches(bool);                     // @0x4da80c
    bool IsBuyButton(InterfaceButton *b) const { return b == m_buyButton; }  // @0x4da9c8
    math::vec2<float> GetTutorialPos();           // @0x4da9dc
    GameItemType GetGameItemType() const;         // @0x4daa9c
    // set when an upgrade was bought in the shop (@0xd0d028)
    static bool checkClickBuyOnShop;
private:
    void Refresh();                               // @0x4d8f60
    bool IsRefreshNeeded();                       // @0x4da840
    void BuyUpgrade(int, int);                    // @0x4daad8
    Bonus *m_bonus;                               // 0x1bc
    int m_shownLevel;                             // 0x1c0: the level the item shows
    bool m_hasPricePromo;                         // 0x1c4
    double m_prevPrice;                           // 0x1c8
    bool m_hasAmountPromo;                        // 0x1d0
    int m_prevAmount;                             // 0x1d4
    bool m_purchaseInProgress;                    // 0x1d8: the real-money purchase of the price goes on
    bool m_unused1d9;                             // 0x1d9
    jet::String m_itemDescription;                // 0x1dc
    InterfaceButton *m_buyButton;                 // 0x1e0
    InterfaceButton *m_buyOffButton;              // 0x1e4
    InterfaceButton *m_expandButton;              // 0x1e8
    InterfaceButton *m_collapseButton;            // 0x1ec: the description, to close it
    InterfaceButton *m_openButton;                // 0x1f0
    Interface3DElement *m_icon3d;                 // 0x1f4
};

#endif
