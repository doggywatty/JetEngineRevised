// source/game/entities/gameitem/GameItem.cpp
#include "GameItem.h"
#include "../Bonus.h"
#include "../BonusSet.h"
#include "../Deco3d.h"
#include "../Minion.h"
#include "../MinionCostume.h"
#include "../../GameLevel.h"
#include "../../Player.h"
#include "../../Statistics.h"
#include "../../gamestates/GS_Gameplay.h"
#include "../../gamemanagers/BappleMgr.h"
#include "../../gamemanagers/CostumeMgr.h"
#include "../../gamemanagers/MinionSpeedMgr.h"
#include "../../gamemanagers/PowerUpMgr.h"
#include "../../utils/ControlledRandom.h"
#include "param.h"
#include <cmath>

jet::String GameItem::k_tmplName("GameItem");
const jet::String GameItem::k_gameItemTypeParam("GameItemType");

// the values the data starts them with (the gameplay definition's parameters replace them)
int GameItem::s_lastSelected = 0;
float GameItem::s_creationSafeDistance = 0;
float GameItem::s_minimumItemSpace = 60.0f;
float GameItem::s_maximumItemSpace = 120.0f;
float GameItem::s_initialSpace = 100.0f;
float GameItem::s_probabilityValue = 0.01f;
float GameItem::s_itemCreationProximityThresholdSq = 1600.0f;
GameItem::GameItemProbability GameItem::s_allGameItems[7] = {{3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}};
GameItem::GameItemProbability GameItem::s_powerUps[4] = {{6, 0}, {7, 0}, {8, 0}, {9, 0}};
GameItem::GameItemProbability GameItem::s_vehicles[3] = {{3, 0}, {4, 0}, {5, 0}};

// the type the level forces when no bapple objective is active: none
static const GameItemType k_noForcedItem = 14;
// the rocket (the side-look tutorial keeps it out of the items)
static const GameItemType k_rocketType = 5;

// the power up each power up item turns on (the items 6..9 in order)
static const EPowerUpDef::type k_powerUpOfItem[4] = {EPowerUpDef::Magnet, EPowerUpDef::Shield, EPowerUpDef::BananaMultiplier, EPowerUpDef::Weapon};

// @0x42b76c: the bapple spot of a vehicle (the types 3..5), -1 for the other items
static int ItemTypeToBappleSpot(GameItemType t) {
    static const int k_spots[3] = {5, 7, 6};
    return t >= 3 && t <= 5 ? k_spots[t - 3] : -1;
}

// the bonus of the type at the level bought (@0x5a4334 BonusSet::GetBonus); a type with no bonus is locked
static const Bonus *GetBonusOf(GameItemType t, bool golden) { return BonusSet::GetInstance()->GetBonus(t, golden); }

// the weight of the type's bonus at its current level (the probability before any perk or the scaling of the lists)
static float BonusWeight(GameItemType t) {
    const Bonus *b = GetBonusOf(t, false);
    return b ? b->GetProbabilityWeight(-2) : 0;
}

// a perk doubles every item's probability (the banana or the despicable multiplier is bought for the run)
static float PerkMultiplier() {
    PowerUpMgr *powerUps = PowerUpMgr::GetInstance();
    return powerUps->HasPerk(PowerUpMgr::EPowerUpPerkDef::BananaMultiplier) || powerUps->HasPerk(PowerUpMgr::EPowerUpPerkDef::DespicableMultiplier) ? 2.0f : 1.0f;
}

// the time (ms) the power up of the type is on: the bonus's amount at the level bought, in seconds, and the costume's extra time for it
static int PowerUpTime(GameItemType t) {
    const Bonus *b = GetBonusOf(t, false);
    const int seconds = b ? b->GetAmount(-2) : 0;
    return 1000 * seconds + CostumeMgr::GetInstance()->GetEquippedCostumeRef()->GetExtraTimeForPowerUp(t);
}

// @0x537228 (GameLevel::PostInit): the distance the next item is built at starts at the initial space, and no power up is on
void GameItem::ResetLastCreatedDistance() {
    s_creationSafeDistance = s_initialSpace;
    PowerUpMgr::s_isPowerUpActive = false;
}

// @0x5664d8
GameItem::GameItem(const clara::Template *t) : CollectableItem(t) { m_type = 4; }

// @0x566538
GameItem::~GameItem() {}

// @0x566970
void GameItem::Init() {
    if (m_flags & F_LIBRARY) return;
    CollectableItem::Init();
}

// @0x566980: the item's type is read; a single item (types above the groups) is built here unless its bonus is locked
void GameItem::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GetParam(k_gameItemTypeParam, m_gameItemType);
    if (m_gameItemType > 2) {
        if (IsBonusLocked()) {
            m_creationSkipped = true;
            m_skippedReason = SkipReason::BonusLocked;
        } else
            BuildItem();
    }
    RegisterForUpdate(true);
    RegisterForRender(true);
}

