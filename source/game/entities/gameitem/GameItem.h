// source/game/entities/gameitem/GameItem.h
#ifndef SOURCE_GAME_ENTITIES_GAMEITEM_GAMEITEM_H
#define SOURCE_GAME_ENTITIES_GAMEITEM_GAMEITEM_H

#include <list>
#include "CollectableItem.h"
#include "GameItemType.h"

class Minion;

// What the level puts on the way for the minion to pick up (the "GameItem" template; 0xEC bytes in the .so): a vehicle (types 3..5), a
// power up (6..9) or a group the level draws from: 0 any item, 1 a vehicle, 2 a power up. The probabilities of the items are the weights of
// their bonuses (BonusSet, Bonus::GetProbabilityWeight) times 2 with a perk; the level builds one when the minion is near enough and it has run
// past the distance of the last item. An item not built (its bonus locked, its probability refused, the power up on, too close to the last) is
// skipped with a reason (GetSkippedReasonAsString). Collecting one turns on its power up or starts its gameplay (Collect).
// Not ported: the bapple replacement (ShouldCreateBapple, TryToReplaceByBapple: the bapple hunt, BappleMgr::TryToCreateBapple, is not in the
// tree) and the debug overlay of the cheats (Render).
class GameItem : public CollectableItem {
public:
    struct SkipReason { enum type { Distance = 0, PowerUp, ItemUnavailable, Probability, BonusLocked, Bapple, Unknown }; };
    struct GameItemProbability { GameItemType type; float probability; };
    explicit GameItem(const clara::Template*);
    virtual ~GameItem();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Collect(Minion*);
    // the type of the item (the items 3..9 are the bonuses: 4 is the large minion)
    GameItemType GetGameItemType() const { return m_gameItemType; }
    // the text of why the item was not built (the cheats show it)
    const char *GetSkippedReasonAsString() const;
    // @0x537228 (GameLevel::PostInit): the distance the next item may be built at starts at the initial space
    static void ResetLastCreatedDistance();
    static void SetMinimumItemSpace(float v) { s_minimumItemSpace = v; }
    static void SetMaximumItemSpace(float v) { s_maximumItemSpace = v; }
    static void SetInitialSpace(float v) { s_initialSpace = v; }
    static void SetTotalProbabilityValue(float v) { s_probabilityValue = v; }
    static jet::String k_tmplName;
private:
    bool IsBonusLocked() const;
    void SkipCreation(int reason) { m_creationSkipped = true; m_skippedReason = reason; }
    void BuildItem();
    void SetNewData();
    bool ShouldCreateItem();
    void CalculateItemsProbability();
    void CalculateVehiclesProbability();
    void CalculatePowerUpsProbability();
    bool AreAllItemsLocked() const;
    bool VerifyGeneralProbability() const;
    GameItemType GetRandomItem();
    GameItemType GetRandomVehicle();
    GameItemType GetRandomPowerUp();
    GameItemType GetForcedItem(const GameItemType&) const;
    bool FindGameItemProbability(const GameItemProbability *arr, int count, GameItemType type, int &index) const;
    void RemoveProbability(GameItemProbability *arr, int count, GameItemType type, std::list<GameItemType> &types) const;
    void ScaleProbabilities(GameItemProbability *arr, int count, const std::list<GameItemType> &types) const;
    float GetNextDistanceSpace();

    GameItemType m_gameItemType = 0;        // 0xCC
    bool m_created = false;                 // 0xD1: the item was built
    bool m_creationSkipped = false;         // 0xD2: the item is not built
    int m_skippedReason = SkipReason::Unknown;  // 0xD4

    static const jet::String k_gameItemTypeParam;
    static int s_lastSelected;
    static float s_creationSafeDistance;
    static float s_minimumItemSpace;
    static float s_maximumItemSpace;
    static float s_initialSpace;
    static float s_probabilityValue;
    static float s_itemCreationProximityThresholdSq;
    static GameItemProbability s_allGameItems[7];
    static GameItemProbability s_powerUps[4];
    static GameItemProbability s_vehicles[3];
};

#endif
