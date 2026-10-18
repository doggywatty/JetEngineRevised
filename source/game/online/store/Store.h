// source/game/online/store/Store.h
#ifndef SOURCE_GAME_ONLINE_STORE_STORE_H
#define SOURCE_GAME_ONLINE_STORE_STORE_H

#include <map>
#include "jsingleton.h"
#include "core/pstring.h"

class Price;

// The shop's catalogue: every Price entity registers here by its name, in-app items apart from the dynamic ones
// (those bought with bananas and tokens). 0x78 bytes in the .so. Not ported: the in-app store itself, the dynamic
// pricing and offline items downloads and the promotion queries, none of which exist without the online services.
class Store : public Singleton<Store> {
public:
    Store();
    virtual ~Store();
    void AddDynamicPriceToDB(Price*);   // @0x409e4c
    void AddIAPPriceToDB(Price*);       // @0x409f04
    void ClearPricesDB();               // @0x40a050
    Price *GetIAPItem(const jet::String &name) const;      // @0x40ae90
    Price *GetDynamicItem(const jet::String &name) const;  // @0x40aff4
    bool BuyItem(const jet::String &name) const;           // @0x40a140: whether the dynamic item is there
    // this build's store has no in-app purchases going on: always an item to buy, never one being bought
    bool IsAnyIAPItemAvailable() const { return true; }    // @0x40b5ec
    bool IsIAPPurchaseInProgress() const { return false; } // @0x40b5dc
    const jet::String &GetIAPItemBeingBought() const { return m_iapItemBeingBought; }  // @0x40b5e4
    // the promotions: none here (the dynamic pricing and the in-app promotions come from the online services)
    bool HasPromotionOnCategory(const jet::String &category) const;         // @0x40a9e0
    bool HasIAPPromotion() const { return false; }                          // @0x40ad44
    bool HasIAPPromotionOnCategory(const jet::String &category) const;      // @0x40ab78
    bool HasAnyPromoInShop() const;                                         // @0x40ada0: an in-app or a dynamic promotion on a category of the shop
    bool HasAnyPricePromotionOnCategory(const jet::String &category) const; // @0x40ac40
    jet::String GetIAPPromotionDescription() const { return jet::String(); }  // @0x40ad4c
    int GetIAPPromotionRemainingTime() const { return 0; }                  // @0x40ad58
    int GetDynamicPricingPromotionRemainingTime() const { return 0; }       // @0x40a9b8: 0 while no promotion is on
    bool IsRestoringGoldenBanana() const { return false; }                  // @0x40a160
    // @0x40a168: without the store ready and the online services the .so shows the popup POPUP_NO_INTERNET (PopupMgr is not
    // ported) and restores nothing
    bool RestorePendingTransactions() { return false; }
    bool IsIAPStoreUpdating() const { return false; }                       // @0x40bda4
    bool IsAnyIAPTokenItemAvailable() const { return true; }                // @0x40b5f4
    // @0x40b340 / @0x40b430: whether the price of an upgrade or a costume is one that was bought (or is not the one on sale)
    bool IsItemBoughtOrNotVisible(const Price&) const;
    bool IsItemBought(const Price&) const;
private:
    bool m_dynamicPromotionOn = false;  // 0x7c: set by the online dynamic pricing, never here
    std::map<jet::String, Price*> m_dynamicPrices;  // 0x28
    std::map<jet::String, Price*> m_iapPrices;      // 0x40
    jet::String m_iapItemBeingBought;               // 0x84
};

#endif
