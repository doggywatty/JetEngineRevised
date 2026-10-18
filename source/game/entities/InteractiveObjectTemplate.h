// source/game/entities/InteractiveObjectTemplate.h
#ifndef SOURCE_GAME_ENTITIES_INTERACTIVEOBJECTTEMPLATE_H
#define SOURCE_GAME_ENTITIES_INTERACTIVEOBJECTTEMPLATE_H

#include <vector>
#include "Object.h"
#include "SynchronizedObject.h"
#include "core/delayer.h"
#include "game/actors/statemachine/StateMachine.h"

class EffectDef;
class EffectControl;
class MaterialDef;

// What the minion meets on its way that reacts to it (doors that open, things that fall, a pusher): an object (a deco, its collision) with an
// animation or a state of its own for the contact, the despicable points it gives, effects when the minion comes near, a post effect that
// distorts the screen and a sound that plays from afar. The data's "InteractiveObjectTemplate" (type 0xF; 0x2A8 bytes in the .so): Object,
// the StateMachine at 0xC8, the SynchronizedObject at 0x158 and its own members from 0x1E0. A copy registers with the level
// (GameLevel::AddInteractiveObject) and leaves it again when it goes.
// The effects of coming near and of the freeze are played (EffectMgr). Not ported yet (no Collision3d, nor the minion's run states, nor
// Minion::IncreaseDespicablePoints in the tree): the effects of the contact and of the pre-contact animations (their EffectControls are
// kept, none is started), the post effect's uniforms and what the minion is told (its points and engine events).
class InteractiveObjectTemplate : public Object, public StateMachine, public SynchronizedObject {
public:
    // where the pre contact animations are: none, waiting for the minion to come near, counting down to the projectile's effect, the
    // projectile on its way, done (+0x1F8)
    struct PreContactAnimState { enum { None = 0, WaitMinion = 1, WaitEffect = 2, Projectile = 3, Done = 4 }; };
    // the post effect: none, getting in, holding, getting out, gone (+0x274)
    struct PostFxAnimState { enum { None = 0, Intro = 1, Hold = 2, Outro = 3, Removed = 4 }; };

    explicit InteractiveObjectTemplate(const clara::Template*);
    virtual ~InteractiveObjectTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void SetSpawnLane(int);
    virtual void OnMinionRevive();
    virtual void OnSyncFinished();
    virtual unsigned SM_GetId();
    // @0x56da0c InteractiveObjectTemplate::CollidesWith(Collision3d*, const CollisionPair&), once the collision has given the entity that touched
    // the object (the listener of the other Collision3d, +0xAC): Collision3d is not in the tree
    void CollidesWith(GameEntity *other);
    void OnFreeze();
    void OnGamePlayChanged();
    float GetDespicablePoints() const { return m_despicablePoints; }
    float GetDespicablePointsOnDistanceTrigger() const { return m_despicablePointsOnDistanceTrigger; }
    bool GetForzeSpawnIt() const { return m_forzeSpawnIt; }
    static jet::String k_tmplName;
    static jet::String k_superflousInteractive_tmplName;
private:
    void InitStateMachine();
    void InitPreContactAnims();
    void SetPostFxState(unsigned);
    void UpdatePostFxState(unsigned dt);
    void UpateDistanceTriggers();
    void UpdatePreContactAnims(unsigned dt);
    void StopIdleSound();
    void ReleaseEffects();

    jet::String m_animOnContact;               // 0x1E0
    bool m_useEngineEvent;                     // 0x1E4
    int m_engineEvent;                         // 0x1E8
    float m_despicablePoints;                  // 0x1EC
    bool m_checkLaneOnContact;                 // 0x1F0
    bool m_hideOnContact;                      // 0x1F1
    bool m_useStateMachine;                    // 0x1F2
    jet::String m_stateOnContact;              // 0x1F4
    unsigned m_preContactState;                // 0x1F8
    jet::String m_stateOnPreContact;           // 0x1FC
    int m_preContactTime;                      // 0x200
    EffectDef *m_preContactProjectileEffect;   // 0x204
    EffectDef *m_preContactImpactEffect;       // 0x208
    EffectDef *m_preContactObstacleEffect;     // 0x20C
    EffectControl *m_effectControls[3];        // 0x210: what is playing (the obstacle effect's at 0x218)
    int m_preContactEffectTime;                // 0x21C
    int m_preContactProjectileTime;            // 0x220
    int m_preContactProjectileTimeCount;       // 0x224
    EffectControl *m_projectileEffectControl;  // 0x228
    bool m_useDistanceTrigger;                 // 0x22C
    float m_distanceTriggerSq;                 // 0x230
    std::vector<EffectDef*> m_effectOnDistanceTrigger;  // 0x234
    bool m_hideOnDistanceTrigger;              // 0x240
    float m_despicablePointsOnDistanceTrigger; // 0x244
    MaterialDef *m_postFxMaterial;             // 0x248
    int m_postFxTimeIntro;                     // 0x24C
    int m_postFxTime;                          // 0x250
    int m_postFxTimeOutro;                     // 0x254
    float m_postFxMaxValue;                    // 0x258
    jet::core::Delayer<float, jet::core::QuadraticEaseIn> m_postFxUniformValue;  // 0x25C
    unsigned m_postFxState;                    // 0x274
    bool m_playIdleSound;                      // 0x27C
    float m_playIdleSoundDistanceSq;           // 0x280
    jet::String m_idleSoundLabel;              // 0x284
    int m_idleSoundHandle;                     // 0x288
    std::vector<jet::String> m_animOnFreeze;   // 0x28C
    std::vector<const EffectDef*> m_effectOnFreeze;  // 0x298
    bool m_hideOnFreeze;                       // 0x2A4
    bool m_frozen;                             // 0x2A5
    bool m_forzeSpawnIt;                       // 0x2A6
    static const jet::String k_animOnContact, k_useEngineEventOnContact, k_engineEventOnContact, k_despicablePoints, k_checkLaneOnContact,
        k_stateOnContact, k_hideOnContact, k_forceSpawnIt, k_enablePreContactAnimations, k_stateOnPreContact, k_preContactTime,
        k_preContactEffectTime, k_preContactProjectileTime, k_preContactProjectileEffect, k_PreContactImpactEffect,
        k_preContactObstacleEffect, k_animOnFreeze, k_effectOnFreeze, k_hideOnFreeze, k_useDistanceTrigger, k_distanceTrigger,
        k_effectOnDistanceTrigger, k_hideOnDistanceTrigger, k_despicablePointsOnDistanceTrigger, k_usePostFxDistortion, k_postFxMaterial,
        k_postFxMaterialLow, k_postFxTimeIntro, k_postFxTime, k_postFxTimeOutro, k_postFxUniformMaxValue, k_playIdleSoundParam,
        k_playIdleSoundDistanceParam, k_idleSoundLabelParam;
};

#endif
