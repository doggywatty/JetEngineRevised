// source/game/entities/BonusSet.cpp
#include "BonusSet.h"
#include "Bonus.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "project.h"

const jet::String BonusSet::k_tmplName("BonusSet");  // @0xc43662
BonusSet *BonusSet::s_instance = 0;

// @0x5a4018
BonusSet::BonusSet(const clara::Template *t) : clara::DataEntity(t) { s_instance = this; }

// @0x5a4074
BonusSet::~BonusSet() {
    if (s_instance == this) s_instance = 0;
}

// @0x5a4108: the labels, and the bonuses of the list but the blind box one
void BonusSet::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("LabelMaxUpgrade"), m_labelMaxUpgrade);
    GetParam(jet::String("LabelMaxUpgradeGolden"), m_labelMaxUpgradeGolden);
    const clara::Param *list = FindParamByName(jet::String("Bonuses"));
    for (unsigned i = 0; list && i < list->GetComponentCount(); i++) {
        const clara::Path &path = list->GetAsPath(i);
        clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByPath(path);
        if (path.GetAsString().Equals("/LevelDef/BonusSet/BonusesDef/Bonus_BlindBox")) continue;
        if (Bonus *b = dynamic_cast<Bonus*>(e)) m_bonuses.push_back(b);
    }
}

// @0x5a4334
Bonus *BonusSet::GetBonus(GameItemType type, bool golden) const {
    for (Bonus *b : m_bonuses)
        if (b->IsGolden() == golden && b->GetGameItemType() == type) return b;
    return 0;
}

// @0x5a42cc
const jet::String &BonusSet::GetLabelMaxUpgrade() { return babel::Babel::GetInstance()->GetStringMgr()->Get(m_labelMaxUpgrade); }

// @0x5a4300
const jet::String &BonusSet::GetLabelMaxUpgradeGolden() { return babel::Babel::GetInstance()->GetStringMgr()->Get(m_labelMaxUpgradeGolden); }
