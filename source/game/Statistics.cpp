// source/game/Statistics.cpp
#include "Statistics.h"
#include "Game.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/gamemanagers/BackgroundMgr.h"
#include "game/online/store/Price.h"
#include <algorithm>

const jet::String StatisticsSaveData::k_statScope_score("score");
const jet::String StatisticsSaveData::k_statScope_tokens("tokens");
const jet::String StatisticsSaveData::k_statScope_bananas("bananas");
const jet::String StatisticsSaveData::k_statScope_distance("distance");
const jet::String StatisticsSaveData::k_statScope_nearMiss("nearMiss");
const jet::String StatisticsSaveData::k_statScope_bestScore("bestScore");
const jet::String StatisticsSaveData::k_statScope_playXRuns("PlayXRuns");
const jet::String StatisticsSaveData::k_statScope_turnsCount("turnsCount");
const jet::String StatisticsSaveData::k_statScope_usedCannon("usedCannon");
const jet::String StatisticsSaveData::k_statScope_defeatMacho("defeatMacho");
const jet::String StatisticsSaveData::k_statScope_defeatMeena("defeatMeena");
const jet::String StatisticsSaveData::k_statScope_reviveCount("reviveCount");
const jet::String StatisticsSaveData::k_statScope_timeElapsed("timeElapsed");
const jet::String StatisticsSaveData::k_statScope_bapplesCount("bapplesCount");
const jet::String StatisticsSaveData::k_statScope_defeatVector("defeatVector");
const jet::String StatisticsSaveData::k_statScope_pickUpsCount("pickUpsCount");
const jet::String StatisticsSaveData::k_statScope_pitFallCount("pitFallCount");
const jet::String StatisticsSaveData::k_statScope_evilEyesCount("evilEyesCount");
const jet::String StatisticsSaveData::k_statScope_fireLipsCount("fireLipsCount");
const jet::String StatisticsSaveData::k_statScope_upgradesCount("upgradesCount");
const jet::String StatisticsSaveData::k_statScope_useBananaPerk("useBananaPerk");
const jet::String StatisticsSaveData::k_statScope_usedClawSaver("usedClawSaver");
const jet::String StatisticsSaveData::k_statScope_maxUpgradesAll("maxUpgradeAll");
const jet::String StatisticsSaveData::k_statScope_fluffyRideCount("FluffyRideCount");
const jet::String StatisticsSaveData::k_statScope_numberOfFriends("NumberOfFriends");
const jet::String StatisticsSaveData::k_statScope_costumesBuyCount("costumesBuyCount");
const jet::String StatisticsSaveData::k_statScope_largeMinionCount("LargeMinionCount");
const jet::String StatisticsSaveData::k_statScope_secreatAreaCount("secreatAreaCount");
const jet::String StatisticsSaveData::k_statScope_shopSpentBananas("shopSpentBananas");
const jet::String StatisticsSaveData::k_statScope_usedJumpersCount("usedJumpersCount");
const jet::String StatisticsSaveData::k_statScope_usedSpeederCount("usedSpeederCount");
const jet::String StatisticsSaveData::k_statScope_beatDistanceCount("BeatDistanceCount");
const jet::String StatisticsSaveData::k_statScope_bestScoreUploaded("bestScoreUploaded");
const jet::String StatisticsSaveData::k_statScope_challengesWonCount("challengesWonCount");
const jet::String StatisticsSaveData::k_statScope_costumeRunDistance("CostumeRunDistance");
const jet::String StatisticsSaveData::k_statScope_gruRocketRideCount("GruRocketRideCount");
const jet::String StatisticsSaveData::k_statScope_sandCastleHitCount("sandCastleHitCount");
const jet::String StatisticsSaveData::k_statScope_beatBestOwnDistance("BeatBestOwnDistance");
const jet::String StatisticsSaveData::k_statScope_beatScoreMultiplier("beatScoreMultiplier");
const jet::String StatisticsSaveData::k_statScope_unlockedBonusCannon("unlockedBonusCannon");
const jet::String StatisticsSaveData::k_statScope_despicableMultiplier("despicableMultiplier");
const jet::String StatisticsSaveData::k_statScope_dodgedObstaclesCount("dodgedObstaclesCount");
const jet::String StatisticsSaveData::k_statScope_multiplierCountValue("multiplierCountValue");
const jet::String StatisticsSaveData::k_statScope_runDistanceNoBananas("runDistanceNoBananas");
const jet::String StatisticsSaveData::k_statScope_runDistanceNoPickups("runDistanceNoPickups");
const jet::String StatisticsSaveData::k_statScope_tobbogansPassedCount("tobbogansPassedCount");
const jet::String StatisticsSaveData::k_statScope_beatFriendsScoreCount("BeatFriendsScoreCount");
const jet::String StatisticsSaveData::k_statScope_bestScoreAmongFriends("bestScoreAmongFriends");
const jet::String StatisticsSaveData::k_statScope_challengeFriendsCount("ChallengeFriendsCount");
const jet::String StatisticsSaveData::k_statScope_freezeRayObjDestroyed("freezeRayObjDestroyed");
const jet::String StatisticsSaveData::k_statScope_maxUpgradeBonusCannon("maxUpgradeBonusCannon");
const jet::String StatisticsSaveData::k_statScope_residentialEnterCount("residentialEnterCount");
const jet::String StatisticsSaveData::k_statScope_unlockedPowerUpMagnet("unlockedPowerUpMagnet");
const jet::String StatisticsSaveData::k_statScope_unlockedPowerUpShield("unlockedPowerUpShield");
const jet::String StatisticsSaveData::k_statScope_unlockedPowerUpWeapon("unlockedPowerUpWeapon");
const jet::String StatisticsSaveData::k_statScope_unlockedVehicleFluffy("unlockedVehicleFluffy");
const jet::String StatisticsSaveData::k_statScope_unlockedVehicleRocket("unlockedVehicleRocket");
const jet::String StatisticsSaveData::k_statScope_bestWeeklyContestScore("bestWeeklyContestScore");
const jet::String StatisticsSaveData::k_statScope_checkLeaderboardsCount("CheckLeaderboardsCount");
const jet::String StatisticsSaveData::k_statScope_completedMissionsCount("completedMissionsCount");
const jet::String StatisticsSaveData::k_statScope_despicableActionsCount("DespicableActionsCount");
const jet::String StatisticsSaveData::k_statScope_jumpOverObstaclesCount("jumpOverObstaclesCount");
const jet::String StatisticsSaveData::k_statScope_rollUnderHammocksCount("rollUnderHammocksCount");
const jet::String StatisticsSaveData::k_statScope_rollUnderRedTapesCount("rollUnderRedTapesCount");
const jet::String StatisticsSaveData::k_statScope_useScoreMultiplierPerk("useScoreMultiplierPerk");
const jet::String StatisticsSaveData::k_statScope_bananaMultiplierBananas("BananaMultiplierBananas");
const jet::String StatisticsSaveData::k_statScope_maxUpgradePowerUpMagnet("maxUpgradePowerUpMagnet");
const jet::String StatisticsSaveData::k_statScope_maxUpgradePowerUpShield("maxUpgradePowerUpShield");
const jet::String StatisticsSaveData::k_statScope_maxUpgradePowerUpWeapon("maxUpgradePowerUpWeapon");
const jet::String StatisticsSaveData::k_statScope_maxUpgradeVehicleFluffy("maxUpgradeVehicleFluffy");
const jet::String StatisticsSaveData::k_statScope_maxUpgradeVehicleRocket("maxUpgradeVehicleRocket");
const jet::String StatisticsSaveData::k_statScope_nearMissSurfBoardsCount("nearMissSurfBoardsCount");
const jet::String StatisticsSaveData::k_statScope_rollUnderFireBowlsCount("rollUnderFireBowlsCount");
const jet::String StatisticsSaveData::k_statScope_rollUnderObstaclesCount("rollUnderObstaclesCount");
const jet::String StatisticsSaveData::k_statScope_weeklyContestPrizeCount("WeeklyContestPrizeCount");
const jet::String StatisticsSaveData::k_statScope_jumpOverFireTonguesCount("jumpOverFireTonguesCount");
const jet::String StatisticsSaveData::k_statScope_jumpOverSandCastlesCount("jumpOverSandCastlesCount");
const jet::String StatisticsSaveData::k_statScope_distanceWithoutDespicable("distanceWithoutDespicable");
const jet::String StatisticsSaveData::k_statScope_maxReachedScoreMultiplier("maxReachedScoreMultiplier");
const jet::String StatisticsSaveData::k_statScope_nearMissRollingCagesCount("nearMissRollingCagesCount");
const jet::String StatisticsSaveData::k_statScope_jumpOverWetfloorSignsCount("jumpOverWetfloorSignsCount");
const jet::String StatisticsSaveData::k_statScope_unlockedPowerUpBananaMulti("unlockedPowerUpBananaMulti");
const jet::String StatisticsSaveData::k_statScope_unlockedVehicleLargeMinion("unlockedVehicleLargeMinion");
const jet::String StatisticsSaveData::k_statScope_avoidDespicableMinionsCount("avoidDespicableMinionsCount");
const jet::String StatisticsSaveData::k_statScope_maxUpgradePowerUpBananaMulti("maxUpgradePowerUpBananaMulti");
const jet::String StatisticsSaveData::k_statScope_maxUpgradeVehicleLargeMinion("maxUpgradeVehicleLargeMinion");
const jet::String StatisticsSaveData::k_statScope_bestWeeklyContestScoreUploaded("bestWeeklyContestScoreUploaded");
const jet::String StatisticsSaveData::k_statScope_nearMissAdvertisingPanelsCount("nearMissAdvertisingPanelsCount");
const jet::String StatisticsSaveData::k_statScope_tracking_reviveIncentive_shown("tracking_rev_p_d");
const jet::String StatisticsSaveData::k_statScope_tracking_reviveIncentive_notShown("tracking_rev_p_nd");
using jet::String;
typedef StatisticsSaveData::StatMap StatMap;

