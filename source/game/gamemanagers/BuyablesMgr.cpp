// source/game/gamemanagers/BuyablesMgr.cpp
#include "BuyablesMgr.h"
#include "game/entities/Buyable.h"
#include "game/online/store/Price.h"
#include <algorithm>

// @0x544638
BuyablesMgr::~BuyablesMgr() { ClearAll(); }

// @0x544714: the buyables that have a price of the category, sorted with BuyablePricesComparison @0x544824
std::vector<Buyable*> BuyablesMgr::GetBuyablesSortedByPrice(const jet::String &category) {
    std::vector<Buyable*> r;
    for (Buyable *b : m_buyables)
        if (b->GetPrice() && b->GetPrice()->GetCategory().Equals(category)) r.push_back(b);
    std::sort(r.begin(), r.end(), [](Buyable *a, Buyable *b) { return a->GetPrice()->GetPriceAsDouble() < b->GetPrice()->GetPriceAsDouble(); });
    return r;
}
