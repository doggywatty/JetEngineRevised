// source/game/entities/ObstacleTemplate.h
#ifndef SOURCE_GAME_ENTITIES_OBSTACLETEMPLATE_H
#define SOURCE_GAME_ENTITIES_OBSTACLETEMPLATE_H

#include <map>
#include <vector>
#include "Object.h"
#include "SynchronizedObject.h"
#include "game/gameenums.h"
#include "game/leveltemplate/gameplaytemplateinstancelistener.h"

class GameplayTemplateInstance;
class Pacesetter;
class ObstacleTemplate;
class EffectControl;

// What an obstacle can hit besides the minion (the runners): it follows a pacesetter and is told when an obstacle was met
class ObstacleTarget {
public:
    ObstacleTarget() : m_targetPacesetter(0) {}
    virtual ~ObstacleTarget();
    virtual void OnObstacleHit(ObstacleTemplate*) = 0;
    Pacesetter *GetTargetPacesetter() const { return m_targetPacesetter; }
protected:
    Pacesetter *m_targetPacesetter;  // +0x4
};

// What the minion runs into (the data's "ObstacleTemplate", type 0x16; 0x22C bytes in the .so): an object, the SynchronizedObject at 0xC8
// and the listener of its level template at 0x150. It holds the lanes its collision covers along the template, how near the minion must
// pass (the trigger distance) and the states it takes: 0 waiting, 1 weapon power up, 2 destroyed, 3 destroyed by the freeze ray, 4 gone.
// The minion's meeting with it (Update's branch: Minion::CheckSuccessStateTypes, OnObstacleHitted, OnObstacleNearMiss, OnObstaclePassed) and the
// weapon power up (UpdateWeaponPowerUp, with the freezing deco) are ported; Collision3d joins with the physics.
class ObstacleTemplate : public Object, public SynchronizedObject, public GameplayTemplateInstanceListener {
public:
    struct ObstacleState { enum { Idle = 0, WeaponPowerUp = 1, Destroyed = 2, DestroyedFrozen = 3, Gone = 4 }; };
    // what CheckTrigger finds: nothing, the pacesetter reaches it on its rail, passes it near, or it is in reach on another rail
    struct CheckTriggerResult { enum { None = 0, Hit = 1, NearMiss = 2, OffRail = 3 }; };
    explicit ObstacleTemplate(const clara::Template*);
    virtual ~ObstacleTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void SetSpawnLane(int);
    virtual void OnHit(bool);
    // @0x581c1c: the target (the minion, or a runner) is put on the trigger plane of the obstacle when it is in front of it
    void PlaceTargetOnTriggerPos(GameEntity *target);
    // whether the minion can hit it (the flag the ctor sets, 0x155 in the .so)
    bool IsHittable() const { return m_hittable; }
    // the data's obstacle id ("fireLips", "sandCastle", ...), which the statistics count the hits by
    const jet::String &GetObstacleId() const { return m_obstacleId; }
    // the kinds of obstacle the statistics count (@0x578f70, @0x578d9c, @0x57a0bc)
    static const jet::String k_typeFireLips, k_typeEvilCage, k_typeSandCastle, k_typeFireBowl, k_typeBarbecue, k_typeHammock, k_typeRedTapes,
        k_typeWetfloorSigns, k_typeSurfBoard, k_typeAdvertisingPanels;
    // the distance (meters) the freeze ray reaches an obstacle from (@0xcff298)
    static const float k_weaponPowerUp_distance;
    virtual void OnMinionRevive();
    virtual void ConstructFrom(const GameEntity*);
    using GameEntity::ConstructFrom;
    virtual void OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent&);
    // the pacesetter's template is gone
    void OnGameplayTemplateInstanceDestroyed();
    void Destroy(bool freezeDeco, bool frozen);
    unsigned GetCollisionTiles() const { return m_collisionTiles; }
    void SetLane(LaneIndex l) { m_lane = l; }
    LaneIndex GetLane() const { return m_lane; }
    bool IsOnLane(LaneIndex l) const { return m_collisionLaneStart <= l && l <= m_collisionLaneEnd; }
    // the template the obstacle was made in: the lanes it covers are blocked along it, and it hears when the template goes
    void SetGameplayTemplateInstance(GameplayTemplateInstance*);
    void SetCollisionStart(unsigned node) { m_collisionStart = node; }
    // blocks (or frees) the nodes of the lanes its collision covers on the template
    void EnableTileCollisions(bool);
    // the lanes its collision covers: its own, and the ones beside it that it is used in
    void InitCollisionLanes();
    bool UseLocalization() const { return m_useLocalization; }
    ObstacleTemplate *GetLocalizedObstacleTemplate(const jet::String &name);
    int GetEngineEventOnCrash() const { return m_engineEventOnCrash; }
    int GetEngineEventOnNearMiss() const { return m_engineEventNearMiss; }
    GameEntity *GetLinkedInteractiveTemplate() const { return m_linkedInteractiveTemplate; }
    void SetLinkedInteractiveInstance(GameEntity *e) { m_linkedInteractiveInstance = e; }
    const std::vector<int> &GetSuccessStateTypesOnGround() const { return m_successStateTypeOnGround; }
    const std::vector<int> &GetSuccessStateTypesOnPlatform() const { return m_successStateTypeOnPlatform; }
    bool IsNearMissPending() const { return m_nearMiss; }
    float GetTriggerDistance() const { return m_triggerDistance; }
    // what can be hit besides the minion
    static void AddTarget(ObstacleTarget*);
    static void RemoveTarget(ObstacleTarget*);
    static jet::String k_tmplName;
    static const jet::String k_enableStateCheck, k_useInRight, k_useInLeft, k_triggerDistance, k_nearMissDistance, k_successStateType,
        k_successStateTypeOnPlatform, k_collisionTiles, k_effectOnDestroyed, k_nodeRefOnDestroyed, k_effectOnIdle, k_nodeRefOnIdle,
        k_engineEventOnCrash, k_useLocalization, k_localizationArray, k_engineEventNearMiss, k_effectOnCrash, k_animOnCrash, k_hideOnCrash,
        k_uselinkedInteractive, k_linkedInteractive, k_freezeObjectDeco, k_freezeEffectOnDestroyed, k_freezeEffect, k_param_obstacleId;
    // @0x5806a0: the obstacle leaves the world (the boss gameplay takes it out): its idle effect is released
    void OnWorldChange();
