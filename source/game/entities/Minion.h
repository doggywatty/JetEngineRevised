// source/game/entities/Minion.h
#ifndef SOURCE_GAME_ENTITIES_MINION_H
#define SOURCE_GAME_ENTITIES_MINION_H

#include "Actor.h"
#include "pacesetterfollower.h"
#include "camera/CameraTarget.h"
#include "core/delayer.h"
#include "../gameenums.h"

class Deco3d;
class MinionCostume;
class ObstacleTemplate;
class JumpDefinition;
class EffectControl;
class EffectDef;

// The player minion ("Minion" template): an actor driven by the state machine of the "states" database (set "minion" for the run,
// "minion_menu" for the menus) that follows the pacesetter along the level templates (it is a PacesetterFollower: the pacesetter tells it where
// it is, the minion eases to it and falls on the ground the physics world has below it). Swipes reach it as screen events
// (SM_OnScreenEvent: dodge left and right) and engine events (9 jump, 0x25 slide); the state it enters decides what it does (SM_OnChangeState):
// the lane change, the jump along a jump definition or under gravity, the slide, the crash, the death.
// 0x520 bytes in the .so. Not ported (they need systems that are not ported): the body parts touched on the minion, the accelerometer and
// the large minion, rocket, fluffy, sledge, mount, cannon and fast forward modes (their flags exist and are never set), the power up effects,
// the freezing projectiles, the despicable points and the camera animation of a state.
class Minion : public Actor, public PacesetterFollower, public CameraTarget {
public:
    // the values of the physics the data tunes (GameplayDef): the order is the one of the original's ChangeablePhysicsDef
    enum ChangeablePhysicsDef { Physics_Acceleration = 0, Physics_JumpImpulse, Physics_JumpFrontDistance, Physics_JumpUpDistance, Physics_DashDownGravity,
                                Physics_SlideDistance, Physics_ChangeLaneTime, Physics_ChangeLaneMaxSpeed, Physics_SledgeGravityMultiplier, Physics_FFWFlyingAltitude,
                                Physics_Count };
    explicit Minion(const clara::Template*);
    virtual ~Minion();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void InitStateMachine();
    virtual bool SM_OnScreenEvent(int code, int param, bool pressed);
    // @0x575220: the minion's contact with a collectable (type 4) or a destroyable obstacle (type 0x16)
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual void SetCurrentJumpDefinition(JumpDefinition*);
    virtual void StartJumpDefinition(JumpDefinition*, const math::vec3<float> &pos, const math::quat<float> &rot);
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&);
    virtual void NotifyInBonusGameplay(bool) {}
    // CameraTarget: the camera looks at where the minion is and the way it runs (@0x55bd48, @0x57aafc), and is placed from where it is (@0x55bcd4, @0x55bd14)
    virtual math::vec3<float> GetTargetPosition() const { return GetPosition(); }
    virtual math::quat<float> GetTargetRotation() const;
    virtual math::vec3<float> GetFocusPosition() const { return GetPosition(); }
    virtual math::quat<float> GetFocusRotation() const { return GetRotation(); }
    // how far the minion leans in a turn (-1..1), and the flags the camera reads
    float GetLean() const { return m_lean; }
    bool IsFallingInPit() const { return m_falling; }
    // the camera shakes once after a crash: the flag is read and cleared
    bool TakeAfterCrashFlag() { const bool f = m_afterCrashFlag; m_afterCrashFlag = false; return f; }

    // the menus: the minion plays a state of a set of the database
    bool SetStateMachine(const jet::String &set, const jet::String &state);
    // @0x57b26c: wears a costume: a copy of its deco takes the place of the minion's. The costume is remembered when it is one of the shop's.
    void SetCostume(Deco3d *costumeDeco, MinionCostume *costume);
    MinionCostume *GetCostume() const { return m_costume; }
    // @0x57b698: the minion wears the costume the manager says is equipped, when it does not
    void ReloadCostumeIfNeeded();
    // where the level placed it (taken in PostInit): the menus put their costumes there
    const math::vec3<float> &GetInitialPosition() const { return m_initialPosition; }

    // @0x5776d4: the input of the frame (the swipes) is read and sent to the state machine
    void UpdateInput();
    // @0x579b30, @0x579bf0: whether a lane is free to the side to dodge to
    bool CanDodgeLeft();
    bool CanDodgeRight();
    // the physics values the data tunes (@0x57966c, @0x5796f0)
    void ChangePhysicsValue(int which, float v);
    float GetPhysicsValue(int which) const;
    void SetPostShieldInvulnerabilityTime(int ms) { m_postShieldInvulnerabilityTime = ms; }      // 0x448
    void SetPostFreezeGunInvulnerabilityTime(int ms) { m_postFreezeGunInvulnerabilityTime = ms; }  // 0x44C
    void SetPostSledgeInvulnerabilityTime(int ms) { m_postSledgeInvulnerabilityTime = ms; }       // 0x444
    // @0x57b4c8: the speed evolution definition the speed manager reads the intervals of
    clara::DataEntity *GetSpeedDefinition() const;
    // the power ups start and stop (@0x57b07c, @0x57b188): the engine events the state machine takes
    void OnPowerUpEnabled(EPowerUp);
    void OnPowerUpDisabled(EPowerUp, bool notify);
    // the flags the level's sledge change reads (GameLevel::CheckMinionStateChange, @0x53bc9c)
    bool IsSledgeMode() const { return m_sledgeMode; }
    bool IsRiding() const { return m_riding; }
    bool IsLargeMinion() const { return m_largeMinion; }
    void SetBlendDirections(bool b) { m_blendDirections = b; }  // 0x2B6: the direction blend the large minion's mode turns off
    // @0x57a764: the sledge begins: the power ups go off but the banana multiplier, and the minion goes into the sledge state (gravity or not)
    void BeginSledge();
    // @0x57a7b8: the sledge ends: the minion is sent the engine event 0x2e
    void EndSledge();
    // @0x57abf0: a pusher (InteractiveAnimatedPusher) takes the minion: the state of the pusher plays, the pacesetter pauses, the minion is linked
    // to the pusher's deco and runs at the animation speed the pacesetter's speed gives between the two speed pairs (x: speed, y: animation speed)
    void OnInteractiveAnimatedPusherStart(Object *pusher, const jet::String &state, const math::vec2<float> &minSpeed, const math::vec2<float> &maxSpeed,
                                          clara::Entity *exitPoint);
    // @0x57ada4: the pusher lets the minion go at the pacesetter's lane: along its jump, or pushed up by the upward speed, at the front speed for the restore time
    void OnInteractiveAnimatedPusherStop(float upSpeed, float frontSpeed, int restoreTime, JumpDefinition *jump, clara::Entity *exitPoint);
    // @0x55be48: whether the minion takes the jumpers it runs onto (not in the rocket, riding, frozen, the cannon, falling out, nor in the
    // flag 0x391 the large minion sets)
    bool InteractWithJumpers() const;
    // @0x579d8c: whether the minion takes a speeder (not a rocket, engaging or riding a ride, falling out, nor in the large minion's transition)
    bool UseSpeeder() const;
    // @0x579cb0: the minion is pushed up: the engine event (0x3c when it is -1), the upward speed as a gravity, and the pacesetter goes at the front
    // speed for the restore time back to its speed when that is slower than the front speed. Returns false when the engine event is refused.
    bool PushUp(float upSpeed, float frontSpeed, int engineEvent, int restoreTime);
    // @0x579dd0: the minion is pushed forward a distance in a time (the engine event 0x3d when sent): the pacesetter goes at the speed that covers
    // the distance in the time, then back to its speed over the time. Returns false without a pacesetter.
    bool PushFront(float distance, float time, bool sendEngineEvent);
    // @0x57a398: the minion gets despicable points where the points happen: the bar fills, each level of the despicable multiplier it fills is
    // counted in the statistics, and the points go on the score (the despicable multiplier and the minion's costume scale them)
    void IncreaseDespicablePoints(float points, const math::vec3<float> &where);
    // @0x57b73c: the bar's fill, 0..1 (its points over the points of the level)
    float GetDespicablePointsPgt();
    // @0x579e78: the obstacle lets the minion pass when it is in the air, its pacesetter is paused, or it is in one of the states of the list
    bool CheckSuccessStateTypes(const std::vector<int> &types) const;
    // @0x579ee0: the minion meets an obstacle it can hit (not while it is invincible, nor with the shield, which destroys the obstacle instead):
    // the hit is counted, the obstacle's crash engine event goes to the minion and the effects of the hits are stopped. Returns whether the
    // obstacle crashes the minion.
    bool OnObstacleHitted(ObstacleTemplate *o);
    // @0x57a0bc: the minion is near an obstacle on its lane: the near miss counts (its statistics and score), unless it is invincible or has
    // the freeze ray. Returns whether it counted.
    bool OnObstacleNearMiss(ObstacleTemplate *o);
    // @0x57a290: the minion passes an obstacle on its lane: the one it passed before is counted, and a jump or a slide keeps this one for the
    // jump over and roll under counts for a while
    void OnObstaclePassed(ObstacleTemplate *o);
    // @0x55bdec: whether the obstacles can hit the minion (not in the rocket, a ride it engages or rides, frozen, nor falling out)
    bool HitObstacles() const;
    // @0x578f70: the obstacle of this type that the minion hit is counted (the fire lips, the sand castles, and the fire bowls and barbecues
    // while it jumps)
    void CountObtacleHit(const jet::String &type) const;
    // the freeze ray's bullet effect (GameplayDef's freezingGunBulletEffect param, @0x568a40 stores it here)
    void SetFreezingBulletEffect(const EffectDef *e) { m_freezingBulletEffect = e; }
    // the points a frozen obstacle gives the minion (@0x568a40 stores the data's DespicablePointsOnFreezeObjects here, at 0x2C4)
    void SetFreezeDespicablePoints(float p) { m_freezeDespicablePoints = p; }
    float GetFreezeDespicablePoints() const { return m_freezeDespicablePoints; }
    // the freezing projectiles: one every k_freezingProjectile_Ratio ms while the freeze ray is on, each flying k_freezingProjectile_Time ms
    static const int k_freezingProjectile_Ratio = 300, k_freezingProjectile_Time = 1800, k_freezingProjectile_Max = 15;  // @0xc8b8c8, @0xc8b8cc, @0xc8b8d0
    static constexpr float k_freezingProjectile_distance = 20.0f;  // @0xc8b8c4
    // the freeze distance on a lane: the distance in the run up to which the lane's obstacles are frozen (UpdateFreezingProjectiles)
    float GetFreezeDistance(int lane) const { return m_freezeDistances[lane]; }
    // @0x46C: the seven effect slots (4 bytes each) the minion keeps for the obstacles it hits; OnObstacleHitted stops and clears them
    EffectControl *&GetObstacleEffectSlot(int i) { return m_obstacleEffects[i]; }
    // @0x57b638: the minion cannot be hurt for a time
    void SetInvulnerable(int ms);
    // @0x57b578: the minion runs again after a death: the speed goes back to the interval of the revive
    void Revive();
    // @0x57769c: the minion landed
    void OnGroundHitted();
    // @0x57a9f4
    void HandleHoldInput() {}

    // what the minion is doing (the type of the state it is in)
    bool IsIdle() const;       // @0x577064
    bool IsJumping() const;    // @0x577088
    bool IsRunning() const;    // @0x5770b0
    bool IsSliding() const;    // @0x577104
    bool IsLanding() const;    // @0x577200
    bool IsCrashed() const;    // @0x577220
    bool IsDead() const;       // @0x577230
    bool IsDying() const;      // @0x577284
    bool IsFrozenRun() const { return m_frozenRun; }
    bool IsFallingOut() const { return m_fallOutFlag; }  // 0x390
    bool IsCannon() const { return m_cannon; }
    bool CanFallOut() const;   // @0x57a708

    // how far the minion has run: the part the level sequence advanced and the part it ran itself
    unsigned GetTraveledDistance() const { return m_distanceBase + m_distanceRun; }
    unsigned GetDistanceBase() const { return m_distanceBase; }
    void SetDistanceBase(unsigned d) { m_distanceBase = d; }
    unsigned GetDistanceRun() const { return m_distanceRun; }
    void SetDistanceRun(unsigned d) { m_distanceRun = d; }
    void SetLocationEnterDistance(unsigned d) { m_locationEnterDistance = d; }
    float GetRunDistance() const { return m_runDistance; }

    // what the run of the level templates needs of the minion: the pacesetter it follows, whether it stands on the ground, whether it is
    // invincible (during the fast forward), +0x265 (set when it dies) and where the bananas are collected
    Pacesetter *GetPacesetter() const { return PacesetterFollower::GetPacesetter(); }
    void SetPacesetter(Pacesetter *p) { PacesetterFollower::SetPacesetter(p); }
    bool IsOnGround() const { return m_onGround; }
    bool IsInvincible() const { return m_fastForward; }
    bool IsInFastForward() const { return m_fastForward; }
    bool GetStateFlag265() const { return m_deadFlag; }
    const math::vec3<float> &GetBananaPosition() const { return m_bananaPosition; }
    // @0x57b50c: the lane the minion takes at the end of a conditional bifurcation
    void OnBifurcationResolved(LaneIndex lane);
    // the modes the background's rocket path takes the pacesetter over in
    bool IsInRocketMode() const { return m_rocketMode; }
    bool IsInFluffyMode() const { return m_fluffyMode; }
    // @0x575320: a banana was collected (the minion is told in the rocket only: the rocket is not ported)
    void OnBananaCollect();
    // the bar's safe time: a collected banana keeps the bar from emptying for the time it is given (@0x5a0a60, Game's 0x748)
    void KeepDespicableBarSafe(float ms) { if (m_despicableTimer < ms) m_despicableTimer = ms; }
    // @0x579240: the run is started: the minion moves again, the level script is told, the physics has its first step
    void InitGameplay();
    // @0x57905c: the minion is put on the ground below where the level placed it
    void ProjectInitialPositionToGround();

    static jet::String k_tmplName;
    // @0xc8b884: the speed the cameras' blending times are made for
    static constexpr float k_referenceSpeed = 10.0f;
