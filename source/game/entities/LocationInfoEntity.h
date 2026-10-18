// source/game/entities/LocationInfoEntity.h
#ifndef SOURCE_GAME_ENTITIES_LOCATIONINFOENTITY_H
#define SOURCE_GAME_ENTITIES_LOCATIONINFOENTITY_H

#include <vector>
#include "GameEntity.h"

class Price;

// What the data says of a location: its title, how it unlocks and what it costs, and the location it falls back to
class LocationInfoEntity : public GameEntity {  // sizeof 0xB8
public:
    static const jet::String k_tmplName, k_param_LocationId, k_param_FallbackLocationId, k_param_UnlockByPrice, k_param_FastTravelPrice,
        k_param_FreeUnlockConditionType, k_param_FastTravelUnlockingMission, k_param_ScoreMultiplierFromPriceAmount, k_param_IsHidden,
        k_param_LocationTitle, k_param_FastTravel_DLC_Info, k_param_FastTravel_CanUseLocation_Info, k_param_HasTutorialImplementation,
        k_param_Location_JustUnlocked_Info, k_param_Location_JustUnlocked_Info2, k_param_FastTravel_LockByProgress_Info,
        k_param_FastTravel_LockByProgress_Info2, k_unlockType_None, k_unlockType_Mission, k_unlockType_ScoreMultiplier;
    explicit LocationInfoEntity(const clara::Template *t) : GameEntity(t), m_locationId(0), m_fallbackLocationId(-1), m_unlockByPrice(0), m_fastTravelPrice(0),
        m_scoreMultiplierPrice(0), m_isHidden(false) {}
    virtual void Init();
    virtual void PostInit();
    int GetLocationId() const { return m_locationId; }
    // the location shown instead while this one is hidden, else the location itself
    int GetFallbackLocationId() const { return m_fallbackLocationId != -1 && m_isHidden ? m_fallbackLocationId : m_locationId; }
    bool IsHidden() const { return m_isHidden; }
    clara::DataEntity *GetScoreMultiplierPrice() const { return m_scoreMultiplierPrice; }
    // this location and the one it falls back to
    const std::vector<int> &GetAssociatedLocations() const { return m_associated; }
    jet::String GetTitle() const;
    bool HasUnlockByPricePrice() const { return m_unlockByPrice != 0; }
    clara::DataEntity *GetUnlockByPricePrice() const { return m_unlockByPrice; }
    Price *GetUnlockByPricePricePtr() const;  // @0x57305c
    clara::DataEntity *GetFastTravelPrice() const { return m_fastTravelPrice; }
    bool IsUnlockByPriceThis(const clara::DataEntity *p) const { return m_unlockByPrice == p; }
    // @0x572f38: whether the data's unlock condition is met (a mission completed, or a score multiplier reached)
    bool IsLocationUnlocked() const;
    // @0x573064
    jet::String GetFastTravel_LockByProgress_Info() const;
    jet::String GetFastTravel_LockByProgress_Info2() const;
    // @0x57354c: the multiplier the location asks for (0 when it does not unlock by multiplier)
    int GetScoreMultiplierValue() const;
    jet::String GetFastTravel_DLC_Info() const;
    jet::String GetFastTravel_CanUseLocation_Info() const;
    bool HasTutorialImplementation() const;
    jet::String GetLocationJustUnlocked_Info() const;
    jet::String GetLocationJustUnlocked_Info2() const;
private:
    void InitAssociatedLocationsId();
    jet::String TextParam(const jet::String &name) const;
    int m_locationId;                  // 0x8C
    int m_fallbackLocationId;          // 0x90
    std::vector<int> m_associated;     // 0x94
    clara::DataEntity *m_fastTravelPrice;  // 0xA0
    bool m_isHidden;                   // 0xA4
    clara::DataEntity *m_unlockByPrice;    // 0xA8
    clara::DataEntity *m_scoreMultiplierPrice;  // 0xAC
    jet::String m_freeUnlockConditionType;      // 0xB0
    jet::String m_fastTravelUnlockingMission;   // 0xB4
};

#endif