// @0x566ad0: the item's bonus has nothing bought yet (its level is below 0); a type with no bonus counts as locked
bool GameItem::IsBonusLocked() const {
    const Bonus *b = GetBonusOf(m_gameItemType, false);
    return !b || b->GetCurrentLevel() < 0;
}

// @0x566b0c: builds the item: a group is drawn from its probabilities (the item the level forces first, and while the side-look tutorial is not
// done no rocket, and the items it lacks are left out); the chosen item gets its deco and effect (SetNewData)
void GameItem::BuildItem() {
    if (m_gameItemType <= 2) {
        if (AreAllItemsLocked()) return SkipCreation(SkipReason::ItemUnavailable);
        CalculateItemsProbability();
        if (!VerifyGeneralProbability()) return SkipCreation(SkipReason::Probability);
        CalculateVehiclesProbability();
        CalculatePowerUpsProbability();
        if (!Player::GetInstance()->IsSideLookTutorialDone()) {
            if (GetForcedItem(m_gameItemType) == 5) return SkipCreation(SkipReason::BonusLocked);
            if (m_gameItemType == 0) {
                if (GetRandomItem() == k_rocketType) return SkipCreation(SkipReason::BonusLocked);
            } else if (m_gameItemType == 1 && GetRandomVehicle() == k_rocketType)
                return SkipCreation(SkipReason::BonusLocked);
        }
    }
    // @0x566be0
    CollectableItem::PostInit();
    bool chosen = false;
    const GameItemType forced = GetForcedItem(m_gameItemType);
    if (forced == k_noForcedItem) {
        if (m_gameItemType == 0) {
            m_gameItemType = GetRandomItem();
            chosen = true;
        } else if (m_gameItemType == 1) {
            m_gameItemType = GetRandomVehicle();
            chosen = true;
        } else if (m_gameItemType == 2) {
            m_gameItemType = GetRandomPowerUp();
            chosen = true;
        }
    } else {
        m_gameItemType = forced;
        chosen = true;
    }
    if (m_gameItemType == k_rocketType && !Player::GetInstance()->IsSideLookTutorialDone()) {
        m_gameItemType = 7;
        chosen = true;
    }
    if (chosen) SetNewData();
    m_created = true;
}

// @0x567918: the item's deco and effect are the ones of its bonus (a golden shield when the golden one is bought)
void GameItem::SetNewData() {
    DestroyDeco3d();
    const Bonus *bonus = 0;
    switch (m_gameItemType) {
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
        case 9: bonus = GetBonusOf(m_gameItemType, false); break;
        case 7: {
            const Bonus *golden = GetBonusOf(7, true);
            bonus = golden && golden->GetCurrentLevel() >= 0 ? golden : GetBonusOf(7, false);
        } break;
        default: break;
    }
    m_effectsOnContact.clear();
    if (!bonus) return;
    if (const clara::DataEntity *deco = bonus->GetDecoRef()) InitDeco3d(dynamic_cast<Deco3d*>(const_cast<clara::DataEntity*>(deco)));
    if (const clara::DataEntity *effect = bonus->GetEffectRef()) m_effectsOnContact.push_back(reinterpret_cast<const EffectDef*>(effect));
}

// @0x566d64: whether the level builds the next item now: the minion is near enough and has run past the distance of the last item, and no
// power up is on. An item that is near but not built is skipped with the reason (too early, or a power up on).
bool GameItem::ShouldCreateItem() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion) return false;
    const float traveled = (float)minion->GetTraveledDistance();
    const float distanceSq = (minion->GetPosition() - GetPosition()).getLengthSq();
    const bool near = distanceSq < s_itemCreationProximityThresholdSq;
    const bool far = traveled > s_creationSafeDistance;
    const bool powerUpOn = PowerUpMgr::s_isPowerUpActive;
    if (near && (!far || powerUpOn)) m_creationSkipped = true;
    if (m_creationSkipped) {
        if (!far) m_skippedReason = SkipReason::Distance;
        else if (powerUpOn) m_skippedReason = SkipReason::PowerUp;
    }
    return near && far && !powerUpOn;
}

