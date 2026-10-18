// source/game/entities/Bonus.h
#ifndef SOURCE_GAME_ENTITIES_BONUS_H
#define SOURCE_GAME_ENTITIES_BONUS_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"
#include "gameitem/GameItemType.h"

class Price;

// What the shop upgrades ("Bonus" data entity, 0xa4 bytes in the .so): a vehicle or power-up (game item type) with a price
// for each of its levels, the percent each level shows and its probability weights. The level is the one bought, -1 when
// the bonus is locked (nothing bought yet), and the price of the next level is what the shop asks. A golden bonus is the
// kind bought with the golden bananas: it has no last level.
// Not ported: the blind box bonus (BlindBox entities), the open graph activity of an upgrade, the tracking of the purchase.
class Bonus : public clara::DataEntity {
public:
    explicit Bonus(const clara::Template*);
    virtual ~Bonus();
    virtual void Init() {}  // @0x5a2e78
    virtual void PostInit();
    virtual void Update() {}  // @0x5a34c8
    const jet::String &GetBonusName();                       // @0x5a34cc: the text of its name
    const jet::String &GetBonusDescriptionForNextLevel();    // @0x5a3614
    const jet::String &GetBonusLabelMaxUpgrade();            // @0x5a3650
    GameItemType GetGameItemType() const { return m_gameItemType; }
    const clara::DataEntity *GetDecoRef() const { return m_decoRef; }
    const clara::DataEntity *GetEffectRef() const { return m_effectRef; }
    const clara::DataEntity *GetShopDecoRef() const { return m_shopDecoRef; }  // @0x5a3680
    bool IsGolden() const { return m_isGolden; }
    int GetCurrentLevel() const { return m_level; }
    int GetShopSpriteFrame() const { return m_shopSpriteFrame; }
    void ResetLevelToDefault();                              // @0x5a3484: -1 when it is locked, else 0
    Price *GetPrice(int level) const;                        // @0x5a3688: below -1 is the current level; none out of the levels
    Price *GetPriceForNextLevel() const { return GetPrice(m_level + 1); }  // @0x49b2dc
    int GetLevelsCount() const { return (int)m_prices.size(); }
    int GetPercentUpgradeDisplay(int level) const;           // @0x5a36b8
    int GetAmount(int level) const;                          // @0x5a3778
    float GetProbabilityWeight(int level) const;             // @0x5a36e8: the weight of the level, with the costume's factor for the type
    void OnCurrentLevelPurchased(unsigned trackingA, unsigned trackingB);  // @0x5a3798: the next level is bought
    bool HasPriceInPriceArray(const Price*) const;           // @0x5a3a9c
    bool IsPriceCurrentlyVisible(const Price *p) const { return p == GetPriceForNextLevel(); }  // @0x5a3ad8
    bool IsPriceBought(const Price*) const;                  // @0x5a3af4
    bool IsPriceFromThisBonus(const Price*) const;           // @0x5a3b9c
    bool ShouldShowGoldBoughtTexts();                        // @0x5a3b30
    static jet::String k_tmplName;
    friend class BonusUpgradeMgr;
private:
    const jet::String &GetBonusDescriptionLevel0();          // @0x5a3500
    const jet::String &GetBonusDescriptionAboveLevel0();     // @0x5a355c
    const jet::String &GetBonusDescriptionLevelMax();        // @0x5a35b8
    jet::String m_name;                                      // 0x20
    jet::String m_descriptionLevel0;                         // 0x24
    jet::String m_descriptionAboveLevel0;                    // 0x28
    jet::String m_description2AboveLevel0;                   // 0x2c
    jet::String m_descriptionMax;                            // 0x30
    jet::String m_descriptionLevel0Gold, m_descriptionAboveLevel0Gold, m_description2AboveLevel0Gold, m_descriptionMaxGold;  // 0x34..0x40
    GameItemType m_gameItemType;                             // 0x44
    clara::DataEntity *m_decoRef;                            // 0x48
    clara::DataEntity *m_effectRef;                          // 0x4c
    clara::DataEntity *m_shopDecoRef;                        // 0x50
    bool m_isGolden;                                         // 0x54
    std::vector<Price*> m_prices;                            // 0x58
    std::vector<int> m_percentUpgrade;                       // 0x68
    std::vector<float> m_probabilityWeights;                 // 0x78
    int m_level;                                             // 0x98
    int m_shopSpriteFrame;                                   // 0xa0
};

#endif
