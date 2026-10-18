// source/game/interfaces/CostumeShopItem.h
#ifndef SOURCE_GAME_INTERFACES_COSTUMESHOPITEM_H
#define SOURCE_GAME_INTERFACES_COSTUMESHOPITEM_H

#include <cmath>
#include "core/delayer.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"

class InterfaceArea;
class InterfaceGraph;
class InterfaceText;
class InterfaceButton;
class InterfacePBar;
class MinionCostume;

// The panel of the costume the shop and the main menu show under the costumes: its name, the bonuses, and the buttons to
// buy it (bananas, tokens, a puzzle or a purchase), to wear it or to download it. The "MainMenu_Shop_Item" layer in the
// main menu, "Shop_Item" in the shop. It slides in and out sideways (SetTransition) and, in the main menu, the bonuses
// can be folded open (Expand). 0x258 bytes in the .so.
class CostumeShopItem : public BasicPage {
public:
    explicit CostumeShopItem(bool isMainMenu);
    virtual ~CostumeShopItem();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void UpdateButtonsVisibility();
    void SetCostume(MinionCostume*);
    MinionCostume *GetCostume() const { return m_costume; }
    void SetState(int);
    int GetState() const { return m_state; }
    void SetButtonsActive(bool);
    // inside: the panel is in its place (offset 0), else it is out of the screen, the width of its area aside
    void SetTransition(bool inside);
    bool IsTransitionInside() const { return std::fabs(m_offsetDelayer.GetTargetValue()) <= 1.1920929e-07f; }
    // where the panel's offset is going to: Menu_BaseCostumes reads it as the value at +0x1d8 (the delayer starts at
    // +0x1d0, its origin, current value, then this target)
    float GetOffsetDelayerValue() const { return m_offsetDelayer.GetTargetValue(); }
    // the costume is not in the data and is to be downloaded (its model cannot be loaded, or it is no shop element)
    bool IsDlcRequired() const { return m_isDlcRequired; }
    bool IsExpanding() const;
    void Expand(bool expand, bool byTouch);
    static const int k_offsetDelayTime;
    static const int k_SkillsExpand_open;
    static const int k_SkillsExpand_close;
    // set when an in-app purchase of a costume went through (Price::DeliverIAPItem @0x409578); Update takes it
    static bool sBoughtCustomes;
private:
    void BuyCostume(int, int);
    void OnBuyCostomeSuccess();
    void HandleSound();
    static void ConfirmDownloadDlc(void*);
    void downloadDlc();
    bool m_isMainMenu;  // 0x1BC
    int m_state;  // 0x1C0
    InterfaceArea *m_area;  // 0x1C4: "Costume_UI_Area", what slides
    float m_width;  // 0x1C8: of the area, how far out of the screen the panel goes
    float m_areaOffsetY;  // 0x1CC
    jet::core::Delayer<float,jet::core::SmootherStep> m_offsetDelayer;  // 0x1D0: the area's offset x (origin, current, target)
    InterfaceText *m_name;  // 0x1E8
    InterfaceGraph *m_nameBox;  // 0x1EC
    InterfaceButton *m_buyButton;  // 0x1F0
    InterfaceButton *m_puzzleButton;  // 0x1F4
    int m_unused1F8[3];  // 0x1F8: not used by the class
    InterfaceButton *m_equipButton;  // 0x204: also the download button
    InterfaceText *m_equippedText;  // 0x208
    InterfacePBar *m_downloadBar;  // 0x20C: over the equip button while the costume is downloaded
    InterfaceGraph *m_iconBananas;  // 0x210
    InterfaceGraph *m_iconTokens;  // 0x214
    InterfaceText *m_priceOld;  // 0x218
    InterfaceText *m_priceNew;  // 0x21C
    InterfaceGraph *m_priceStroke;  // 0x220
    InterfaceGraph *m_promoTag;  // 0x224
    InterfaceText *m_promoValue;  // 0x228
    MinionCostume *m_costume;  // 0x22C
    bool m_isDlcRequired;  // 0x230
    bool m_unused231;  // 0x231
    InterfaceText *m_bonus1;  // 0x234: the bonus text, in the folded panel
    InterfaceText *m_bonus2;  // 0x238: the bonus text, in the panel with the multiplier's place
    InterfaceText *m_multiplier;  // 0x23C: the despicable points multiplier
    InterfaceButton *m_expandButton;  // 0x240: main menu only
    InterfaceGraph *m_itemButtonBox;  // 0x244: main menu only
    bool m_expanded;  // 0x248
    bool m_expandByTouch;  // 0x249: once opened or closed by a touch, only a touch does
    MinionCostume *m_buyingCostume;  // 0x24C: the one an in-app purchase is for
    bool m_smallBonusText;  // 0x250: German and Russian on a 480 pixels wide window need the bonus text smaller
    InterfaceGraph *m_waitAnim;  // 0x254: shown while a purchase goes on
};

#endif
