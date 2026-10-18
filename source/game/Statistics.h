// source/game/Statistics.h
#ifndef SOURCE_GAME_STATISTICS_H
#define SOURCE_GAME_STATISTICS_H

#include <list>
#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "core/protectedstorage.h"
#include "game/utils/GameSaveable.h"

// One statistic: an integer and a float, both protected
// The integer is 64 bits wide (a score goes past 2,147,483,647): the low word is the .so's integer and the high word sits
// beside it; both are 32 bit values, so 32 bit devices need nothing special (long long is a plain pair of registers there)
struct GameplayStatisticsValue {
    jet::core::ProtectedStorage<int> intValue;      // 0x0: the low 32 bits
    jet::core::ProtectedStorage<float> floatValue;  // 0x28
    jet::core::ProtectedStorage<int> highValue;     // the high 32 bits (0, or -1 for a negative value)
    long long Get64() const { return ((long long)highValue.GetValue() << 32) | (long long)(unsigned)intValue.GetValue(); }
    void Set64(long long v) { intValue.SetValue((int)(unsigned)(v & 0xffffffffLL)); highValue.SetValue((int)(v >> 32)); }
    // the value an int reads: what does not fit in 32 bits is clamped
    int GetClamped() const { long long v = Get64(); return v > 2147483647LL ? 2147483647 : v < -2147483647LL - 1 ? (int)(-2147483647LL - 1) : (int)v; }
};

// What a scope's value is counted over: 0 the current run, 1 the best, ... (7 kinds)
typedef int EStatisticType;

// Scope names of the statistics (the "scope" of a mission condition)
struct StatisticsSaveData {
    typedef std::map<jet::String, GameplayStatisticsValue> StatMap;
    static const jet::String k_statScope_score,
        k_statScope_tokens,
        k_statScope_bananas,
        k_statScope_distance,
        k_statScope_nearMiss,
        k_statScope_bestScore,
        k_statScope_playXRuns,
        k_statScope_turnsCount,
        k_statScope_usedCannon,
        k_statScope_defeatMacho,
        k_statScope_defeatMeena,
        k_statScope_reviveCount,
        k_statScope_timeElapsed,
        k_statScope_bapplesCount,
        k_statScope_defeatVector,
        k_statScope_pickUpsCount,
        k_statScope_pitFallCount,
        k_statScope_evilEyesCount,
        k_statScope_fireLipsCount,
        k_statScope_upgradesCount,
        k_statScope_useBananaPerk,
        k_statScope_usedClawSaver,
        k_statScope_maxUpgradesAll,
        k_statScope_fluffyRideCount,
        k_statScope_numberOfFriends,
        k_statScope_costumesBuyCount,
        k_statScope_largeMinionCount,
        k_statScope_secreatAreaCount,
        k_statScope_shopSpentBananas,
        k_statScope_usedJumpersCount,
        k_statScope_usedSpeederCount,
        k_statScope_beatDistanceCount,
        k_statScope_bestScoreUploaded,
        k_statScope_challengesWonCount,
        k_statScope_costumeRunDistance,
        k_statScope_gruRocketRideCount,
        k_statScope_sandCastleHitCount,
        k_statScope_beatBestOwnDistance,
        k_statScope_beatScoreMultiplier,
        k_statScope_unlockedBonusCannon,
        k_statScope_despicableMultiplier,
        k_statScope_dodgedObstaclesCount,
        k_statScope_multiplierCountValue,
        k_statScope_runDistanceNoBananas,
        k_statScope_runDistanceNoPickups,
        k_statScope_tobbogansPassedCount,
        k_statScope_beatFriendsScoreCount,
        k_statScope_bestScoreAmongFriends,
        k_statScope_challengeFriendsCount,
        k_statScope_freezeRayObjDestroyed,
        k_statScope_maxUpgradeBonusCannon,
        k_statScope_residentialEnterCount,
        k_statScope_unlockedPowerUpMagnet,
        k_statScope_unlockedPowerUpShield,
        k_statScope_unlockedPowerUpWeapon,
        k_statScope_unlockedVehicleFluffy,
        k_statScope_unlockedVehicleRocket,
        k_statScope_bestWeeklyContestScore,
        k_statScope_checkLeaderboardsCount,
        k_statScope_completedMissionsCount,
        k_statScope_despicableActionsCount,
        k_statScope_jumpOverObstaclesCount,
        k_statScope_rollUnderHammocksCount,
        k_statScope_rollUnderRedTapesCount,
        k_statScope_useScoreMultiplierPerk,
        k_statScope_bananaMultiplierBananas,
        k_statScope_maxUpgradePowerUpMagnet,
        k_statScope_maxUpgradePowerUpShield,
        k_statScope_maxUpgradePowerUpWeapon,
        k_statScope_maxUpgradeVehicleFluffy,
        k_statScope_maxUpgradeVehicleRocket,
        k_statScope_nearMissSurfBoardsCount,
        k_statScope_rollUnderFireBowlsCount,
        k_statScope_rollUnderObstaclesCount,
        k_statScope_weeklyContestPrizeCount,
        k_statScope_jumpOverFireTonguesCount,
        k_statScope_jumpOverSandCastlesCount,
        k_statScope_distanceWithoutDespicable,
        k_statScope_maxReachedScoreMultiplier,
        k_statScope_nearMissRollingCagesCount,
        k_statScope_jumpOverWetfloorSignsCount,
        k_statScope_unlockedPowerUpBananaMulti,
        k_statScope_unlockedVehicleLargeMinion,
        k_statScope_avoidDespicableMinionsCount,
        k_statScope_maxUpgradePowerUpBananaMulti,
        k_statScope_maxUpgradeVehicleLargeMinion,
        k_statScope_bestWeeklyContestScoreUploaded,
        k_statScope_nearMissAdvertisingPanelsCount,
        k_statScope_tracking_reviveIncentive_shown,
        k_statScope_tracking_reviveIncentive_notShown;
    // per location (-1 = all), the values of every statistic type
    std::map<int, std::vector<StatMap> > locations;  // 0x4
    int u18, u1c;                                    // 0x18
    jet::String s20, weeklyContest;                  // 0x20 0x24
    int u28;                                         // 0x28
    jet::core::ProtectedStorage<int> lastScore;      // 0x2c
    jet::core::ProtectedStorage<int> lastScoreHigh;  // its high 32 bits
    jet::core::ProtectedStorage<int> lastBananas;    // 0x54
    jet::core::ProtectedStorage<int> lastDistance;   // 0x7c
    StatisticsSaveData() : u18(0), u1c(0), u28(0) {}
};

