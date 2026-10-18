// source/game/entities/Bonus.cpp
#include "Bonus.h"
#include "game/Game.h"
#include "BonusSet.h"
#include "MinionCostume.h"
#include "game/Statistics.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/BonusUpgradeMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "project.h"
#include <cstdlib>

jet::String Bonus::k_tmplName("Bonus");  // @0xc4355d

static const jet::String &Text(const jet::String &key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(key); }

// @0x5a2c24
Bonus::Bonus(const clara::Template *t)
    : clara::DataEntity(t), m_gameItemType(0), m_decoRef(0), m_effectRef(0), m_shopDecoRef(0), m_isGolden(false), m_level(-1), m_shopSpriteFrame(-1) {}

// @0x5a2d3c
Bonus::~Bonus() {}

// @0x5a2e7c: the texts, the game item type, the deco of the shop, the prices (of the bonus: each takes the name of the
// bonus as its description), the percents and the probabilities. A locked bonus starts at level -1 (the .so also reads
// the "OpenGraphObj" list and, for the blind box type 13, the "RefBlindBox", which are not ported)
void Bonus::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("Name"), m_name);
    GetParam(jet::String("DescriptionLevel0"), m_descriptionLevel0);
    GetParam(jet::String("Description"), m_descriptionAboveLevel0);
    GetParam(jet::String("Description_2"), m_description2AboveLevel0);
    GetParam(jet::String("DescriptionMax"), m_descriptionMax);
    GetParam(jet::String("DescriptionLevel0_GoldBought"), m_descriptionLevel0Gold);
    GetParam(jet::String("Description_GoldBought"), m_descriptionAboveLevel0Gold);
    GetParam(jet::String("Description_2_GoldBought"), m_description2AboveLevel0Gold);
    GetParam(jet::String("DescriptionMax_GoldBought"), m_descriptionMaxGold);
    GetParam(jet::String("GameItemType"), m_gameItemType);
    clara::Project *project = clara::Project::GetInstance();
    if (const clara::Param *p = FindParamByName(jet::String("Deco"))) m_decoRef = project->FindEntityByPath(p->GetAsPath(0));
    if (const clara::Param *p = FindParamByName(jet::String("Effect"))) m_effectRef = project->FindEntityByPath(p->GetAsPath(0));
    if (const clara::Param *p = FindParamByName(jet::String("ShopDeco"))) m_shopDecoRef = project->FindEntityByPath(p->GetAsPath(0));
    GetParam(jet::String("isGolden"), m_isGolden);
    bool locked = false;
    GetParam(jet::String("isLocked"), locked);
    if (!locked && m_level < 0) m_level = 0;
    // the frame of the sprite of the shop
    if (const clara::Param *p = FindParamByName(jet::String("ShopSprite"))) m_shopSpriteFrame = atoi(p->GetAsSprite(0).anim.c_str());
    if (const clara::Param *p = FindParamByName(jet::String("Prices")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            Price *price = dynamic_cast<Price*>(project->FindEntityByPath(p->GetAsPath(i)));
            if (!price) continue;
            price->SetItemDescription(Text(m_name));
            if (!price->GetItemName().Equals("Prices_Blindbox")) m_prices.push_back(price);
        }
    if (const clara::Param *p = FindParamByName(jet::String("PercentUpgrade")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) m_percentUpgrade.push_back(p->GetAsS32(i));
    if (const clara::Param *p = FindParamByName(jet::String("Probabilities")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) m_probabilityWeights.push_back(p->GetAsFloat(i));
}

// @0x5a3484
void Bonus::ResetLevelToDefault() {
    bool locked = false;
    GetParam(jet::String("isLocked"), locked);
    m_level = locked ? -1 : 0;
}

// @0x5a34cc
const jet::String &Bonus::GetBonusName() { return Text(m_name); }

// the texts of a bonus, in the golden bananas' version when the golden one was bought
// @0x5a3500
const jet::String &Bonus::GetBonusDescriptionLevel0() { return Text(ShouldShowGoldBoughtTexts() && !m_descriptionLevel0Gold.empty() ? m_descriptionLevel0Gold : m_descriptionLevel0); }
// @0x5a355c
const jet::String &Bonus::GetBonusDescriptionAboveLevel0() { return Text(ShouldShowGoldBoughtTexts() && !m_descriptionAboveLevel0Gold.empty() ? m_descriptionAboveLevel0Gold : m_descriptionAboveLevel0); }
// @0x5a35b8
const jet::String &Bonus::GetBonusDescriptionLevelMax() { return Text(ShouldShowGoldBoughtTexts() && !m_descriptionMaxGold.empty() ? m_descriptionMaxGold : m_descriptionMax); }

// @0x5a3614: what the next level gives; the first level before anything is bought, the last one once the prices are over
// (a golden bonus never gets there)
const jet::String &Bonus::GetBonusDescriptionForNextLevel() {
    if (m_level < 0) return GetBonusDescriptionLevel0();
    if (!m_isGolden && m_level + 1 >= (int)m_prices.size()) return GetBonusDescriptionLevelMax();
    return GetBonusDescriptionAboveLevel0();
}

// @0x5a3650
const jet::String &Bonus::GetBonusLabelMaxUpgrade() {
    return m_isGolden ? BonusSet::GetInstance()->GetLabelMaxUpgradeGolden() : BonusSet::GetInstance()->GetLabelMaxUpgrade();
}

// @0x5a3688
Price *Bonus::GetPrice(int level) const {
    if (level < -1) level = m_level;
    if (level < 0 || level >= (int)m_prices.size()) return 0;
    return m_prices[(size_t)level];
}

// @0x5a36b8
int Bonus::GetPercentUpgradeDisplay(int level) const {
    if (level < -1) level = m_level;
    if (level < 0 || level >= (int)m_percentUpgrade.size()) return 0;
    return m_percentUpgrade[(size_t)level];
}

// @0x5a3778
int Bonus::GetAmount(int level) const {
    if (level < -1) level = m_level;
    Price *p = GetPrice(level);
    return p ? p->GetAmount() : 0;
}

// @0x5a36e8: the weight of the level (the current one below -1; nothing for a locked bonus or a level it has no weight for), raised by the
// costume's weight factor for the type (each point of it adds 1%)
float Bonus::GetProbabilityWeight(int level) const {
    if (level < -1) level = m_level;
    if (level < 0 || (size_t)level >= m_probabilityWeights.size()) return 0;
    const float weight = m_probabilityWeights[(size_t)level];
    const int factor = CostumeMgr::GetInstance()->GetEquippedCostumeRef()->GetExtraWeightFactorForPowerUp(m_gameItemType);
    return weight + (float)factor * 0.01f * weight;
}

// @0x5a3798: the level goes up, the manager keeps it; the first level counts as the unlock of the type, the later ones
// as upgrades, and the last one as a maximum (all the types at their maximum count as one more statistic)
void Bonus::OnCurrentLevelPurchased(unsigned, unsigned) {
    m_level++;
    BonusUpgradeMgr::GetInstance()->OnBonusLevelChanged(this);
    // the statistics, and then (the tracking of the purchase is not ported) the save
    [this] {
        Statistics *stats = Statistics::GetInstance();
        if (m_isGolden) return;
        if (m_level == 0) {
            const jet::String unlocked = GameItemTypeUnlockedStatisticsScope(m_gameItemType);
            if (!unlocked.empty()) stats->IncreaseLocationStat(-1, unlocked, 1);
        }
        if (m_level <= 0) return;
        stats->IncreaseLocationStat(-1, StatisticsSaveData::k_statScope_upgradesCount, 1);
        if (m_level + 1 < (int)m_prices.size()) return;
        const jet::String maxScope = GameItemTypeToMaxUpgradeStatisticsScope(m_gameItemType);
        if (maxScope.empty()) return;
        stats->IncreaseLocationStat(-1, maxScope, 1);
        for (GameItemType t = 3; t < 14; t++) {
            const jet::String scope = GameItemTypeToMaxUpgradeStatisticsScope(t);
            if (!scope.empty() && stats->GetIntValue(-1, scope, 1) <= 0) return;
        }
        stats->IncreaseLocationStat(-1, StatisticsSaveData::k_statScope_maxUpgradesAll, 1);
    }();
    Game::GetInstance()->Save(false, false);
}

// @0x5a3a9c
bool Bonus::HasPriceInPriceArray(const Price *p) const {
    for (Price *q : m_prices)
        if (q == p) return true;
    return false;
}

// @0x5a3af4: the price is of a level already bought
bool Bonus::IsPriceBought(const Price *p) const {
    Price *next = GetPriceForNextLevel();
    if (!next) return false;
    return next->GetAmount() > p->GetAmount();
}

// @0x5a3b9c
bool Bonus::IsPriceFromThisBonus(const Price *p) const {
    for (Price *q : m_prices)
        if (q->GetItemName().Equals(p->GetItemName())) return true;
    return false;
}

// @0x5a3b30: the golden bonus of the type has no level left to buy, so the texts of the golden version show
bool Bonus::ShouldShowGoldBoughtTexts() {
    if (m_isGolden) return false;
    Bonus *golden = BonusSet::GetInstance()->GetBonus(m_gameItemType, true);
    if (!golden || golden->m_level < 0) return false;
    return golden->GetPriceForNextLevel() == 0;
}
