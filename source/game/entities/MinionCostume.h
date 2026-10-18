// source/game/entities/MinionCostume.h
#ifndef SOURCE_GAME_ENTITIES_MINIONCOSTUME_H
#define SOURCE_GAME_ENTITIES_MINIONCOSTUME_H

#include <vector>
#include "Actor.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "gameitem/GameItemType.h"

class Price;
class BonusForCostumes;
class Deco3d;

// A costume of the minion ("MinionCostume" template, 0x25C bytes in the .so). The ones in the CostumeMgr's
// "MinionCostumes" folder are library entities (data only: id, name, price, bonuses); the menus show copies, which get
// the body parts to touch, a blob shadow and walk on their own to the place they are given (UpdateReachTarget).
class MinionCostume : public Actor, public BodyPartOwner {
public:
    explicit MinionCostume(const clara::Template*);
    MinionCostume(MinionCostume *original, unsigned ctorFlags);
    virtual ~MinionCostume();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual math::vec3<float> SM_GetScale() { return GetScale(); }  // @0x561788
    virtual void BPO_OnBodyPartTouched(BodyPartSection*);
    virtual void BPO_OnBodyPartDragged(BodyPartSection*) {}  // @0x57c120
    virtual void BPO_OnBodyPartPressed(BodyPartSection*) {}  // @0x57c12c
    virtual void BPO_OnBodyPartReleased(BodyPartSection*) {}  // @0x57c138
    virtual GameEntity *BPO_GetOwnerGameEntity() { return this; }  // @0x56177c

    static void SetIsDragging(bool d) { s_isDragging = d; }
    static void SetCurrentBigCostumePosWorldY(float y) { s_currentBigCostumePosWorldY = y; }
    const jet::String &GetClaraEntityName() const { return m_claraEntityName; }
    int GetCostumeId() const { return m_id; }
    const jet::String &GetGameName() const;
    const jet::String &GetBonusText();
    const jet::String &GetBuyType() const { return m_buyType; }
    Price *GetPrice() const { return m_price; }
    bool HasPromo();
    Deco3d *GetShadowDeco() const { return m_shadowDeco; }
    int GetOrderValue() const { return m_orderValue; }
    bool IsShopElement() const { return m_isShopElement; }
    bool HasBonus() const { return m_hasBonus; }
    unsigned GetNumBonuses() const { return (unsigned)m_bonuses.size(); }
    BonusForCostumes *GetBonus(int i) const { return m_bonuses[(unsigned)i]; }
    void SetEnableReachTarget(bool e) { m_enableReachTarget = e; }
    void SetTargetPosition(const math::vec3<float> &p) { m_targetPos = p; }
    void SetMoveSpeed(float s) { m_moveSpeed = s; }
    const math::vec3<float> &GetTargetPosition() const { return m_targetPos; }
    void SetEffectLayerId(int id);
    void SetTracksEnabled(bool sounds, bool effects) { StateMachine::SetTracksEnabled(sounds, effects); }
    void SM_StopAllSounds() { StateMachine::SM_StopAllSounds(); }
    bool ShouldBeShownInShop();
    bool IsNearTheCurrentPos();
    bool HasReachedTargetPos();
    bool IsCurrentlyEquipped();
    bool IsTimeLimited();
    void ReleaseEffects();
    void SetPlaceHolderDecoIfNeeded();
    // the deco the minion (or a menu) copies to wear this costume
    Deco3d *GetCostumeDecoReference() const { return GetDecoReference(0, false); }
    // the bonuses
    float GetFriendChallengeEasierFactor() const;
    int GetExtraBananasOnFriendsChallenge() const;
    float GetExtraBananasFactor() const;
    int GetExtraTimeForPowerUp(GameItemType) const;
    int RevivesForFree() const;
    bool FreezeRaySmashesObstacleApart() const;
    int GetExtraDespicablePointsFactorWhenRunning() const;
    int GetExtraDespicablePointsFactorForGlobal() const;
    int GetDespicableActionFactor() const;
    int GetExtraWeightFactorForPowerUp(GameItemType) const;
    // the puzzle a "puzzle" costume is bought with
    int GetPuzzlePiecesCount() const;
    const jet::String &GetPuzzleSpriteName() const;
    const jet::String &GetPuzzleBGImageSpriteName() const;
    int GetPuzzleBGImageSpriteFrameId() const;
    int GetPuzzleBGImageSpriteAnimId() const;

    static jet::String k_tmplName;
    static const jet::String k_buyType_buyable, k_buyType_puzzle, k_headNodeName;
    static const float k_defaultMoveSpeed, k_baseAngle, k_runAngleDiff;
    static const int k_maxTimeRunningInPlace;
private:
    void UpdateReachTarget(unsigned dt);
    std::vector<BodyPartDefinition> m_bodyPartDefinitions;  // 0x1F8
    jet::String m_claraEntityName;  // 0x214: the library entity this costume is (copies keep it)
    int m_id;                       // 0x218
    jet::String m_name;             // 0x21C
    jet::String m_bonusText;        // 0x220
    jet::String m_buyType;          // 0x224
    Price *m_price;                 // 0x228
    Deco3d *m_shadowDeco;           // 0x22C
    int m_orderValue;               // 0x230
    bool m_isShopElement;           // 0x234
    bool m_enableReachTarget;       // 0x235
    bool m_hasBonus;                // 0x236
    std::vector<BonusForCostumes*> m_bonuses;  // 0x238
    math::vec3<float> m_targetPos;  // 0x248
    int m_timeRunning;              // 0x254
    float m_moveSpeed;              // 0x258
    static bool s_isDragging;
    static float s_currentBigCostumePosWorldY;
    static const jet::String k_costumeId, k_name, k_bonusText, k_price, k_orderValue, k_hasBonus, k_bonuses, k_isShopElement,
        k_shadowDeco, k_buyType, k_PuzzleBGImage, k_PuzzlePiecesCount, k_PuzzlePiecesSprite;
};

#endif
