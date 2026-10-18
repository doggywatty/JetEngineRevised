// source/game/interfaces/menus/Menu_Shop.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_SHOP_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_SHOP_H

#include <vector>
#include "Menu_BaseCostumes.h"

class BananasAndTokensBox;
class InterfaceButton;
class InterfaceList;
class InterfaceText;
class InterfaceGraph;
class ShopBuyableItemBox;
class ShopBonusUpgradeItemBox;

// The shop ("Shop_Common" layer): the tabs of bananas, tokens, costumes (the wheel of Menu_BaseCostumes) and bonus
// upgrades under a header, a back button and the counters; the bananas, tokens and bonus tabs fill a list with boxes
// (ShopBuyableItemBox, ShopBonusUpgradeItemBox). 0x2c0 bytes in the .so.
// Not ported: the sounds (SoundMgr), the popups (PopupMgr: "no internet", the IGP reward), the tracking of the screens and
// of the loading times, the online ads (ShopADItemBox, the ad bar's download), the free cash button (an Android network
// of ads, its natives are not in this build) and CrmMgr's point cuts.
class Menu_Shop : public Menu_BaseCostumes {
public:
    enum EShopTabs { Shop_Bananas = 0, Shop_Tokens = 1, Shop_Costumes = 2, Shop_Bonus = 3, Shop_None = 4 };
    Menu_Shop();
    virtual ~Menu_Shop();
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void SetState(int);
    virtual void Refresh();
    virtual bool UpdateGameInBG() { return false; }
    virtual bool RenderGameInBG() { return true; }
    virtual void OnEnter();
    virtual void OnQuit();
    virtual int GetCurrentTrackingScreen() const { return GetTrackingState(m_currentTab); }
    virtual int GetPreviousTrackingScreen() const { return GetTrackingState(m_previousTab); }
    using Menu_BaseCostumes::Init;
    // @0x5b2b24: the tab opens (right away, or in the next update); a tab that is already the one shows again only when forced
    void GoToTab(EShopTabs tab, bool force, bool refreshOnNextFrame);
    EShopTabs GetCurrentTab() const { return m_currentTab; }
    static const jet::String k_menuName;

private:
    void InitTutorial();                       // @0x5b1724
    void CheckFreeCash();                      // @0x5b1b50
    void UpdateButtons();                      // @0x5b23a0
    void RefreshRedeemCostume();               // @0x5b3160
    void RefreshPromoStatus();                 // @0x5b3468
    void SetupBuyablesTab(const jet::String &category);  // @0x5b48fc
    void SetupUpgradesTab();                   // @0x5b4dc8
    void ClearBuyablesAndUpgradesBoxes();      // @0x5b4f60
    void RefreshCostsRealMoneyTextsVisibility();  // @0x5b5050
    void EnableTouchesTutorial(bool);          // @0x5b50d4
    void SetupTutorial();                      // @0x5b51ac
    void EnableChildrenWithButtons(bool);      // @0x5b5364
    void TutorialFinished();                   // @0x5b5450
    void HandleTutorial();                     // @0x5b54c4
    void UpdateTutorial();                     // @0x5b5564
    int GetIndexShield();                      // @0x5b55b0
    void SetShieldFirst();                     // @0x5b55f0
    static int GetTrackingState(EShopTabs);    // @0x5b5340

    // an object of the tutorial that shows and hides (the arrow and the hand)
    struct TutorialElement {
        explicit TutorialElement(InterfaceObject*);  // @0x5b53f0
        void LaunchTutorial();                 // @0x5b5418
        void Hide();                           // @0x5b5434
        InterfaceObject *m_object;
    };

    bool m_canPressFreeButton;                 // 0x1ed
    bool m_freeCashShown;                      // 0x1ee
    bool m_freeCashFlag;                       // 0x1ef
    InterfaceObject *m_bgFillRect;             // 0x1f0
    InterfaceButton *m_backButton;             // 0x1f4
    InterfaceButton *m_freeBananasButton;      // 0x1f8
    InterfaceButton *m_freeBananasLocked;      // 0x1fc
    InterfaceButton *m_restoreButton;          // 0x200
    std::vector<InterfaceButton*> m_tabButtons;       // 0x204: bananas, tokens, costumes, bonus
    std::vector<InterfaceButton*> m_tabButtonsPromo;  // 0x210
    InterfaceButton *m_newCostumes;            // 0x21c
    InterfaceButton *m_promoCostumes;          // 0x220
    InterfaceButton *m_limitedCostumes;        // 0x224
    BananasAndTokensBox *m_counters;           // 0x228
    InterfaceText *m_textCategory;             // 0x22c
    InterfaceText *m_textTokenWarning;         // 0x230
    InterfaceText *m_textTokenWarningFreeCash; // 0x234
    InterfaceText *m_textLoading;              // 0x238
    EShopTabs m_currentTab;                    // 0x23c
    EShopTabs m_previousTab;                   // 0x240
    EShopTabs m_lastRefreshedTab;              // 0x244
    EShopTabs m_previousTabTracking;           // 0x248
    std::vector<jet::String> m_tabHeaders;     // 0x24c
    InterfaceGraph *m_fadeL, *m_fadeR;         // 0x258, 0x25c
    int m_freeCashTimer;                       // 0x260
    int m_unused264;                           // 0x264
    InterfaceList *m_list;                     // 0x268
    InterfaceObject *m_scrollBarObject;        // 0x26c
    InterfaceObject *m_waitingBg;              // 0x270
    std::vector<ShopBuyableItemBox*> m_buyableBoxes;      // 0x274
    std::vector<ShopBonusUpgradeItemBox*> m_upgradeBoxes;  // 0x280
    bool m_firstBuy;                           // 0x290
    int m_tutorialState;                       // 0x2a0: 0 none, 1 start, 2 the buy button, 3 the back button, 4 and 5 finished
    int m_shieldIndex;                         // 0x2a4: the box of the shield upgrade
    std::vector<TutorialElement> m_tutorialElements;  // 0x2a8
    int m_tutorialElementIndex;                // 0x2b4
    bool m_tutorialStarted;                    // 0x2b8
    bool m_freeCashEnabled;                    // 0x2b9
    bool m_freeCashNotLoaded;                  // 0x2ba
    int m_freeCashChecks;                      // 0x2bc
};

#endif
