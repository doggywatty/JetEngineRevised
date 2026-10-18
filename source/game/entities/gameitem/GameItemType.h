// source/game/entities/gameitem/GameItemType.h
#ifndef SOURCE_GAME_ENTITIES_GAMEITEM_GAMEITEMTYPE_H
#define SOURCE_GAME_ENTITIES_GAMEITEM_GAMEITEMTYPE_H

#include "core/pstring.h"
#include "game/Statistics.h"

// The kind of a pickup (GameItem, not ported yet). 0 any, 1 the power-ups 3..5, 2 the power-ups 6..9.
typedef int GameItemType;

// GameItem::IsItemTypeEquivalent @0x56807c: whether an item of type a counts as one of type b
inline bool IsItemTypeEquivalent(GameItemType a, GameItemType b) {
    if (a == b || b == 0) return true;
    if (b == 1) return (unsigned)(a - 3) <= 2;
    if (b == 2) return (unsigned)(a - 6) <= 3;
    return false;
}

// GameItem::GameItemTypeToMaxUpgradeStatisticsScope @0x5665a0: the statistic that counts the vehicles and power-ups of the
// type at their last upgrade (3 fluffy, 4 large minion, 5 rocket, 6 magnet, 7 shield, 8 banana multiplier, 9 weapon, 10 bonus cannon)
inline jet::String GameItemTypeToMaxUpgradeStatisticsScope(GameItemType t) {
    switch (t) {
        case 3: return StatisticsSaveData::k_statScope_maxUpgradeVehicleFluffy;
        case 4: return StatisticsSaveData::k_statScope_maxUpgradeVehicleLargeMinion;
        case 5: return StatisticsSaveData::k_statScope_maxUpgradeVehicleRocket;
        case 6: return StatisticsSaveData::k_statScope_maxUpgradePowerUpMagnet;
        case 7: return StatisticsSaveData::k_statScope_maxUpgradePowerUpShield;
        case 8: return StatisticsSaveData::k_statScope_maxUpgradePowerUpBananaMulti;
        case 9: return StatisticsSaveData::k_statScope_maxUpgradePowerUpWeapon;
        case 10: return StatisticsSaveData::k_statScope_maxUpgradeBonusCannon;
        default: return jet::String("");
    }
}

// GameItem::GameItemTypeUnlockedStatisticsScope @0x566668: the statistic that is set when the type is first bought
inline jet::String GameItemTypeUnlockedStatisticsScope(GameItemType t) {
    switch (t) {
        case 3: return StatisticsSaveData::k_statScope_unlockedVehicleFluffy;
        case 4: return StatisticsSaveData::k_statScope_unlockedVehicleLargeMinion;
        case 5: return StatisticsSaveData::k_statScope_unlockedVehicleRocket;
        case 6: return StatisticsSaveData::k_statScope_unlockedPowerUpMagnet;
        case 7: return StatisticsSaveData::k_statScope_unlockedPowerUpShield;
        case 8: return StatisticsSaveData::k_statScope_unlockedPowerUpBananaMulti;
        case 9: return StatisticsSaveData::k_statScope_unlockedPowerUpWeapon;
        case 10: return StatisticsSaveData::k_statScope_unlockedBonusCannon;
        default: return jet::String("");
    }
}

#endif
