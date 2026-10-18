// source/game/entities/BossObstacleTemplate.h
#ifndef SOURCE_GAME_ENTITIES_BOSSOBSTACLETEMPLATE_H
#define SOURCE_GAME_ENTITIES_BOSSOBSTACLETEMPLATE_H

#include <vector>
#include "ObstacleTemplate.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "core/delayer.h"
#include "Pacesetter.h"

class EffectDef;
class EffectControl;

// The obstacle a boss throws at the minion (the data's "BossObstacleTemplate"; its .so class is 0x324 bytes with the BodyPartOwner base at
// 0x1F8, the ObstacleTemplate part ending there). It flies from the boss's shoot position along the lanes (the spawn offset, the approach timeline
// spawn, pre-warning, warning, attack, vanish), and it is hit by the minion's body parts or stops at the minion's lane (UpdateStop, the Meena and
// Macho missile): a touch makes it fly to the boss (state FlyToBoss, which hurts it by the data's damage) or past it (FlyPastBoss). The offsets in
// the comments are the .so's offsets; the port's BodyPartOwner is 4 bytes larger than the .so's (its scene manager id), so the members after it sit 4 bytes later.
class BossObstacleTemplate : public ObstacleTemplate, public BodyPartOwner {
public:
    // the states the obstacle goes through (SetBossObstacleState, the .so's state at 0x27C): 0 none, 1 spawn, 2 pre-warning, 3 warning, 4 attack,
    // 5 vanishing, 6 flying to the boss, 7 flying past it, 8 stopped at the minion's lane, 9 done
    struct State { enum { Idle = 0, Spawning = 1, PreWarning = 2, Warning = 3, Attack = 4, Vanishing = 5, FlyToBoss = 6, FlyPastBoss = 7, Stopped = 8, Done = 9 }; };
    explicit BossObstacleTemplate(const clara::Template*);
    virtual ~BossObstacleTemplate();
    virtual void PostInit();
    virtual void Update();
    virtual void OnHit(bool);
    virtual void BPO_OnBodyPartPressed(BodyPartSection*);
    virtual void BPO_OnBodyPartTouched(BodyPartSection*);
    virtual void BPO_OnBodyPartDragged(BodyPartSection*);
    virtual void BPO_OnBodyPartReleased(BodyPartSection*);
    // @0x599c5c: whether the closing lane attack (behaviour 5) is on at dt ms into the timeline
    bool EnableClosingLaneAttack(int dt);
private:
    // @0x5a7ea0: the timeline, the state of the approach, and the effects of the state
    void Update(int dt);
    // @0x5a8d90: the state changes: the effects of the states it leaves are released
    void SetBossObstacleState(int state);
    // @0x5a8eb8: the effect is made on the first call (with the obstacle's position and rotation), then it follows the given position
    void UpdateEffect(const EffectDef *def, EffectControl *&ctrl, const math::vec3<float> &pos, float scale, bool setBodyPartModel);
    // @0x5a8604: the flight to the boss (the boss's place plus the curve from the missile's start to its end); it hits the boss when it is there
    void UpdateToBoss(int dt);
    // @0x5a87e0: the missile stopped at the Meena or Macho lane: its parabola, and the hit of the boss when it passes the boss's plane
    void UpdateStop(int dt);
    // @0x5a8a5c: the minion is dead: the obstacle's impact effect is played where the minion's effect is, and the obstacle is done
    void UpdateMinionIsDead();

    int m_spawnTime;                                  // 0x240: the time of the spawn approach (ms)
    int m_preWarningTime;                             // 0x244
    int m_warningTime;                                // 0x248
    int m_startAttackTime;                            // 0x24C
    int m_startVanishTime;                            // 0x254
    int m_lastDt;                                     // 0x258: the time of the last update (the cap of the next one)
    jet::core::Delayer<float, jet::core::LinearStep> m_offsetDelayer;  // 0x25C: the approach of the spawn (origin, current, target)
    float m_missileSpawnPosOffset;                    // 0x274: how far in front of the boss the missile is made
    int m_behavior;                                   // 0x278: the data's "bossObstacleBehavior" (-1 when none)
    int m_state;                                      // 0x27C: State
    bool m_hasBossParams;                             // 0x280: the behaviour takes the boss's damage and the body parts
    EffectDef *m_spawningEffect;                      // 0x284
    EffectDef *m_preWarningEffect;                    // 0x288
    EffectDef *m_warningEffect;                       // 0x28C
    EffectDef *m_attackEffect;                        // 0x290
    EffectDef *m_impactEffect;                        // 0x294
    EffectDef *m_touchedEffect;                       // 0x298
    EffectControl *m_spawningEffectCtrl;              // 0x29C
    EffectControl *m_preWarningEffectCtrl;            // 0x2A0
    EffectControl *m_warningEffectCtrl;               // 0x2A4
    EffectControl *m_attackEffectCtrl;                // 0x2A8
    EffectControl *m_impactEffectCtrl;                // 0x2AC
    float m_bossDistance;                             // 0x2B0: how near the touch must be to the boss to hit it (the flight past it otherwise)
    float m_bossDamage;                               // 0x2B4: the health the boss loses
    float m_despicablePointsOnHit;                    // 0x2B8: the points the minion gets for the hit
    math::vec3<float> m_returnFrom;                   // 0x2BC: where the flight to the boss starts (the touch, from the boss)
    math::vec3<float> m_returnTo;                     // 0x2C8: where the flight ends (the point on the boss's plane, from the boss)
    int m_returnTime;                                 // 0x2D4: the flight's time (ms)
    int m_returnTimeCount;                            // 0x2D8: the time of the flight so far (ms)
    math::vec3<float> m_missileOffset;                // 0x2DC: the missile's offset from the boss
    math::vec3<float> m_missileOffsetStep;            // 0x2E8: how far the missile moves in a frame of its attack
    math::vec3<float> m_missileStopPos;               // 0x2F4: where the missile stopped
    int m_stopParaboleTimer;                          // 0x300: the time the missile has been stopped (ms)
    int m_stopParaboleState;                          // 0x304
    float m_stopLastBossDistance;                     // 0x308: the distance from the boss's plane (negative once the missile is past it)
    bool m_firstUpdate;                               // 0x30C: true until the approach timeline is started
    bool m_active;                                    // 0x30D: false once the obstacle is done with the timeline
    int m_closingLaneStartTime;                       // 0x310
    int m_closingLaneEndTime;                         // 0x314
    EffectDef *m_closingLaneEffect;                   // 0x318
    EffectControl *m_closingLaneEffectCtrl;           // 0x31C
    bool m_canBeReplacedByItem;                       // 0x320
    // the path the boss's pacesetter was found at last (Update's time to reach the plane, the .so's cache at 0xD0)
    std::vector<Pacesetter::PacestterCacheData> m_timeCache;

    static const jet::String k_spawnTime, k_preWarningTime, k_warningTime, k_startAttackTime, k_startVanishTime, k_spawningEffect,
        k_preWarningEffect, k_warningEffect, k_attackEffect, k_impactEffect, k_touchedEffect, k_behavior, k_bossDistance, k_bossDamage,
        k_despicablePointsOnHit, k_closingLaneStartTime, k_closingLaneEndTime, k_closingLaneEffect, k_canBeReplacedByItem, k_bodyParts,
        k_missileSpawnPosOffset;
};

#endif
