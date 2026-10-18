// source/game/entities/LocationInfoEntity.cpp
#include "LocationInfoEntity.h"
#include "param.h"
#include "game/Statistics.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/online/store/Price.h"
#include "localization/formatter.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "localization/formatter.h"
#include "game/Statistics.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/online/store/Price.h"

const jet::String LocationInfoEntity::k_tmplName("LocationInfoEntity"), LocationInfoEntity::k_param_LocationId("LocationId"),
    LocationInfoEntity::k_param_FallbackLocationId("FallbackLocationId"), LocationInfoEntity::k_param_UnlockByPrice("UnlockByPrice"),
    LocationInfoEntity::k_param_FastTravelPrice("FastTravelPrice"), LocationInfoEntity::k_param_FreeUnlockConditionType("FreeUnlockConditionType"),
    LocationInfoEntity::k_param_FastTravelUnlockingMission("FastTravelUnlockingMission"),
    LocationInfoEntity::k_param_ScoreMultiplierFromPriceAmount("ScoreMultiplierFromPriceAmount"), LocationInfoEntity::k_param_IsHidden("IsHidden"),
    LocationInfoEntity::k_param_LocationTitle("LocationTitle"), LocationInfoEntity::k_param_FastTravel_DLC_Info("FastTravel_DLC_Info"),
    LocationInfoEntity::k_param_FastTravel_CanUseLocation_Info("FastTravel_CanUseLocation_Info"),
    LocationInfoEntity::k_param_HasTutorialImplementation("HasTutorialImplementation"),
    LocationInfoEntity::k_param_Location_JustUnlocked_Info("Location_JustUnlocked_Info"),
    LocationInfoEntity::k_param_Location_JustUnlocked_Info2("Location_JustUnlocked_Info2"),
    LocationInfoEntity::k_param_FastTravel_LockByProgress_Info("FastTravel_LockByProgress_Info"),
    LocationInfoEntity::k_param_FastTravel_LockByProgress_Info2("FastTravel_LockByProgress_Info2"), LocationInfoEntity::k_unlockType_None("None"),
    LocationInfoEntity::k_unlockType_Mission("Mission"), LocationInfoEntity::k_unlockType_ScoreMultiplier("ScoreMultiplier");

// @0x4e6328
void LocationInfoEntity::Init() {
    clara::Entity::Init();
}

static clara::DataEntity *PathEntity(const clara::DataEntity *e, const jet::String &param) {
    const clara::Param *p = e->FindParamByName(param);
    return p ? clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)) : clara::Project::GetInstance()->FindEntityByPath(clara::Path());
}

// @0x4e632c
void LocationInfoEntity::PostInit() {
    GameEntity::PostInit();
    m_locationId = FindParamByName(k_param_LocationId)->GetAsIdEnum(0).value;
    m_fallbackLocationId = FindParamByName(k_param_FallbackLocationId)->GetAsIdEnum(0).value;
    m_unlockByPrice = PathEntity(this, k_param_UnlockByPrice);
    m_fastTravelPrice = PathEntity(this, k_param_FastTravelPrice);
    GetParam(k_param_FreeUnlockConditionType, m_freeUnlockConditionType);
    GetParam(k_param_FastTravelUnlockingMission, m_fastTravelUnlockingMission);
    m_scoreMultiplierPrice = 0;
    if (m_freeUnlockConditionType.Equals(k_unlockType_ScoreMultiplier)) m_scoreMultiplierPrice = PathEntity(this, k_param_ScoreMultiplierFromPriceAmount);
    GetParam(k_param_IsHidden, m_isHidden);
    InitAssociatedLocationsId();
    RegisterForUpdate(false);
    RegisterForRender(false);
}

// @0x4e64e4
void LocationInfoEntity::InitAssociatedLocationsId() {
    m_associated.push_back(m_locationId);
    if (m_fallbackLocationId != -1) m_associated.push_back(m_fallbackLocationId);
}

