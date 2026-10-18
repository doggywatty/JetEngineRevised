// source/game/entities/BonusSet.h
#ifndef SOURCE_GAME_ENTITIES_BONUSSET_H
#define SOURCE_GAME_ENTITIES_BONUSSET_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"
#include "gameitem/GameItemType.h"

class Bonus;

// The bonuses of the data ("BonusSet" data entity, 0x34 bytes in the .so): the list of the ones the shop upgrades (but the
// blind box one) and the labels of an upgrade at its last level. The set is the instance the shop and the upgrade
// manager reach it by.
class BonusSet : public clara::DataEntity {
public:
    explicit BonusSet(const clara::Template*);
    virtual ~BonusSet();
    virtual void Init() {}  // @0x5a4104
    virtual void PostInit();
    static BonusSet *GetInstance() { return s_instance; }
    static bool HasInstance() { return s_instance != 0; }
    unsigned GetBonusCount() const { return (unsigned)m_bonuses.size(); }  // @0x5a42b8
    // @0x5a4334: the bonus of the item type that is a golden one or not
    Bonus *GetBonus(GameItemType type, bool golden) const;
    std::vector<Bonus*> &GetBonusesArray() { return m_bonuses; }
    const jet::String &GetLabelMaxUpgrade();        // @0x5a42cc
    const jet::String &GetLabelMaxUpgradeGolden();  // @0x5a4300
    static const jet::String k_tmplName;
private:
    std::vector<Bonus*> m_bonuses;                   // 0x20
    jet::String m_labelMaxUpgrade;                   // 0x2c
    jet::String m_labelMaxUpgradeGolden;             // 0x30
    static BonusSet *s_instance;
};

#endif