static GameplayStatisticsValue Value(long long v) {
    GameplayStatisticsValue g;
    g.Set64(v);
    return g;
}

// @0x51e5b0
Statistics::Statistics() : m_notifying(0) {
    for (int loc = -1; loc != 6; loc++) StatData_CreateArrayInfo(loc);
    Reset();
}

// @0x51e78c: seven empty statistic maps for a location, in the data and its backup
void Statistics::StatData_CreateArrayInfo(int location) {
    std::vector<StatMap> v(k_typeCount);
    m_data.locations[location] = v;
    m_backup.locations[location] = v;
}

// @0x51e8a0
void Statistics::ResetStatisticsFor(EStatisticType t) {
    for (std::map<int, std::vector<StatMap> >::iterator i = m_data.locations.begin(); i != m_data.locations.end(); ++i) i->second[t].clear();
}

// @0x51e904: a location whose statistics are empty takes those of another one
void Statistics::MatchStatisticsIfEmpty(int from, int to) {
    std::vector<StatMap> &dst = m_data.locations[to];
    for (size_t i = 0; i < dst.size(); i++)
        if (!dst[i].empty()) return;
    std::vector<StatMap> &src = m_data.locations[from];
    for (size_t i = 0; i < src.size(); i++)
        for (StatMap::iterator it = src[i].begin(); it != src[i].end(); ++it) dst[i][it->first] = it->second;
}

