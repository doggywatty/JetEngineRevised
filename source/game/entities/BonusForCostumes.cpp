// source/game/entities/BonusForCostumes.cpp
#include "BonusForCostumes.h"
#include "game/gamemanagers/BackgroundMgr.h"
#include "game/gamemanagers/LocationsMgr.h"

jet::String BonusForCostumes::k_tmplName("BonusForCostumes");

// @0x5a3cc8
BonusForCostumes::BonusForCostumes(const clara::Template *t) : clara::DataEntity(t), m_amount(0), m_skillType(0), m_gameItemType(0), m_isLocationDependent(false) {}

// @0x5a3d1c
BonusForCostumes::~BonusForCostumes() {}

// @0x5a3db4
void BonusForCostumes::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("Amount"), m_amount);
    GetParam(jet::String("GameItemType"), m_gameItemType);
    GetParam(jet::String("SkillType"), m_skillType);
    GetParam(jet::String("IsLocationDependent"), m_isLocationDependent);
    if (!m_isLocationDependent) return;
    const clara::Param *p = FindParamByName(jet::String("LocationDef"));
    for (unsigned i = 0; p && i < p->GetComponentCount(); i++) m_locations.push_back((int)p->GetAsEnum(i));
}

// @0x5a3f70: no skill away from its locations (the current one or one associated with them)
int BonusForCostumes::GetSkillType() const {
    if (!m_isLocationDependent) return m_skillType;
    const int current = BackgroundMgr::GetInstance()->GetLocation();
    for (int loc : m_locations)
        for (int a : LocationsMgr::GetInstance()->GetAssociatedLocationsFor(loc))
            if (a == current) return m_skillType;
    return 0;
}