protected:
    // @0x5806ac: the idle effect follows the obstacle's node while it is near the camera and in front of it
    void UpdateEffectCtrl(EffectControl **ctrl, const EffectDef *def, const jet::String &nodeRef);
    // @0x58091c: the effect is released (or stopped when release is false) and the control forgotten
    void ReleaseEffectCtrl(EffectControl **ctrl, bool release);
    // @0x581010: the destruction effects of a destroyed obstacle, then its deco is hidden and it is gone
    void UpdateDestroyedEffects();
    // @0x581814: whether the pacesetter is on a rail the obstacle is used on (or beside)
    bool IsTargetOnRail(Pacesetter*);
    // @0x58198c
    int CheckTrigger(Pacesetter*);
    // @0x580adc, 0x581758
    void UpdateTargetsTrigger();
    // @0x580ed4: an obstacle that waits starts its weapon power up when the freeze ray is on and the minion is in reach, or meets the minion
    void UpdateIdle();
    // @0x5809f4: the freeze ray starts on the obstacle when it is on the minion's rail and within k_weaponPowerUp_distance of the minion
    bool MustStartWeaponPowerUp();
    // @0x580adc: one of the lanes of the obstacle is frozen as far as the obstacle is in the run (the minion's freeze distances)
    bool IsFreezingDistanceOk();
    // @0x580b40: the weapon power up: the freezing deco (the copy of the freeze deco) takes the place of the obstacle's while the freeze reaches it,
    // then the obstacle is destroyed (or the minion passes it when it is on its rail and the success states allow it)
    void UpdateWeaponPowerUp();
    void ExplodeLinkedInteractiveObject();
    void LoadArray(const jet::String &param, std::vector<int> &out);
    bool m_triggerOnRail;                                      // 0x154: the pacesetter's rail alone decides
    bool m_hittable = true;                                    // 0x155: the minion's hits count (set by the ctor, never cleared in the .so)
    float m_triggerDistance;                                   // 0x158
    float m_nearMissDistance;                                  // 0x15C
    std::vector<int> m_successStateTypeOnGround;               // 0x160
    std::vector<int> m_successStateTypeOnPlatform;             // 0x16C
    unsigned m_collisionStart;                                 // 0x178: the node of the template it stands at
    unsigned m_collisionTiles;                                 // 0x17C
    GameplayTemplateInstance *m_gameplayTemplateInstance;      // 0x180
    clara::DataEntity *m_effectOnDestroyed;                    // 0x184 (an EffectDef)
    jet::String m_nodeRefOnDestroyed;                          // 0x188
    clara::DataEntity *m_effectOnIdle;                         // 0x18C
    jet::String m_nodeRefOnIdle;                               // 0x190
    EffectControl *m_effectOnIdleCtrl = 0;                     // 0x194: the idle effect while it plays
    int m_engineEventOnCrash, m_engineEventNearMiss;           // 0x198, 0x19C
    bool m_useInRight, m_useInLeft, m_nearMiss;                // 0x1A0, 0x1A1, 0x1A2
    int m_state;                                               // 0x1A4
    LaneIndex m_lane;                                          // 0x1A8
    int m_collisionLaneStart, m_collisionLaneEnd;              // 0x1AC, 0x1B0
    bool m_freezeDecoChanged;                                  // 0x1B4
    clara::DataEntity *m_freezeDeco;                           // 0x1B8
    clara::DataEntity *m_freezeEffectOnDestroyed, *m_freezeEffect;  // 0x1BC, 0x1C0
    clara::DataEntity *m_effectOnCrash;                        // 0x1C4
    jet::String m_animOnCrash, m_idleAnim;                     // 0x1C8, 0x1CC
    bool m_hideOnCrash, m_useLocalization;                     // 0x1D0, 0x1D1
    std::map<jet::String, ObstacleTemplate*> m_localizationPairs;  // 0x1D4
    GameEntity *m_linkedInteractiveTemplate, *m_linkedInteractiveInstance;  // 0x1EC, 0x1F0
    jet::String m_obstacleId;                                  // 0x1F4
    static std::vector<ObstacleTarget*> s_obstacleTargets;
};

#endif