// @0x51e9f4
void Statistics::ResetData(StatisticsSaveData &d) const {
    for (std::map<int, std::vector<StatMap> >::iterator i = d.locations.begin(); i != d.locations.end(); ++i) StatData_ResetArrayInfo(d, i->first);
    d.u18 = d.u1c = 0;
    d.s20 = String("");
    d.weeklyContest = String::null;
    d.lastScore.SetValue(0);
    d.lastScoreHigh.SetValue(0);
    d.lastBananas.SetValue(0);
    d.lastDistance.SetValue(0);
}

// @0x51eac0: the uploaded scores start at -1
void Statistics::StatData_ResetArrayInfo(StatisticsSaveData &d, int location) const {
    std::vector<StatMap> &v = d.locations[location];
    for (size_t i = 0; i < k_typeCount && i < v.size(); i++) {
        v[i].clear();
        v[i][StatisticsSaveData::k_statScope_bestScoreUploaded] = Value(-1);
        v[i][StatisticsSaveData::k_statScope_bestWeeklyContestScoreUploaded] = Value(-1);
    }
}

// @0x51ebc8
void Statistics::StatData_Serialize(jet::stream::IStreamW &s, StatisticsSaveData &d, int location, bool wide) const {
    std::vector<StatMap> &v = d.locations[location];
    unsigned n = (unsigned)v.size();
    s << n;
    for (size_t i = 0; i < v.size(); i++) {
        unsigned count = (unsigned)v[i].size();
        s << count;
        for (StatMap::iterator it = v[i].begin(); it != v[i].end(); ++it) {
            jet::WriteString(s, it->first);
            int iv = it->second.intValue.GetValue();
            s << iv;
            float fv = it->second.floatValue.GetValue();
            s << fv;
            if (wide) { int hi = it->second.highValue.GetValue(); s << hi; }
        }
    }
}