jet::String LocationInfoEntity::TextParam(const jet::String &name) const {
    jet::String id;
    GetParam(name, id);
    return babel::Babel::GetInstance()->GetStringMgr()->Get(id);
}

// @0x4e6550
jet::String LocationInfoEntity::GetTitle() const { return TextParam(k_param_LocationTitle); }
// @0x4e6ac0
jet::String LocationInfoEntity::GetFastTravel_DLC_Info() const { return TextParam(k_param_FastTravel_DLC_Info); }
// @0x4e6b30
jet::String LocationInfoEntity::GetFastTravel_CanUseLocation_Info() const { return TextParam(k_param_FastTravel_CanUseLocation_Info); }

// @0x4e6990: the text of the id, nothing when there is no id
static jet::String InfoText(const clara::DataEntity *e, const jet::String &name) {
    jet::String id;
    e->GetParam(name, id);
    if (id.empty()) return id;
    return babel::Babel::GetInstance()->GetStringMgr()->Get(id);
}
jet::String LocationInfoEntity::GetLocationJustUnlocked_Info() const { return InfoText(this, k_param_Location_JustUnlocked_Info); }
jet::String LocationInfoEntity::GetLocationJustUnlocked_Info2() const { return InfoText(this, k_param_Location_JustUnlocked_Info2); }

// @0x4e6ba8
bool LocationInfoEntity::HasTutorialImplementation() const {
    bool b = false;
    GetParam(k_param_HasTutorialImplementation, b);
    return b;
}

// @0x572f38: the mission it asks for was done ("none" asks for nothing), or the multiplier the missions have reached is
// the one of its price; with no condition it is open
bool LocationInfoEntity::IsLocationUnlocked() const {
    if (m_freeUnlockConditionType.Equals(k_unlockType_Mission)) {
        if (m_fastTravelUnlockingMission.Equals("none")) return true;
        return MissionMgr::GetInstance()->WasMissionIdCompleted(m_fastTravelUnlockingMission);
    }
    if (m_freeUnlockConditionType.Equals(k_unlockType_ScoreMultiplier)) {
        if (!m_scoreMultiplierPrice) return false;
        return Statistics::GetInstance()->GetBaseScoreMultiplier() >= static_cast<Price*>(m_scoreMultiplierPrice)->GetAmount();
    }
    return true;
}

// @0x573078: the text of the data's id with the multiplier the location asks for
jet::String LocationInfoEntity::GetFastTravel_LockByProgress_Info() const {
    jet::String id;
    GetParam(k_param_FastTravel_LockByProgress_Info, id);
    jet::String measure;
    if (m_scoreMultiplierPrice && m_freeUnlockConditionType.Equals(k_unlockType_ScoreMultiplier))
        measure = babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(static_cast<Price*>(m_scoreMultiplierPrice)->GetAmount());
    return jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(id).c_str(), measure.c_str());
}

// @0x5731b0: the same with the multiplier the player has; no text when the data has no id
jet::String LocationInfoEntity::GetFastTravel_LockByProgress_Info2() const {
    jet::String id;
    GetParam(k_param_FastTravel_LockByProgress_Info2, id);
    if (id.empty()) return id;
    jet::String measure;
    if (m_freeUnlockConditionType.Equals(k_unlockType_ScoreMultiplier))
        measure = babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(Statistics::GetInstance()->GetBaseScoreMultiplier());
    return jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(id).c_str(), measure.c_str());
}

// @0x57354c
int LocationInfoEntity::GetScoreMultiplierValue() const {
    if (!m_freeUnlockConditionType.Equals(k_unlockType_ScoreMultiplier)) return 0;
    return static_cast<Price*>(m_scoreMultiplierPrice)->GetAmount();
}

// @0x57305c
Price *LocationInfoEntity::GetUnlockByPricePricePtr() const { return static_cast<Price*>(m_unlockByPrice); }