// Watches one statistic of a location
class StatisticsObserver {
public:
    StatisticsObserver() : m_registered(false), m_type(0), m_location(-1) {}
    virtual ~StatisticsObserver() {}
    virtual void OnStatisticChanged(int location, const jet::String &scope, const GameplayStatisticsValue &value) = 0;
    bool m_registered;     // 0x8
    EStatisticType m_type; // 0x4
    int m_location;        // 0xc
};

// The gameplay statistics (counts the missions and challenges read), saved as the record "Statistics"
class Price;
class Statistics : public Singleton<Statistics>, public GameSaveable<StatisticsSaveData> {  // sizeof 0x164
public:
    static const int k_signatureV1 = 0xaa0009, k_signatureV2 = 0xaa000b, k_signatureV3 = 0xaa000c, k_saveGameVersion = 3;  // 3: 64 bit values
    static const int k_typeCount = 7;
    Statistics();
    virtual ~Statistics() {}
    virtual void ResetData(StatisticsSaveData &d) const;
    virtual void SaveDataInStream(jet::stream::IStreamW &s, StatisticsSaveData &d) const;
    virtual bool LoadDataFromStream(jet::stream::IStream &s, StatisticsSaveData &d) const;
    void Reset() { ResetData(m_data); }
    void StatData_CreateArrayInfo(int location);
    void ResetStatisticsFor(EStatisticType t);
    void MatchStatisticsIfEmpty(int from, int to);

    int GetIntValue(int location, const jet::String &scope, EStatisticType t);  // clamped to 32 bits
    long long GetInt64Value(int location, const jet::String &scope, EStatisticType t);
    const GameplayStatisticsValue &GetGameplayStatValue(int location, const jet::String &scope, EStatisticType t);
    // @0x428d0c: the mission multiplier's level plus one, and what the costumes add
    int GetBaseScoreMultiplier();
    // @0x428cb0: the multiplier the despicable points give plus the missions' and the costume's
    int GetScoreMultiplier() { return GetDepicableMultiplier() + GetBaseScoreMultiplier(); }
    // @0x428b10: the run went on to this distance (meters): what it ran since is counted in the distance statistics and the score; false when it did not go farther
    bool SetRanGameDistance(unsigned meters);
    long long GetBestScore() { return GetInt64Value(-1, StatisticsSaveData::k_statScope_bestScore, 1); }
    int GetRanGameDistance() { return GetIntValue(-1, StatisticsSaveData::k_statScope_distance, 0); }
    long long GetBestWeeklyContestScore() { return GetInt64Value(-1, StatisticsSaveData::k_statScope_bestWeeklyContestScore, 1); }
    int GetCurrentRunBananas() { return GetIntValue(-1, StatisticsSaveData::k_statScope_bananas, 0); }
    long long GetScore() { return GetInt64Value(-1, StatisticsSaveData::k_statScope_score, 0); }
    int GetDepicableMultiplier() { return GetIntValue(-1, StatisticsSaveData::k_statScope_despicableMultiplier, 0); }