// @0x51ed30: the values of a location that exists; the others are read and dropped
void Statistics::StatData_Deserialize(jet::stream::IStream &s, StatisticsSaveData &d, int location, bool wide) const {
    bool exists = d.locations.find(location) != d.locations.end();
    int count = 0;
    s >> count;
    if (exists) {
        std::vector<StatMap> &v = d.locations[location];
        size_t want = std::max<int>(count, 7);
        v.resize(want);
    }
    for (int i = 0; i < count; i++) {
        if (exists) d.locations[location][i].clear();
        int n = 0;
        s >> n;
        for (int k = 0; k < n; k++) {
            String key;
            int iv = 0;
            float fv = 0;
            s >> key >> iv >> fv;
            // a save made before the values were 64 bits holds signed 32 bit ones
            int hi = iv < 0 ? -1 : 0;
            if (wide) s >> hi;
            GameplayStatisticsValue g;
            g.intValue.SetValue(iv);
            g.highValue.SetValue(hi);
            g.floatValue.SetValue(fv);
            if (exists) d.locations[location][i][key] = g;
        }
    }
}

// @0x51efb0
void Statistics::SaveDataInStream(jet::stream::IStreamW &s, StatisticsSaveData &d) const {
    int version = k_saveGameVersion;
    s << version;
    SerializeV3(s, d);
}

// @0x51effc
bool Statistics::LoadDataFromStream(jet::stream::IStream &s, StatisticsSaveData &d) const {
    int version = 0;
    s >> version;
    if (version == 1) return DeserializeV1(s, d);
    if (version == 2) return DeserializeV2(s, d);
    if (version == 3) return DeserializeV3(s, d);
    return false;
}

#define SD_HEAD(sig) \
    int sg = sig; s << sg; s << d.u18; s << d.u1c; jet::WriteString(s, d.s20); jet::WriteString(s, d.weeklyContest); s << d.u28; \
    { int p = d.lastScore.GetValue(); s << p; p = d.lastBananas.GetValue(); s << p; p = d.lastDistance.GetValue(); s << p; }
#define SD_HEAD_READ \
    s >> d.u18 >> d.u1c >> d.s20 >> d.weeklyContest >> d.u28; \
    { int p = 0; s >> p; d.lastScore.SetValue(p); s >> p; d.lastBananas.SetValue(p); s >> p; d.lastDistance.SetValue(p); }

// @0x51f068
void Statistics::SerializeV1(jet::stream::IStreamW &s, StatisticsSaveData &d) const {
    SD_HEAD(k_signatureV1)
    StatData_Serialize(s, d, -1);
}

// @0x51f13c
bool Statistics::DeserializeV1(jet::stream::IStream &s, StatisticsSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    SD_HEAD_READ
    StatData_Deserialize(s, d, -1);
    return true;
}

// @0x51f218
void Statistics::SerializeV2(jet::stream::IStreamW &s, StatisticsSaveData &d) const {
    SD_HEAD(k_signatureV2)
    unsigned n = (unsigned)d.locations.size();
    s << n;
    for (std::map<int, std::vector<StatMap> >::iterator i = d.locations.begin(); i != d.locations.end(); ++i) {
        int loc = i->first;
        s.Write(&loc, 4);
        StatData_Serialize(s, d, loc);
    }
}

// @0x51f340
bool Statistics::DeserializeV2(jet::stream::IStream &s, StatisticsSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV2) return false;
    SD_HEAD_READ
    unsigned n = 0;
    s >> n;
    for (unsigned i = 0; i < n; i++) {
        int loc = 0;
        s >> loc;
        StatData_Deserialize(s, d, loc);
    }
    return true;
}

void Statistics::SerializeV3(jet::stream::IStreamW &s, StatisticsSaveData &d) const {
    SD_HEAD(k_signatureV3)
    { int hi = d.lastScoreHigh.GetValue(); s << hi; }
    unsigned n = (unsigned)d.locations.size();
    s << n;
    for (std::map<int, std::vector<StatMap> >::iterator i = d.locations.begin(); i != d.locations.end(); ++i) {
        int loc = i->first;
        s.Write(&loc, 4);
        StatData_Serialize(s, d, loc, true);
    }
}

