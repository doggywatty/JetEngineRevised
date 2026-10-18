// source/game/gamemanagers/BuyablesMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_BUYABLESMGR_H
#define SOURCE_GAME_GAMEMANAGERS_BUYABLESMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"

class Buyable;

// The buyables of the data (the packs of bananas and tokens), added as they are initialized (0x10 bytes in the .so). They
// belong to the clara project, the manager only lists them. Not ported: CreateDynamicBuyable @0x544860, which makes a
// buyable of a dynamic price of the online store.
class BuyablesMgr : public Singleton<BuyablesMgr> {
public:
    BuyablesMgr() {}  // @0x5445f4
    virtual ~BuyablesMgr();
    void Reset() {}   // @0x5446f0
    void Update() {}  // @0x5446f4
    void AddBuyable(Buyable *b) { m_buyables.push_back(b); }  // @0x5446f8
    // @0x544714: the buyables of the price category ("Bananas", "Tokens"), the cheaper ones first
    std::vector<Buyable*> GetBuyablesSortedByPrice(const jet::String &category);
private:
    void ClearAll() { m_buyables.clear(); }  // @0x5446d8
    std::vector<Buyable*> m_buyables;  // 0x4
};

#endif