    void SetStatValueIfBetter(const jet::String &scope, long long v);
    void SetLocationStatValueIfBetter(int location, const jet::String &scope, long long v);
    void SetStatValue(const jet::String &scope, long long v);
    void SetLocationStatValue(int location, EStatisticType t, const jet::String &scope, long long v);
    void SetNumberOfFriends(int n);
    void SetBestScore(long long v);
    void SetBestScoreUploaded(long long v);
    void SetBestWeeklyContestScore(long long v, const jet::String &contest);
    void SetBestWeeklyContestScoreUploaded(long long v, const jet::String &contest);
    void IncreaseStatistic(const jet::String &scope, long long n);
    // @0x428f28: the run's bananas count up; when addScore is set the score takes the score multiplier times the banana value (Game::Score_Banana) per banana
    void AddBananas(int n, bool addScore);
    // raises the greatest run distance (.so +0x20) to distance when it is greater; true when it was raised
    bool RaiseDistanceRecord(int distance) {
        if (distance <= m_distanceRecord) return false;
        m_distanceRecord = distance;
        return true;
    }
    int GetDistanceRecord() const { return m_distanceRecord; }
    void AddShopSpentPrice(const Price&, bool importantEvent);
    void Location_IncreaseEnterLocationCount(int location);
    void IncreaseLocationStat(int location, const jet::String &scope, long long n);
    void IncreaseStatisticFloat(const jet::String &scope, float f);
    void IncreaseLocationStatFloat(int location, const jet::String &scope, float f);
    void RegisterObserver(StatisticsObserver *o);
    void UnregisterObserver(StatisticsObserver *o);
    static bool IsScopeCodeImplemented(const jet::String &scope);
    void UpdateLastRunValues();
    void SetLastScore64(long long v);
    long long GetLastScore64() const;
    void SetLastRunValues(long long score, int bananas, int distance);
    void GetLastRunValues(long long &score, int &bananas, int &distance);
    bool HasLastRunValues();
private:
    void StatData_ResetArrayInfo(StatisticsSaveData &d, int location) const;
    void StatData_Serialize(jet::stream::IStreamW &s, StatisticsSaveData &d, int location, bool wide = false) const;
    void StatData_Deserialize(jet::stream::IStream &s, StatisticsSaveData &d, int location, bool wide = false) const;
    void SerializeV1(jet::stream::IStreamW &s, StatisticsSaveData &d) const;
    bool DeserializeV1(jet::stream::IStream &s, StatisticsSaveData &d) const;
    void SerializeV2(jet::stream::IStreamW &s, StatisticsSaveData &d) const;
    bool DeserializeV2(jet::stream::IStream &s, StatisticsSaveData &d) const;
    // V3: V2 with the high 32 bits of every integer (and of the last score) after the low ones
    void SerializeV3(jet::stream::IStreamW &s, StatisticsSaveData &d) const;
    bool DeserializeV3(jet::stream::IStream &s, StatisticsSaveData &d) const;
    void _SetLocationStatValueIfBetter(int location, const jet::String &scope, long long v);
    void _SetLocationStatValue(int location, EStatisticType t, const jet::String &scope, long long v);
    void _IncreaseLocationStat(int location, const jet::String &scope, long long n);
    void _IncreaseLocationStatFloat(int location, const jet::String &scope, float f);
    void NotifyObservers(int location, const jet::String &scope, EStatisticType t);
    // .so +0x20: the greatest run distance seen (UpdateEndGameStatistics raises it); the .so keeps it with the statistics object
    int m_distanceRecord = 0;
    std::list<StatisticsObserver*> m_observers;         // 0x150
    int m_notifying;                                    // 0x158
    std::list<StatisticsObserver*> m_observersToRemove; // 0x15c
};

#endif