bool Statistics::DeserializeV3(jet::stream::IStream &s, StatisticsSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV3) return false;
    SD_HEAD_READ
    { int hi = 0; s >> hi; d.lastScoreHigh.SetValue(hi); }
    unsigned n = 0;
    s >> n;
    for (unsigned i = 0; i < n; i++) {
        int loc = 0;
        s >> loc;
        StatData_Deserialize(s, d, loc, true);
    }
    return true;
}

// @0x37d12c: what does not fit in 32 bits is clamped (GetInt64Value reads all of it)
int Statistics::GetIntValue(int location, const String &scope, EStatisticType t) {
    return m_data.locations[location][t][scope].GetClamped();
}

long long Statistics::GetInt64Value(int location, const String &scope, EStatisticType t) {
    return m_data.locations[location][t][scope].Get64();
}

const GameplayStatisticsValue &Statistics::GetGameplayStatValue(int location, const String &scope, EStatisticType t) { return m_data.locations[location][t][scope]; }

// the locations a location's statistics are counted for: itself and the ones it is associated with
static std::vector<int> Associated(int location) {
    LocationsMgr *m = LocationsMgr::GetInstance();
    return m->GetAssociatedLocationsFor(location);
}

// @0x51f450
void Statistics::SetStatValueIfBetter(const String &scope, long long v) {
    _SetLocationStatValueIfBetter(-1, scope, v);
    if (BackgroundMgr *b = BackgroundMgr::GetInstance()) SetLocationStatValueIfBetter(b->GetLocation(), scope, v);
}

// @0x51f4c4
void Statistics::SetLocationStatValueIfBetter(int location, const String &scope, long long v) {
    if (location == -1) { _SetLocationStatValueIfBetter(location, scope, v); return; }
    if (!LocationsMgr::GetInstance()) return;
    std::vector<int> a = Associated(location);
    for (size_t i = 0; i < a.size(); i++) _SetLocationStatValueIfBetter(a[i], scope, v);
}

// @0x51f550
void Statistics::_SetLocationStatValueIfBetter(int location, const String &scope, long long v) {
    std::vector<StatMap> &vec = m_data.locations[location];
    for (int t = 0; t < k_typeCount; t++) {
        if (vec[t][scope].Get64() < v) {
            vec[t][scope].Set64(v);
            NotifyObservers(location, scope, t);
        }
    }
}

// @0x51f5ec
void Statistics::SetStatValue(const String &scope, long long v) {
    _SetLocationStatValue(-1, -1, scope, v);
    if (BackgroundMgr *b = BackgroundMgr::GetInstance()) SetLocationStatValue(b->GetLocation(), -1, scope, v);
}

// @0x51f674
void Statistics::SetLocationStatValue(int location, EStatisticType t, const String &scope, long long v) {
    if (location == -1) { _SetLocationStatValue(location, t, scope, v); return; }
    if (!LocationsMgr::GetInstance()) return;
    std::vector<int> a = Associated(location);
    for (size_t i = 0; i < a.size(); i++) _SetLocationStatValue(a[i], t, scope, v);
}

// @0x51f710: type -1 sets the value of every type
void Statistics::_SetLocationStatValue(int location, EStatisticType t, const String &scope, long long v) {
    std::vector<StatMap> &vec = m_data.locations[location];
    if (t == -1) {
        for (int i = 0; i < k_typeCount; i++) {
            vec[i][scope].Set64(v);
            NotifyObservers(location, scope, i);
        }
    } else {
        vec[t][scope].Set64(v);
        NotifyObservers(location, scope, t);
    }
}

// @0x51f7c4
void Statistics::SetNumberOfFriends(int n) {
    std::vector<StatMap> &vec = m_data.locations[-1];
    const String &scope = StatisticsSaveData::k_statScope_numberOfFriends;
    for (int t = 0; t < k_typeCount; t++) {
        if (vec[t][scope].Get64() < n) {
            vec[t][scope].Set64(n);
            NotifyObservers(-1, scope, t);
        }
    }
}

// the all-locations statistic of type 1
#define SET_BEST(scope, v) \
    m_data.locations[-1][1][scope].Set64(v); NotifyObservers(-1, scope, 1)

void Statistics::SetBestScore(long long v) { SET_BEST(StatisticsSaveData::k_statScope_bestScore, v); }
void Statistics::SetBestScoreUploaded(long long v) { SET_BEST(StatisticsSaveData::k_statScope_bestScoreUploaded, v); }