// @0x56703c: the items' probabilities are their bonus weights (doubled by a perk); the item last built and, while the side-look tutorial is not
// done, the rocket when another item has a probability are left out; the rest is scaled to 100
void GameItem::CalculateItemsProbability() {
    std::list<GameItemType> types;
    for (GameItemProbability &p : s_allGameItems) {
        p.probability = BonusWeight(p.type) * PerkMultiplier();
        types.push_back(p.type);
    }
    if (s_lastSelected != 14) RemoveProbability(s_allGameItems, 7, s_lastSelected, types);
    if (!Player::GetInstance()->IsSideLookTutorialDone()) {
        bool otherItem = false;
        for (const GameItemProbability &p : s_allGameItems)
            if (p.type != k_rocketType && p.probability != 0) {
                otherItem = true;
                break;
            }
        if (otherItem) RemoveProbability(s_allGameItems, 7, k_rocketType, types);
    }
    ScaleProbabilities(s_allGameItems, 7, types);
}

// @0x5672b0: the vehicles' probabilities are their bonus weights; the last one built and, while the side-look tutorial is not done, the rocket
// when another vehicle has a probability are left out; the rest is scaled to 100
void GameItem::CalculateVehiclesProbability() {
    std::list<GameItemType> types;
    for (GameItemProbability &p : s_vehicles) {
        p.probability = BonusWeight(p.type);
        types.push_back(p.type);
    }
    if (s_lastSelected != 14) RemoveProbability(s_vehicles, 3, s_lastSelected, types);
    if (!Player::GetInstance()->IsSideLookTutorialDone()) {
        bool otherVehicle = false;
        for (const GameItemProbability &p : s_vehicles)
            if (p.type != k_rocketType && p.probability != 0) {
                otherVehicle = true;
                break;
            }
        if (otherVehicle) RemoveProbability(s_vehicles, 3, k_rocketType, types);
    }
    ScaleProbabilities(s_vehicles, 3, types);
}

// @0x5674a8: the power ups' probabilities are their bonus weights (doubled by a perk); the last one built is left out; scaled to 100
void GameItem::CalculatePowerUpsProbability() {
    std::list<GameItemType> types;
    for (GameItemProbability &p : s_powerUps) {
        p.probability = BonusWeight(p.type) * PerkMultiplier();
        types.push_back(p.type);
    }
    if (s_lastSelected != 14) RemoveProbability(s_powerUps, 4, s_lastSelected, types);
    ScaleProbabilities(s_powerUps, 4, types);
}

// @0x566e88: every item's bonus is locked
bool GameItem::AreAllItemsLocked() const {
    bool locked = true;
    for (const GameItemProbability &p : s_allGameItems) {
        const Bonus *b = GetBonusOf(p.type, false);
        if (b && b->GetCurrentLevel() >= 0) locked = false;
    }
    return locked;
}

// @0x566ee8: the probability of the group (its items' weights, doubled by a perk) against the total the data asks for: a total above it always
// passes, a smaller one passes by a random draw
bool GameItem::VerifyGeneralProbability() const {
    float total = 0;
    for (const GameItemProbability &p : s_allGameItems) total += PerkMultiplier() * BonusWeight(p.type);
    if (total <= 0) return false;
    if (total > s_probabilityValue) return true;
    const float draw = GameLevel::s_randomizer[3]->Randf(0.0f, 1.0f);
    return draw < total / s_probabilityValue;
}

// @0x5676c4: an item by the probabilities; the last one holds when none of the others is reached
GameItemType GameItem::GetRandomItem() {
    const float r = GameLevel::s_randomizer[3]->Randf(0.0f, 100.0f);
    for (int i = 6; i > 0; i--)
        if (r - s_allGameItems[i].probability <= 0) return s_allGameItems[i].type;
    return s_allGameItems[0].type;
}

// @0x567738: a vehicle by the probabilities (the first in the table is the last to be drawn)
GameItemType GameItem::GetRandomVehicle() {
    const float r = GameLevel::s_randomizer[3]->Randf(0.0f, 100.0f);
    const float afterLast = r - s_vehicles[2].probability;
    if (afterLast <= 0) return s_vehicles[2].type;
    if (afterLast - s_vehicles[1].probability <= 0) return s_vehicles[1].type;
    return s_vehicles[0].type;
}

// @0x5677c0: a power up by the probabilities (the first in the table is the last to be drawn)
GameItemType GameItem::GetRandomPowerUp() {
    const float r = GameLevel::s_randomizer[3]->Randf(0.0f, 100.0f);
    const float afterLast = r - s_powerUps[3].probability;
    if (afterLast <= 0) return s_powerUps[3].type;
    const float afterSecond = afterLast - s_powerUps[2].probability;
    if (afterSecond <= 0) return s_powerUps[2].type;
    if (afterSecond - s_powerUps[1].probability <= 0) return s_powerUps[1].type;
    return s_powerUps[0].type;
}