protected:
    virtual void SM_OnChangeState(ActorState *from, ActorState *to);
private:
    void UpdateStateMachine();
    void UpdateSpeed();
    float ComputeDesiredSpeed() const;
    float ComputeDesiredAcceleration() const { return m_physics[Physics_Acceleration]; }
    void UpdateMovement();
    // @0x57a5f8: the bar empties once the time after the last despicable action has run out, while the minion is in its first state
    void UpdateDespicablePoints();
    // @0x578d38: the last passed obstacle is counted when its time (a second) has run out
    void UpdatePassedObjects();
    // @0x578d9c: the last passed obstacle is counted for the kind it was passed in, then forgotten
    void CountLastPassedObstacle();
    // @0x574538: the freezing projectiles fly on, and the freeze distances of the lanes follow them (see FreezingProjectileCtrl)
    void UpdateFreezingProjectiles(unsigned dt);
    // @0x577b2c: the position goes on under the gravity or along the jump; the engine event the end of it asks for (-1 for none)
    int UpdateGravityMovement(math::vec3<float> &pos, math::quat<float> &rot);
    bool CheckFallOut(const math::vec3<float> &pos);
    void ChangeLane(int lanes, bool toRight);
    void UpdateDirAnimBlend(const math::vec3<float> &forward);
    void OnStateEnter(ActorState *from, ActorState *to);
    // @0x55bdd8: the minion is in a ride it engages or leaves (the fluffy and the fast forward flags)
    bool IsEngagingOrDisengagingRide() const { return m_fluffyMode || m_fastForward; }

    MinionCostume *m_costume = 0;                                      // 0x118
    math::vec3<float> m_initialPosition = math::vec3<float>(0, 0, 0);  // 0x220
    float m_speed = 0;                                                 // 0x22C: the pacesetter's speed of the frame
    float m_runDistance = 0;                                           // 0x230: how far it ran in the meters of the pacesetter
    float m_physics[Physics_Count];                                    // 0x234
    bool m_onGround = true;                                            // 0x260: the collision under it is solid ground
    bool m_afterCrashFlag = false;                                     // 0x261
    bool m_falling = false;                                            // 0x262: fell in a pit
    bool m_crashed = false;                                            // 0x263
    bool m_movementDisabled = true;                                    // 0x264: until the run starts
    bool m_deadFlag = false;                                           // 0x265: the death camera is on
    math::vec3<float> m_crashBounceScreenOffset = math::vec3<float>(0, 0, 0);  // 0x274: where on the screen it bounces to when it crashes
    bool m_fallOutFlag = false;                                        // 0x390: in the state of falling out
    math::vec3<float> m_deathCameraPosition = math::vec3<float>(0, 0, 0);  // 0x268
    math::vec3<float> m_moveDirection = math::vec3<float>(0, 0, 0);   // 0x280: where it moved the last frame
    math::quat<float> m_smoothRotation;                                // 0x28C
    math::vec3<float> m_lastPosition = math::vec3<float>(0, 0, 0);     // 0x29C
    math::vec3<float> m_bananaPosition = math::vec3<float>(0, 0, 0);   // 0x2A8
    bool m_inAir = false;                                              // 0x2B4: pushed up
    bool m_rampingSpeed = false;                                       // 0x2B5
    bool m_blendDirections = true;                                     // 0x2B6
    float m_lean = 0;                                                  // 0x2B8: the turn it makes
    math::vec3<float> m_lastDirection = math::vec3<float>(0, 0, 0);    // the direction the last frame looked (static in the original)
    jet::core::Delayer<math::vec3<float>, jet::core::LinearStep> m_positionDelayer;  // 0x308: where the pacesetter says it is
    jet::core::Delayer<math::quat<float>, jet::core::LinearStep> m_rotationDelayer;  // 0x338
    math::vec3<float> m_pacesetterPosition = math::vec3<float>(0, 0, 0);  // 0x374: the position the pacesetter gave the frame before
    int m_navigation = 0;                                              // 0x380: touch (the accelerometer is not ported)
    bool m_sledgeMode = false, m_rocketMode = false, m_fluffyMode = false, m_fastForward = false, m_frozenRun = false, m_cannon = false;  // 0x388..0x38D
    bool m_riding = false;       // 0x38E: the minion rides a vehicle (set by the states that enter one)
    bool m_largeMinion = false;  // 0x38F: the large minion (SetLargeMinionScale(true))
    bool m_largeMinionTransition = false;  // 0x391: set with the large minion's changes (SM_OnChangeState); what it means is not decoded
    int m_pusherDistance = 0;                                          // 0x424: the way to the exit of the pusher, times 0.1 (the run reads it)
    math::vec3<float> m_pusherStartPosition = math::vec3<float>(0, 0, 0);  // 0x428: where the pusher let it go (its z is then the exit's, see the .so)
    math::vec3<float> m_pusherExitPosition = math::vec3<float>(0, 0, 0);   // 0x434: the exit point of the pusher
    int m_postSledgeInvulnerabilityTime = 0;                           // 0x444
    int m_postShieldInvulnerabilityTime = 0;                           // 0x448
    int m_postFreezeGunInvulnerabilityTime = 0;                        // 0x44C
    unsigned m_distanceBase = 0, m_distanceRun = 0;                    // 0x500, 0x504
    unsigned m_locationEnterDistance = 0;                              // 0x508
    float m_despicablePoints = 0;                                      // 0x2BC: the points of the bar (its level is the statistic's despicable multiplier)
    float m_despicableTimer = 0;                                       // 0x2C0: ms until the bar starts to empty
    float m_freezeDespicablePoints = 0;                                // 0x2C4: the points a frozen obstacle gives (GameplayDef's DespicablePointsOnFreezeObjects)
    EffectControl *m_obstacleEffects[7] = {};                          // 0x46C..0x484: the effects of the obstacles it hit
    // the freezing projectile (the freeze ray's shot): its effect, the time it has flown, its two lane points and its lane (0x14 bytes)
    struct FreezingProjectileCtrl {
        EffectControl *effect;
        int time;
        float from, to;
        int lane;
    };
    const EffectDef *m_freezingBulletEffect = 0;                       // 0x4C8
    int m_freezeFireTime = 0;                                          // 0x4CC: ms until the freeze ray fires its next projectile
    float m_freezeDistances[3] = {0, 0, 0};                            // 0x4D0: per lane, the distance in the run the freezing reaches
    std::vector<FreezingProjectileCtrl> m_freezingProjectiles;         // 0x4DC
    ObstacleTemplate *m_lastPassedObstacle = 0;                        // 0x4F4: the obstacle passed last, counted when another one is passed
    int m_lastPassedTime = 0;                                          // 0x4F8: ms the last passed obstacle stays to be counted
    int m_lastPassedKind = 0;                                          // 0x4FC: 6 for a jump over, 0x23 for a roll under
    jet::String m_hitObstacleName;                                     // 0x518: the name of the obstacle the minion hit last
};

#endif