void Statistics::SetBestWeeklyContestScore(long long v, const String &contest) {
    m_data.weeklyContest = contest;
    SET_BEST(StatisticsSaveData::k_statScope_bestWeeklyContestScore, v);
}

void Statistics::SetBestWeeklyContestScoreUploaded(long long v, const String &contest) {
    m_data.weeklyContest = contest;
    SET_BEST(StatisticsSaveData::k_statScope_bestWeeklyContestScoreUploaded, v);
}

// @0x51faa8
void Statistics::IncreaseStatistic(const String &scope, long long n) {
    _IncreaseLocationStat(-1, scope, n);
    if (BackgroundMgr *b = BackgroundMgr::GetInstance()) IncreaseLocationStat(b->GetLocation(), scope, n);
}

// @0x428f28: the 32-bit product of the banana count and the banana value is multiplied by the score multiplier in the .so (ints wrap)
void Statistics::AddBananas(int n, bool addScore) {
    IncreaseStatistic(StatisticsSaveData::k_statScope_bananas, n);
    if (!addScore) return;
    const unsigned value = (unsigned)Game::GetInstance()->GetScoreValue(Game::Score_Banana);
    const int points = (int)(value * (unsigned)n);
    IncreaseStatistic(StatisticsSaveData::k_statScope_score, (long long)(GetScoreMultiplier() * points));
}

// @0x51fb1c
void Statistics::IncreaseLocationStat(int location, const String &scope, long long n) {
    if (location == -1) { _IncreaseLocationStat(location, scope, n); return; }
    if (!LocationsMgr::GetInstance()) return;
    std::vector<int> a = Associated(location);
    for (size_t i = 0; i < a.size(); i++) _IncreaseLocationStat(a[i], scope, n);
}

// @0x51fba8
void Statistics::_IncreaseLocationStat(int location, const String &scope, long long n) {
    std::vector<StatMap> &vec = m_data.locations[location];
    for (int t = 0; t < k_typeCount; t++) {
        GameplayStatisticsValue &g = vec[t][scope];
        g.Set64(g.Get64() + n);
        NotifyObservers(location, scope, t);
    }
}

// @0x51fc18
void Statistics::IncreaseStatisticFloat(const String &scope, float f) {
    _IncreaseLocationStatFloat(-1, scope, f);
    if (BackgroundMgr *b = BackgroundMgr::GetInstance()) IncreaseLocationStatFloat(b->GetLocation(), scope, f);
}

// @0x51fc8c
void Statistics::IncreaseLocationStatFloat(int location, const String &scope, float f) {
    if (location == -1) { _IncreaseLocationStatFloat(location, scope, f); return; }
    if (!LocationsMgr::GetInstance()) return;
    std::vector<int> a = Associated(location);
    for (size_t i = 0; i < a.size(); i++) _IncreaseLocationStatFloat(a[i], scope, f);
}

// @0x51fd18
void Statistics::_IncreaseLocationStatFloat(int location, const String &scope, float f) {
    std::vector<StatMap> &vec = m_data.locations[location];
    for (int t = 0; t < k_typeCount; t++) {
        GameplayStatisticsValue &v = vec[t][scope];
        v.floatValue.SetValue(v.floatValue.GetValue() + f);
        NotifyObservers(location, scope, t);
    }
}

// @0x51fdac: the observers of that type and location; the ones removed meanwhile go when the last notification ends
void Statistics::NotifyObservers(int location, const String &scope, EStatisticType t) {
    GameplayStatisticsValue &v = m_data.locations[location][t][scope];
    m_notifying++;
    for (std::list<StatisticsObserver*>::iterator i = m_observers.begin(); i != m_observers.end(); ++i) {
        StatisticsObserver *o = *i;
        if (o->m_registered && o->m_type == t && o->m_location == location) o->OnStatisticChanged(o->m_location, scope, v);
    }
    if (--m_notifying != 0) return;
    for (std::list<StatisticsObserver*>::iterator i = m_observersToRemove.begin(); i != m_observersToRemove.end(); ++i) UnregisterObserver(*i);
    m_observersToRemove.clear();
}

// @0x51fe9c
void Statistics::RegisterObserver(StatisticsObserver *o) {
    if (std::find(m_observers.begin(), m_observers.end(), o) != m_observers.end()) return;
    m_observers.push_back(o);
    o->m_registered = true;
}