// @0x567864: the item the level forces for a group (any item, or a vehicle): the first with a probability whose bapple spot has an objective
// active; none (14) otherwise and for the single types
GameItemType GameItem::GetForcedItem(const GameItemType &t) const {
    if (t > 1) return k_noForcedItem;
    const GameItemProbability *arr = t == 0 ? s_allGameItems : s_vehicles;
    const int count = t == 0 ? 7 : 3;
    for (int i = 0; i < count; i++) {
        if (arr[i].probability == 0) continue;
        const int spot = ItemTypeToBappleSpot(arr[i].type);
        if (spot == -1) continue;
        if (BappleMgr::GetInstance()->AreActiveObjectivesInSpot(spot)) return arr[i].type;
    }
    return k_noForcedItem;
}

// @0x567688: the index of the type in the table (from the last entry)
bool GameItem::FindGameItemProbability(const GameItemProbability *arr, int count, GameItemType type, int &index) const {
    for (int i = count - 1; i >= 0; i--)
        if (arr[i].type == type) {
            index = i;
            return true;
        }
    return false;
}

// the type's entry is left out of the probabilities and of the list
void GameItem::RemoveProbability(GameItemProbability *arr, int count, GameItemType type, std::list<GameItemType> &types) const {
    int i = 0;
    if (!FindGameItemProbability(arr, count, type, i)) return;
    const GameItemType left = arr[i].type;
    arr[i].probability = 0;
    types.remove(left);
}

// the probabilities of the list's types are scaled so that they add up to 100 (the sum is of the whole table)
void GameItem::ScaleProbabilities(GameItemProbability *arr, int count, const std::list<GameItemType> &types) const {
    float total = 0;
    for (int i = 0; i < count; i++) total += arr[i].probability;
    const float scale = 100.0f / total;
    for (GameItemType t : types) {
        int i = 0;
        if (FindGameItemProbability(arr, count, t, i)) arr[i].probability *= scale;
    }
}

// @0x566730: the minion took the item: a power up is on for its time, and the statistics count it. The large minion (4) starts its mode
// (GS_Gameplay::BeginLargeMinionGameplay @0x4fd420). The fluffy (3, only off the sledge and on a run, BeginFluffyGameplay @0x4fd344) and the
// rocket (5, BeginRocketGameplay @0x4fd940) are not in the tree.
void GameItem::Collect(Minion *minion) {
    (void)minion;
    if (getenv("JET_DBGMINION")) fprintf(stderr, "GameItem collected type %d at (%.1f %.1f %.1f)\n", (int)m_gameItemType, GetPosition().x, GetPosition().y, GetPosition().z);
    PowerUpMgr *powerUps = PowerUpMgr::GetInstance();
    switch (m_gameItemType) {
        case 6:
        case 7:
        case 8:
        case 9: powerUps->EnablePowerUp(k_powerUpOfItem[m_gameItemType - 6], PowerUpTime(m_gameItemType)); break;
        case 4: GS_Gameplay::GetInstance()->BeginLargeMinionGameplay(); break;
        default: break;
    }
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_pickUpsCount, 1);
    Statistics::GetInstance()->SetStatValue(StatisticsSaveData::k_statScope_runDistanceNoPickups, -1);
    SetState(CollectableStateDef::Collected);
}

// @0x567aec: a built item waits; the level builds the next one when the minion is near and has run far enough. The collected base update
// (CollectableItem::Update) runs once the item is built.
void GameItem::Update() {
    if (m_created) {
        CollectableItem::Update();
        return;
    }
    if (m_creationSkipped) return;
    if (!ShouldCreateItem()) return;
    BuildItem();
    const float traveled = (float)GameLevel::GetInstance()->GetMinion()->GetTraveledDistance();
    s_creationSafeDistance = GetNextDistanceSpace() + traveled;
    s_lastSelected = m_gameItemType;
}

// @0x567f40: why the item was not built
const char *GameItem::GetSkippedReasonAsString() const {
    switch (m_skippedReason) {
        case SkipReason::Distance: return "Distance";
        case SkipReason::PowerUp: return "PowerUp";
        case SkipReason::ItemUnavailable: return "ItemUnavailable";
        case SkipReason::Probability: return "Probability";
        case SkipReason::BonusLocked: return "BonusLocked";
        default: return "Unknown";
    }
}

// @0x567fc4: the space to the next item: a random step between the minimum and the maximum space, longer (up to twice) while the minion runs
// faster than the speed the factor starts at
float GameItem::GetNextDistanceSpace() {
    const float minimum = s_minimumItemSpace;
    float space = (float)GameLevel::s_randomizer[3]->Rand(0, (int)(s_maximumItemSpace - minimum)) + minimum;
    if (MinionSpeedMgr *speeds = MinionSpeedMgr::GetInstance()) {
        const float factor = math::clamp(speeds->GetDelayedSpeed() / MinionSpeedMgr::k_minimumSpeedForIncreaseFactor, 1.0f, 2.0f);
        space *= factor;
    }
    return space;
}
