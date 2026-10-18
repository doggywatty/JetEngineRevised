// source/game/gamemanagers/BonusUpgradeMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_BONUSUPGRADEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_BONUSUPGRADEMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "game/utils/GameSaveable.h"

class Bonus;
class BonusSet;
class Price;

// The level of each bonus the player has bought (by the name of its entity): the one saved is the one the bonuses take up
struct BonusUpgradeMgrSaveData {
    struct BonusInfo {
        jet::String name;  // 0x0
        int level;         // 0x4
        BonusInfo() : level(0) {}
    };
    std::vector<BonusInfo> bonuses;  // 0x0
};

// What the player upgraded, saved as the record "BonusUpgradeMgr" (0x34 bytes in the .so).
class BonusUpgradeMgr : public Singleton<BonusUpgradeMgr>, public GameSaveable<BonusUpgradeMgrSaveData> {
public:
    BonusUpgradeMgr();
    virtual ~BonusUpgradeMgr() {}
    void Reset();  // @0x544028: the bonuses of the data take the levels saved, or their first one
    virtual void ResetData(BonusUpgradeMgrSaveData&) const;
    virtual void SaveDataInStream(jet::stream::IStreamW&, BonusUpgradeMgrSaveData&) const;
    virtual bool LoadDataFromStream(jet::stream::IStream&, BonusUpgradeMgrSaveData&) const;
    unsigned GetBonusCount() const;  // @0x544338
    const BonusUpgradeMgrSaveData::BonusInfo &GetBonusInfo(unsigned i) { return m_data.bonuses[i]; }  // @0x544340
    void OnBonusLevelChanged(Bonus*);  // @0x54434c
    // @0x5444f0: a real-money price was delivered: the bonus it is a price of buys the level of it (true when it is one of the bonuses'
    // prices); alreadyBought is set when the price is one of a level already bought
    bool HandleDeliverIAPItem(const Price*, bool &alreadyBought);
    static const jet::String k_saveRecordName;
private:
    void SerializeV1(jet::stream::IStreamW&, BonusUpgradeMgrSaveData&) const;
    bool DeserializeV1(jet::stream::IStream&, BonusUpgradeMgrSaveData&) const;
    static const int k_saveGameVersion = 1;
    static const int k_signatureV1 = 0xabcd02;
    BonusSet *m_bonusSet;  // 0x20
};

#endif