// @0x51fef0
void Statistics::UnregisterObserver(StatisticsObserver *o) {
    o->m_registered = false;
    if (m_notifying < 1) m_observers.remove(o);
    else m_observersToRemove.push_back(o);
}

// @0x520430
bool Statistics::IsScopeCodeImplemented(const String &scope) {
    return scope != StatisticsSaveData::k_statScope_dodgedObstaclesCount && scope != StatisticsSaveData::k_statScope_turnsCount &&
           scope != StatisticsSaveData::k_statScope_avoidDespicableMinionsCount;
}

void Statistics::SetLastScore64(long long v) {
    m_data.lastScore.SetValue((int)(unsigned)(v & 0xffffffffLL));
    m_data.lastScoreHigh.SetValue((int)(v >> 32));
}

long long Statistics::GetLastScore64() const {
    return ((long long)m_data.lastScoreHigh.GetValue() << 32) | (long long)(unsigned)m_data.lastScore.GetValue();
}

// @0x520494
void Statistics::UpdateLastRunValues() {
    SetLastScore64(GetScore());
    m_data.lastBananas.SetValue(GetCurrentRunBananas());
    m_data.lastDistance.SetValue(GetRanGameDistance());
}

// @0x5204d8
void Statistics::SetLastRunValues(long long score, int bananas, int distance) {
    SetLastScore64(score);
    m_data.lastBananas.SetValue(bananas);
    m_data.lastDistance.SetValue(distance);
}

// @0x52050c
void Statistics::GetLastRunValues(long long &score, int &bananas, int &distance) {
    score = GetLastScore64();
    bananas = m_data.lastBananas.GetValue();
    distance = m_data.lastDistance.GetValue();
}

// @0x520548
bool Statistics::HasLastRunValues() {
    return GetLastScore64() > 0 && m_data.lastBananas.GetValue() > 0 && m_data.lastDistance.GetValue() > 0;
}

// @0x520680: entering a location the residential one counts as
void Statistics::Location_IncreaseEnterLocationCount(int location) {
    LocationsMgr *m = LocationsMgr::GetInstance();
    if (!m) return;
    for (int l : m->GetAssociatedLocationsFor(1))
        if (location == l) IncreaseStatistic(StatisticsSaveData::k_statScope_residentialEnterCount, 1);
}

// @0x428eb4: what was paid in bananas counts as spent in the shop (the rate-the-game manager is told of a big event
// by an important one, it is not ported)
void Statistics::AddShopSpentPrice(const Price &price, bool) {
    if (price.GetPriceType() != Price::Bananas) return;
    IncreaseStatistic(StatisticsSaveData::k_statScope_shopSpentBananas, (int)price.GetPriceAsDouble());
}

// @0x428d0c: the level of the multiplier the missions reached, and the extra points the costumes give
int Statistics::GetBaseScoreMultiplier() { return (int)MissionMgr::GetInstance()->GetMultiplierLevel() + 1 + CostumeMgr::GetInstance()->GetGlobalExtraDespicablePointsFactor(false); }

// @0x428b10: the meters since the last call add to the distance statistics (and those that count the run without bananas, pickups or despicable
// actions, while they are still going), and to the score by the points of a meter and the multiplier
bool Statistics::SetRanGameDistance(unsigned meters) {
    const int ran = (int)meters - GetRanGameDistance();
    if (ran <= 0) return false;
    IncreaseStatistic(StatisticsSaveData::k_statScope_distance, ran);
    IncreaseStatistic(StatisticsSaveData::k_statScope_beatBestOwnDistance, ran);
    for (const jet::String *scope : { &StatisticsSaveData::k_statScope_runDistanceNoBananas, &StatisticsSaveData::k_statScope_runDistanceNoPickups,
                                      &StatisticsSaveData::k_statScope_distanceWithoutDespicable })
        if (GetIntValue(-1, *scope, 0) >= 0) IncreaseStatistic(*scope, ran);
    IncreaseStatistic(StatisticsSaveData::k_statScope_score, (long long)ran * Game::GetInstance()->GetScoreValue(Game::Score_Meter) * GetScoreMultiplier());
    if (CostumeMgr::GetInstance()->IsCurrentCostumeADressup()) IncreaseStatistic(StatisticsSaveData::k_statScope_costumeRunDistance, ran);
    return true;
}
