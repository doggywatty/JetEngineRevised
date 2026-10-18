// source/game/online/store/Store.cpp
#include "Store.h"
#include "Price.h"
#include "game/entities/Bonus.h"
#include "game/entities/BonusSet.h"
#include "game/gamemanagers/CostumeMgr.h"
#include <cstdio>

Store::Store() {}
Store::~Store() {}

// @0x409e4c: names are unique
void Store::AddDynamicPriceToDB(Price *p) {
    if (!p) return;
    if (!m_dynamicPrices.emplace(p->GetItemName(), p).second) fprintf(stderr, "Trying to add a duplicated price (%s). Price names must be unique.\n", p->GetItemName().c_str());
}

// @0x409f04
void Store::AddIAPPriceToDB(Price *p) {
    if (!p) return;
    if (!m_iapPrices.emplace(p->GetItemName(), p).second) fprintf(stderr, "Trying to add a duplicated price (%s). Price names must be unique.\n", p->GetItemName().c_str());
}

void Store::ClearPricesDB() {
    m_iapPrices.clear();
    m_dynamicPrices.clear();
}

Price *Store::GetIAPItem(const jet::String &n) const {
    auto it = m_iapPrices.find(n);
    return it == m_iapPrices.end() ? 0 : it->second;
}

// @0x40aff4 compares with the other map's end in the .so; a miss is no item
Price *Store::GetDynamicItem(const jet::String &n) const {
    auto it = m_dynamicPrices.find(n);
    return it == m_dynamicPrices.end() ? 0 : it->second;
}

bool Store::BuyItem(const jet::String &n) const { return m_dynamicPrices.find(n) != m_dynamicPrices.end(); }

// @0x40a9e0: a price of the category (any, when it is empty) that is on sale and not bought: the price or the amount it
// gives is better than the regular one (costumes and upgrades only count by their price, and the costumes by their extra
// categories). Without a dynamic promotion only the costumes can have one
bool Store::HasPromotionOnCategory(const jet::String &category) const {
    const bool costumes = category.Equals("Costumes");
    if (!m_dynamicPromotionOn && !costumes) return false;
    const bool priceOnly = category.Equals("Upgrades") || costumes;
    for (const auto &kv : m_dynamicPrices) {
        Price *p = kv.second;
        if (!p->IsActive()) continue;
        if (!category.empty() && !p->GetCategory().Equals(category)) continue;
        if (IsItemBoughtOrNotVisible(*p)) continue;
        if (p->HasPricePromotion()) return true;
        if (!priceOnly) {
            if (p->HasAmountPromotion()) return true;
        } else if (costumes && p->GetExtraCategoryCount() > 0) {
            return true;
        }
    }
    return false;
}

// @0x40ab78
bool Store::HasIAPPromotionOnCategory(const jet::String &category) const {
    if (!HasIAPPromotion()) return false;
    for (const auto &kv : m_iapPrices) {
        Price *p = kv.second;
        if (!p->IsActive()) continue;
        if (!category.empty() && !p->GetCategory().Equals(category)) continue;
        if (p->HasAmountPromotion() || p->HasPricePromotion()) return true;
    }
    return false;
}

// @0x40ada0 (HasIAPPromotionInShop @0x40ab1c, HasDynamicPromotionInShop @0x40b048): the categories of the shop are bananas, tokens,
// costumes and upgrades (Store::InitShopPriceCategoriesName @0x40adc8); without a dynamic promotion on only the costumes count
bool Store::HasAnyPromoInShop() const {
    static const char *k_categories[] = { "Bananas", "Tokens", "Costumes", "Upgrades" };
    if (HasIAPPromotion())
        for (const char *c : k_categories)
            if (HasIAPPromotionOnCategory(jet::String(c))) return true;
    if (m_dynamicPromotionOn) {
        for (const char *c : k_categories)
            if (HasPromotionOnCategory(jet::String(c))) return true;
        return false;
    }
    return HasPromotionOnCategory(jet::String("Costumes"));
}

// @0x40ac40
bool Store::HasAnyPricePromotionOnCategory(const jet::String &category) const {
    for (const auto &kv : m_iapPrices) {
        Price *p = kv.second;
        if (!p->IsActive()) continue;
        if (!category.empty() && !p->GetCategory().Equals(category)) continue;
        if (p->HasAmountPromotion() || p->HasPricePromotion()) return true;
    }
    return false;
}

// @0x40b340: an upgrade price is visible when it is the next of its bonus; a costume one when it was not bought
bool Store::IsItemBoughtOrNotVisible(const Price &p) const {
    if (p.GetCategory().Equals("Upgrades")) {
        for (Bonus *b : BonusSet::GetInstance()->GetBonusesArray())
            if (b->IsPriceCurrentlyVisible(&p)) return false;
        return true;
    }
    if (!p.GetCategory().Equals("Costumes")) return false;
    return CostumeMgr::GetInstance()->IsPriceBought(&p);
}

// @0x40b430
bool Store::IsItemBought(const Price &p) const {
    if (p.GetCategory().Equals("power up") || p.GetCategory().Equals("Upgrades")) {
        for (Bonus *b : BonusSet::GetInstance()->GetBonusesArray())
            if (b->IsPriceFromThisBonus(&p) && b->IsPriceBought(&p)) return true;
        return false;
    }
    if (!p.GetCategory().Equals("Costumes")) return false;
    return CostumeMgr::GetInstance()->IsPriceBought(&p);
}
