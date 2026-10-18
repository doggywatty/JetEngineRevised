// source/game/entities/Minion.cpp
#include "Minion.h"
#include "Collision3d.h"
#include "Deco3d.h"
#include "GameplayDef.h"
#include "JumpDefinition.h"
#include "LevelDef.h"
#include "ShadowLightPoint.h"
#include "MinionCostume.h"
#include "Pacesetter.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../Statistics.h"
#include "../actors/statemachine/ActorState.h"
#include "../actors/statemachine/ActorStateSet.h"
#include "../actors/statemachine/StateDatabase.h"
#include "../cheats/CheatsMgr.h"
#include "../gamemanagers/CostumeMgr.h"
#include "../gamemanagers/MinionSpeedMgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "../dynamicobjects/EffectMgr.h"
#include "../dynamicobjects/EffectControl.h"
#include "gameitem/GameItemType.h"
#include "gameitem/CollectableItem.h"
#include "Bonus.h"
#include "BonusSet.h"
#include "ObstacleTemplate.h"
#include "../gamestates/GS_Gameplay.h"
#include "../interfaces/menus/Menu_Ingame.h"
#include "../input/InputMgr.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "physics/physicsworld.h"
#include "camera/cameramgr.h"
#include "system.h"
#include "param.h"
#include "project.h"
#include "scene/model.h"
#include "scene/scenemgr.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

jet::String Minion::k_tmplName("Minion");
static const jet::String k_speedDefinitionParam("SpeedEvolutionDef");
static const jet::String k_mainMenuStateMachine("MainMenuStateMachine");
static const jet::String k_crashBounceScreenOffset("crashBounceScreenOffset");
static const jet::String k_spineDummy("Spine_Dummy");
static const jet::String k_movementDummy("Movement_Dummy");

// the gameplay definition of the level
static GameplayDef *Def() { return static_cast<GameplayDef*>(GameLevel::GetInstance()->GetGameplayDef()); }

// the engine events a power up sends when it starts (@0xc8b88c) and ends (@0xc8b8a8); -1 for none. Only the weapon (the freeze gun) has them.
static const int k_powerUpStartEngineEvent[EPowerUpDef::COUNT] = {-1, -1, -1, -1, -1, -1, 109};
static const int k_powerUpEndEngineEvent[EPowerUpDef::COUNT] = {-1, -1, -1, -1, -1, -1, 110};

// the engine events of the state machine data
enum {
    Event_Land = 4,
    Event_Jump = 9,
    Event_RunFast = 13,
    Event_RunSlow = 14,
    Event_Slide = 0x25,
    Event_LostGround = 0x24,
    Event_FellOut = 0x2a,
};

// @0x5738fc
Minion::Minion(const clara::Template *t)
    : Actor(t), m_smoothRotation(0, 0, 0, 1), m_positionDelayer(math::vec3<float>(0, 0, 0), math::vec3<float>(0, 0, 0), 1000),
      m_rotationDelayer(math::quat<float>(0, 0, 0, 1), math::quat<float>(0, 0, 0, 1), 1000) {
    m_type = 9;
    for (float &p : m_physics) p = 0;
}

// @0x573ce4
Minion::~Minion() {}

// @0x573ed4: the offset on the screen the minion bounces to when it crashes
void Minion::Init() {
    if (IsLibraryEntity()) return;
    Actor::Init();
    math::vec3<float> offset(0, 0, 0);
    GetParam(k_crashBounceScreenOffset, offset);
    m_crashBounceScreenOffset = offset;
}

// @0x573f70: the state machine of the data, and the one of the main menu next to it (its empty states are left at once)
void Minion::InitStateMachine() {
    Actor::InitStateMachine();
    if (const clara::Param *p = FindParamByName(k_mainMenuStateMachine)) {
        const clara::Param::StateValue v = p->GetAsState(0);
        SM_AddSateSet(v.a);
        if (const ActorStateSet *set = ActorStateSet::GetStateSetFromCollection(StateDatabase::GetInstance()->FindStateSetDataByName(v.a)))
            const_cast<ActorStateSet*>(set)->m_skipEmptyStates = true;
    }
}

// @0x574070: the minion is where the level placed it, the state machine starts at the first state of label 1 and the movement waits for the run
void Minion::PostInit() {
    if (IsLibraryEntity()) return;
    m_decoTransferred = false;
    m_initialPosition = GetPosition();  // @0x5740ac
    Actor::PostInit();
    RegisterForRender(true);
    RegisterForUpdate(true);
    SM_SetStateLabel(1);
    m_movementDisabled = true;
    m_positionDelayer.ResetValues(GetPosition(), GetPosition());
    m_rotationDelayer.ResetValues(GetRotation(), GetRotation());
    if (GetDeco()) ShadowLightPoint::SetModelRefPoint(GetDeco()->GetModel());  // @0x574210
    m_moveDirection = GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    m_smoothRotation = GetRotation();
    m_lastPosition = GetPosition();
    m_pacesetterPosition = m_positionDelayer.GetCurrentValue();
    m_falling = m_crashed = m_deadFlag = false;
    // the gameplay definition may have been read before the minion was there
    if (GameLevel::GetInstance()->GetGameplayDef())
        if (GameplayDef *def = dynamic_cast<GameplayDef*>(GameLevel::GetInstance()->GetGameplayDef())) def->ApplyToMinion();
}

// @0x574a40: the speed manager, the state machine, the speed of the pacesetter and the movement, in this order
void Minion::Update() {
    const unsigned dt = (unsigned)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    MinionSpeedMgr::GetInstance()->Update(dt);
    Actor::Update();
    UpdateStateMachine();
    UpdateSpeed();
    UpdatePassedObjects();
    UpdateMovement();
    UpdateDespicablePoints();
    UpdateFreezingProjectiles(dt);
    // where the bananas are collected: the spine of the body
    math::vec3<float> spine;
    math::quat<float> rot;
    m_bananaPosition = GetLinkPointTransform(k_spineDummy, spine, rot) ? spine : GetPosition();
    if (getenv("JET_DBGMINION")) {
        static int n = 0;
        if (++n % 30 == 0) fprintf(stderr, "camera ctrl %s\n", CameraMgr::GetInstance()->GetCurrentController() ? CameraMgr::GetInstance()->GetCurrentController()->GetType().c_str() : "none");
        if (n % 30 == 0) {
            const math::vec3<float> front = GetRotation() * jet::scene::SceneMgr::GetFrontVector();
            fprintf(stderr, "minion t=%lld state=%s type=%d pos=(%.1f %.1f %.1f) front=(%.2f %.2f %.2f) speed=%.2f ground=%d pace=%p lane=%d pacex=%.1f\n", (long long)jet::System::GetTime(), SM_GetState() ? SM_GetState()->GetName().c_str() : "-", SM_GetState() ? SM_GetState()->m_type : -1, GetPosition().x, GetPosition().y, GetPosition().z, front.x, front.y, front.z, m_speed, (int)m_onGround, (void*)GetPacesetter(), GetPacesetter() ? (int)GetPacesetter()->GetCurrentLane() : -1, GetPacesetter() ? GetPacesetter()->GetPosition().x : -1.0f);
        }
    }
    // the meters it ran (a meter is 0.9 of a unit)
    Statistics::GetInstance()->SetRanGameDistance((unsigned)(m_runDistance * 1.1111112f));
    if (Pacesetter *pace = GetPacesetter()) {
        m_speed = pace->GetSpeed();
        GameplayTemplateInstance *gti = pace->GetCurrentGameplayTemplateInstance();
        if (gti && gti->IsGameplayTemplate() && !IsInFluffyMode()) m_runDistance += pace->GetStep();
    }
}

// @0x575344: a minion that runs (or stands) takes the engine event of the speed it has
void Minion::UpdateStateMachine() {
    ActorState *s = SM_GetState();
    if (!s || (s->m_type != 0 && s->m_type != 0x33)) return;
    Pacesetter *pace = GetPacesetter();
    SM_OnEngineEvent(pace && pace->GetSpeed() > 10.0f ? Event_RunFast : Event_RunSlow);
}

// @0x579504: the speed the speed manager says (nothing in the fast forward)
float Minion::ComputeDesiredSpeed() const {
    if (m_fastForward || m_frozenRun) return 0;
    return MinionSpeedMgr::GetInstance()->GetCurrentSpeed();
}

// @0x5795d4: the pacesetter is asked for the speed, at the acceleration of the data, unless it is changing speed already or the minion
// fell or crashed
void Minion::UpdateSpeed() {
    Pacesetter *pace = GetPacesetter();
    if (!pace || pace->IsSpeedChanging() || m_falling || m_crashed) return;
    const float desired = ComputeDesiredSpeed();
    pace->SetSpeedAcceleration(ComputeDesiredAcceleration());
    pace->SetSpeed(desired, -1);
}

// @0x579a6c: the pacesetter says where the minion should be
void Minion::GoTo(const math::vec3<float> &pos, const math::quat<float> &rot) {
    m_positionDelayer.SetTargetValue(pos);
    m_rotationDelayer.SetTargetValue(rot);
}

// @0x57852c: the minion follows the pacesetter: along the lane it keeps its height (or falls and jumps in the gravity states), and it looks
// the way it moves
void Minion::UpdateMovement() {
    if (IsLinked() || m_movementDisabled) return;
    if (SM_GetState() && SM_GetState()->m_type == 0x32) {
        math::vec3<float> pos = GetPosition();
        math::quat<float> rot = GetRotation();
        UpdateGravityMovement(pos, rot);
        SetPosition(pos);
        SetRotation(rot);
        return;
    }
    if (m_frozenRun) return;
    const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
    if (GetPacesetter()) {
        const math::vec3<float> heightBefore = GetPosition() * up;
        const int dt = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
        m_positionDelayer.Update(dt);
        m_rotationDelayer.Update(dt);
        const math::vec3<float> pacePos = m_positionDelayer.GetCurrentValue();
        math::vec3<float> pos = pacePos;
        math::quat<float> rot = m_rotationDelayer.GetCurrentValue();
        // out of the gravity states the height follows the pacesetter's
        math::vec3<float> heightDelta(0, 0, 0);
        if (!SM_GetState()->IsGravityState()) heightDelta = (pacePos - m_pacesetterPosition) * up;
        pos = (pacePos - pacePos * up) + heightBefore + heightDelta;
        m_pacesetterPosition = pacePos;
        const int event = UpdateGravityMovement(pos, rot);
        SetPosition(pos);
        SetRotation(rot);
        if (event >= 0) SM_OnEngineEvent(event);
        // where it goes, and the way it looks, eased
        math::vec3<float> dir = GetPosition() - m_lastPosition;
        dir.normalize();
        m_moveDirection = dir;
        m_smoothRotation = math::lerp(m_smoothRotation, GetRotation(), 0.1f);
        m_lastPosition = GetPosition();
        if (m_rampingSpeed) {
            Pacesetter *pace = GetPacesetter();
            if (pace->GetTargetSpeed() / pace->GetSpeed() >= 0.5f) m_rampingSpeed = false;
        }
        if (m_blendDirections) UpdateDirAnimBlend(GetRotation() * jet::scene::SceneMgr::GetFrontVector());
    } else {
        math::vec3<float> pos = GetPosition();
        math::quat<float> rot = GetRotation();
        const int event = UpdateGravityMovement(pos, rot);
        SetPosition(pos);
        if (event >= 0) SM_OnEngineEvent(event);
    }
}

// @0x577b2c (the run): in a gravity state the gravity (or the jump) moves it and it can land (event 4), fall out of the level (0x2a) or
// walk out of the ground (0x24); out of the gravity states it is put on the ground below it
int Minion::UpdateGravityMovement(math::vec3<float> &pos, math::quat<float> &rot) {
    ActorState *state = SM_GetState();
    int event = -1;
    if (!state->IsGravityState()) {
        if (m_jump) SetCurrentJumpDefinition(0);
        // the ground below it, a little (0.2) under to a unit above; if there is none it fell off
        const math::vec3<float> front = m_smoothRotation * jet::scene::SceneMgr::GetFrontVector();
        const math::vec3<float> up = m_smoothRotation * jet::scene::SceneMgr::GetUpVector();
        const math::vec3<float> frontOfRot = rot * jet::scene::SceneMgr::GetFrontVector();
        math::vec3<float> p = pos;
        if (ProjectOnGround(p, rot, 0.2f, 1.0f)) {
            pos = p;
            m_onGround = m_groundCollision ? m_groundCollision->IsGroundCollision() : false;
        } else if (up.z > 0 && math::dot(-front, frontOfRot) > 0.0f) {
            event = Event_LostGround;
        }
    } else {
        const float heightBefore = pos.z;
        if (UpdateGravity(pos, !m_falling)) {
            if (m_jump) SetCurrentJumpDefinition(0);
            event = Event_Land;
        } else if (CanFallOut() && CheckFallOut(pos)) {
            event = Event_FellOut;
        } else if (pos.z < heightBefore) {
            // falling: what is under it tells whether it is solid ground
            math::vec3<float> p = pos;
            if (ProjectOnGround(p, -jet::scene::SceneMgr::GetUpVector(), 0.0f)) m_onGround = m_groundCollision ? m_groundCollision->IsGroundCollision() : false;
        }
    }
    if (getenv("JET_DBGMINION") && state->IsGravityState()) fprintf(stderr, "gravity z %.2f state %s event %d\n", pos.z, state->GetName().c_str(), event);
    // @0x5780a0: the cheat invincibility (CheatsMgr::IsInvincible @0x55e080) keeps a fall out (0x2a) or a walk out of the ground (0x24) from the
    // death. Below 5 m the height is raised to the floor value at the .so's +0x31c (a float in the position delayer: the z of its current value in
    // the port's layout), and a gravity state lands (Event_Land) while the others get no event. A walk out (0x24) only does this when the floor is
    // not below the height, as in the .so.
    if (event == Event_FellOut || event == Event_LostGround) {
        if (CheatsMgr::GetInstance()->IsInvincible()) {
            const float floorZ = m_positionDelayer.GetCurrentValue().z;
            if ((event == Event_FellOut || floorZ >= pos.z) && pos.z < 5.0f) {
                if (floorZ > pos.z) pos.z = floorZ;
                event = state->IsGravityState() ? Event_Land : -1;
            }
        }
    }
    return event;
}

// @0x5778b8: the minion that is going down looks for ground to the sides and below; none means it fell out of the level
bool Minion::CheckFallOut(const math::vec3<float> &pos) {
    const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
    if (math::dot(up, m_moveDirection) < -0.17f) {
        const math::vec3<float> front = GetRotation() * jet::scene::SceneMgr::GetFrontVector();
        const math::vec3<float> upRot = GetRotation() * up;
        math::vec3<float> ahead = front + upRot;
        ahead.normalize();
        math::vec3<float> down = ahead * m_moveDirection;
        down.normalize();
        math::vec3<float> p = pos;
        if (ProjectOnGround(p, down * 0.99f, 1.0f)) return false;
        math::vec3<float> p2 = pos;
        if (ProjectOnGround(p2)) return false;
    }
    Pacesetter *pace = GetPacesetter();
    bool right = false, left = false;
    if (pace) {
        right = CanDodgeRight();
        left = CanDodgeLeft();
    }
    // ground in the lanes next to it
    math::vec3<float> toRight(0, 0, pos.z), toLeft(0, 0, pos.z);
    if (right) {
        toRight = pace->GetLaneCachePosition((unsigned)pace->GetCurrentLane() + 1);
        toRight.z = pos.z;
    }
    if (left) {
        toLeft = pace->GetLaneCachePosition((unsigned)pace->GetCurrentLane() - 1);
        toLeft.z = pos.z;
    }
    if (right && ProjectOnGround(toRight)) return false;
    if (left && ProjectOnGround(toLeft)) return false;
    return true;
}

// @0x57a708: a minion on a jump that cannot fall out does not, and one that is not on a pacesetter never does
bool Minion::CanFallOut() const {
    if (m_jump && !m_jump->GetCanFallOut()) return false;
    if (GS_Gameplay::GetInstance()) return GetPacesetter() != 0;
    return true;
}

// @0x579b30, @0x579bf0
bool Minion::CanDodgeLeft() {
    Pacesetter *pace = GetPacesetter();
    return pace && m_navigation == 0 && pace->CanDodgeLeft(m_onGround);
}
// @0x579bf0
bool Minion::CanDodgeRight() {
    Pacesetter *pace = GetPacesetter();
    return pace && m_navigation == 0 && pace->CanDodgeRight(m_onGround);
}

// the lane change the pacesetter makes: the ones next to it, over the time of the data (@0x587ae0 for the right, @0x579420 for the left)
void Minion::ChangeLane(int lanes, bool toRight) {
    Pacesetter *pace = GetPacesetter();
    const int ms = (int)GetPhysicsValue(Physics_ChangeLaneTime);
    const float mu = pace->GetTargetChangeLaneMu();
    const int lane = (int)pace->GetCurrentLane();
    if (toRight) {
        const int free = (int)(2.0f - (float)lane - mu);
        pace->SetChangeLaneMu((float)std::min(lanes % 3, free) + mu, ms);
    } else {
        const int reachable = (int)((float)lane + mu);
        pace->SetChangeLaneMu(mu - (float)std::min(lanes % 3, reachable), ms);
    }
}

// @0x579320: a swipe to the sides: the state's triggers take it; one that has none (in the air) and allows dodging by input still changes
// the lane when it can
bool Minion::SM_OnScreenEvent(int code, int param, bool pressed) {
    bool taken = false;
    if (SM_FindTriggerIdxByEvent(2, code, param) != -1 || !SM_GetState()->m_allowDodgeByInput) return StateMachine::SM_OnScreenEvent(code, param, pressed);
    if (getenv("JET_DBGMINION") && GetPacesetter())
        fprintf(stderr, "screen event code=%d param=%d state=%s nav=%d right=%d left=%d lane=%d ctrl=%p mu=%.2f\n", code, param, SM_GetState() ? SM_GetState()->GetName().c_str() : "-", (int)m_navigation, (int)CanDodgeRight(), (int)CanDodgeLeft(), (int)GetPacesetter()->GetCurrentLane(), (void*)GetPacesetter()->GetController(), GetPacesetter()->GetTargetChangeLaneMu());
    if (code == 0 && param == 6) {
        if (CanDodgeRight() && GetPacesetter()) ChangeLane(1, true);
    } else if (param == 2 && code == 0) {
        if (CanDodgeLeft() && GetPacesetter()) ChangeLane(1, false);
    }
    return taken;
}

// @0x575220: the contact of the minion with the entity of the other collision. A collectable (type 4, CollectableItem) is taken unless the
// run is frozen (CollectableItem::OnMinionTouch @0x5664b8; the .so's GameLevel field 0x214 is always zero, so the vehicle check of the
// item is not reached). An obstacle (type 0x16) of the collision group 0x20 is destroyed while the god mode is on (ObstacleTemplate::Destroy
// @0x575220 with (false, false)).
void Minion::CollidesWith(Collision3d *c, const CollisionPair &pair) {
    (void)pair;
    Collision3dListener *l = c->GetCollisionListener();
    GameEntity *e = l ? l->GetAsEntity() : 0;
    if (!e) return;
    if (getenv("JET_DBGMINION"))
        fprintf(stderr, "minion contact type=%d template=%s group=%d frozen=%d\n", e->GetType(), e->GetTemplateName().c_str(), (int)c->GetCollisionGroup(), (int)m_frozenRun);
    if (e->GetType() == 4) {
        if (m_frozenRun) return;
        static_cast<CollectableItem*>(e)->OnMinionTouch(this);
    } else if (e->GetType() == 0x16) {
        if (c->GetCollisionGroup() != 0x20 || !PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::GodMode)) return;
        static_cast<ObstacleTemplate*>(e)->Destroy(false, false);
    }
}

// @0x5755e8: what entering a state does. The lane changes, the jump and the fall under gravity, the slide, the crash and the death (the
// states of the sledge, the mount, the rocket, the fluffy, the large minion, the cannon and the fast forward come with those modes).
void Minion::SM_OnChangeState(ActorState *from, ActorState *to) {
    StateMachine::SM_OnChangeState(from, to);
    if (!to) return;
    Pacesetter *pace = GetPacesetter();
    GameLevel *level = GameLevel::GetInstance();
    const bool both = from != 0;
    if (!both) return;
    if (to->IsDodgeRightState() && pace) ChangeLane(to->m_type == 0x22 ? 2 : 1, true);
    if (to->IsDodgeLeftState() && pace) ChangeLane(to->m_type == 0x21 ? 2 : 1, false);
    // the slide: the animation lasts as long as the minion takes to cover the distance of the slide
    if (from->m_type != 0x23 && to->m_type == 0x23) {
        const float speed = MinionSpeedMgr::GetInstance()->GetCurrentSpeed();
        const float ms = GetPhysicsValue(Physics_SlideDistance) * 1000.0f / speed;
        to->m_animSpeed = (float)(to->m_animEndTime - to->m_animStartTime) / ms;
    }
    // landing: on the ground
    if (from->m_type != 7 && to->m_type == 7) {
        math::vec3<float> pos = GetPosition();
        math::quat<float> rot = GetRotation();
        ProjectOnGround(pos, rot, 99999.0f, 1.0f);
        SetPosition(pos);
        m_inAir = false;
    }
    // the dash down: a strong gravity
    if (from->m_type != 0x27 && to->m_type == 0x27) {
        const float g = GetPhysicsValue(Physics_DashDownGravity) * 1e-6f;
        const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
        m_gravitySpeed[0] = m_gravitySpeed[1] = 0;
        m_gravity = math::vec3<float>(g * up.x, g * up.y, g * up.z);
        SetCurrentJumpDefinition(0);
        m_inAir = false;
    }
    // the jump: along the jump definition of the speed, else under the gravity of the data
    if (from->m_type != 5 && to->m_type == 5) {
        if (pace) {
            JumpDefinition *jd = Def() ? Def()->GetJumpDefinition(GameplayDef::Jump, pace->GetSpeed()) : 0;
            if (jd) {
                StartJumpDefinition(jd, GetPosition(), GetRotation());
            } else {
                const float targetSpeed = pace->GetTargetSpeed();
                ResetGravity(14.0f, GetPhysicsValue(Physics_JumpFrontDistance), GetPhysicsValue(Physics_JumpUpDistance), true);
                pace->SetSpeed(14.0f, 0);
                pace->SetSpeed(targetSpeed, 0x21);
                pace->HoldSpeedChange((int)m_gravitySpeed[0]);
            }
        } else {
            const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
            const float impulse = GetPhysicsValue(Physics_JumpImpulse) * 0.001f;
            math::vec3<float> dir(impulse * up.x, impulse * up.y, impulse * up.z);
            ResetGravity(dir, 1.0f);
        }
    }
    // falling off the ground: the gravity of the data and the fall definition of the speed
    if (!from->IsGravityState() && to->m_type == 0x12) {
        ResetGravity(m_sledgeMode ? GetPhysicsValue(Physics_SledgeGravityMultiplier) : 1.0f);
        if (pace) {
            JumpDefinition *jd = Def()
                                     ? Def()->GetJumpDefinition(m_onGround ? GameplayDef::FallFromGround : GameplayDef::Fall, pace->GetSpeed())
                                     : 0;
            if (jd && !m_sledgeMode && !m_inAir) {
                // a fall that starts on the liana leaves the pusher's link first: the jump sets the minion's place, and a linked entity keeps
                // that place as an offset to the link point (rotated by the link), which threw the minion sideways
                if (IsLinked() && GetLinkPointName().Equals(k_movementDummy)) LinkTo(0, jet::String::null, 0);
                StartJumpDefinition(jd, GetPosition(), GetRotation());
            }
        }
    }
    // the crash: the minion stops where the plane of the pacesetter cuts its way and no longer follows the pacesetter
    const bool wasCrash = from->m_type == 0x24 || from->m_type == 0x25;
    const bool isCrash = to->m_type == 0x24 || to->m_type == 0x25;
    if (!wasCrash && isCrash) {
        if (GS_Gameplay *g = GS_Gameplay::GetInstance()) g->EnableShootingArea(0);
        m_afterCrashFlag = true;
        if (pace) {
            pace->ResetLaneForCrash();
            const math::vec3<float> pacePos = pace->GetPosition();
            const math::vec3<float> pacefront = pace->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
            math::plane<float> cut(pacePos, pacefront);
            math::vec3<float> projected = cut.projectPoint(GetPosition());
            if (to->GetName().Equals("CrashBomb")) projected -= (pace->GetRotation() * jet::scene::SceneMgr::GetFrontVector()) * 0.8f;
            SetPosition(projected);
            SetRotation(pace->GetRotation());
            m_positionDelayer.ResetValues(GetPosition(), GetPosition());
            m_rotationDelayer.ResetValues(GetRotation(), GetRotation());
            m_pacesetterPosition = m_positionDelayer.GetCurrentValue();
            pace->SetSpeed(0, 0);
            SetPacesetter(0);
        }
    }
    // the death
    if (from->m_type != 0x2e && to->m_type == 0x2e) {
        if (GS_Gameplay *g = GS_Gameplay::GetInstance()) g->EnableShootingArea(0);
        m_deathCameraPosition = GetPosition();
        m_deadFlag = true;
        if (pace) {
            pace->SetSpeed(0, 0);
            SetPacesetter(0);
        }
    }
    if (!from->IsAnyCrashState() && to->IsAnyCrashState()) {
        if (GS_Gameplay *g = GS_Gameplay::GetInstance()) g->EnableShootingArea(0);
        PowerUpMgr::GetInstance()->DisableAll(false);
        m_crashed = true;
    }
    // falling into a pit: the gravity, the stop
    if (from->m_type != 0x26 && to->m_type == 0x26) {
        ResetGravity(1.0f);
        m_falling = true;
        Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_pitFallCount, 1);
        if (pace) pace->SetSpeed(0, 0);
        if (GetDeco()) GetDeco()->SetVisible(false);
    }
    if (from->m_type == 0x26) PowerUpMgr::GetInstance()->DisableAll(true);
    if (from->m_type != 0x2a && to->m_type == 0x2a) m_fallOutFlag = true;
    if (from->m_type == 0x2a && to->m_type != 0x2a) m_fallOutFlag = false;
    // the large minion's end: IsAnyDespicableEndState (0x2c, 0x2d) is the end of the mode. Entering it from another state flashes the level
    // back, and the flash's end ends the mode (GS_Gameplay::ApplyLargeMinionEndChanges); the transition flag 0x391 is set with it. Leaving it
    // for another state clears the flag and 0x38F. SetLargeMinionScale(false) (@0x576a7c, taken when `to` has no GIANTSCALE_ track) is not
    // ported: the port has no minion scale tween to set.
    if (!from->IsAnyDespicableEndState()) {
        if (to->IsAnyDespicableEndState()) {
            if (GS_Gameplay *g = GS_Gameplay::GetInstance()) g->MakeLargeMinionFinalFlash();  // @0x576a6c
            m_largeMinionTransition = true;                                                  // @0x575f44
        }
    } else if (!to->IsAnyDespicableEndState()) {
        m_largeMinionTransition = false;  // @0x576a80 (strb 0x391)
        m_largeMinion = false;            // @0x576a84 (strb 0x38f)
    }
}

// @0x57b07c etc.: the power ups tell the state machine
void Minion::OnPowerUpEnabled(EPowerUp p) {
    const int e = k_powerUpStartEngineEvent[p.underlying()];
    if (e >= 0) SM_OnEngineEvent(e);
}

// @0x57b188
void Minion::OnPowerUpDisabled(EPowerUp p, bool notify) {
    if (notify && k_powerUpEndEngineEvent[p.underlying()] >= 0) SM_OnEngineEvent(k_powerUpEndEngineEvent[p.underlying()]);
    // after the shield (3) or the freeze gun (6) is gone the minion is safe for a time
    if (notify && p.underlying() == EPowerUpDef::Shield) SetInvulnerable(m_postShieldInvulnerabilityTime);
    else if (notify && p.underlying() == EPowerUpDef::Weapon) SetInvulnerable(m_postFreezeGunInvulnerabilityTime);
}

// @0x575320: in the rocket the minion's rocket hears of the banana (its vfunc at +0xE8 on the mount at 0x25C); the rocket is not ported, so
// outside it nothing happens, as in the .so
void Minion::OnBananaCollect() {
    if (!m_rocketMode) return;
}

// @0x57b638: the invincible power up (for the time given) keeps the minion from being hurt
void Minion::SetInvulnerable(int ms) {
    if (!GetDeco()) return;
    PowerUpMgr::GetInstance()->EnablePowerUp(EPowerUpDef::Invincible, ms);
}

// @0x57a398: the points go on the bar (the despicable multiplier power up doubles the good ones, the costume scales them by its action factor).
// Each level of the bar it fills is a despicable multiplier; the points times the score multiplier go on the score; the safe time of the bar starts.
void Minion::IncreaseDespicablePoints(float points, const math::vec3<float> &where) {
    if (std::fabs(points) <= 1.1920929e-7f) return;
    if (PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::DespicableMultiplier) && points > 0) points *= 2;
    const int factor = CostumeMgr::GetInstance()->GetEquippedCostumeRef()->GetDespicableActionFactor();
    const float amount = (float)factor * points;
    Game *game = Game::GetInstance();
    Statistics *stats = Statistics::GetInstance();
    // the interface's bar (the points it shows) is the one that is full when the original stops here: that flag is set by Menu_Ingame::SetDespicablePointsPgt, not ported
    GS_Gameplay *gameplay = GS_Gameplay::GetInstance();
    Menu_Ingame *hud = gameplay ? gameplay->GetInterface() : 0;
    if (m_rocketMode || IsCrashed() || IsDead()) return;
    // the bar keeps no less than nothing
    const float filled = m_despicablePoints + amount;
    m_despicablePoints = filled < 0 ? 0 : filled;
    // every level of the bar that is full counts as one despicable multiplier (the last level of the data holds once the bar is past them all)
    for (;;) {
        const float level = game->MaxDespPointsArray_GetValue();
        // a level with no points never empties (the original loops forever on it)
        if (level <= 0 || m_despicablePoints < level) break;
        m_despicablePoints -= level;
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_despicableMultiplier, 1);
    }
    const int count = (int)amount;
    if (count <= 0) return;
    if (amount >= 1.0f && hud) hud->StartDespicableAnim(where);
    // the points: the score value of the despicable action for each, times the score multiplier
    const int value = game->GetScoreValue(Game::Score_Despicable);
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_score, (long long)stats->GetScoreMultiplier() * ((long long)value * count));
    stats->SetStatValueIfBetter(StatisticsSaveData::k_statScope_multiplierCountValue, stats->GetScoreMultiplier());
    stats->SetStatValue(StatisticsSaveData::k_statScope_distanceWithoutDespicable, 0);
    // the bar waits its safe time before it empties
    const float safeTime = game->GetDespicableSafeTime();
    if (m_despicableTimer < safeTime) m_despicableTimer = safeTime;
}

// @0x57b73c
float Minion::GetDespicablePointsPgt() { return m_despicablePoints / Game::GetInstance()->MaxDespPointsArray_GetValue(); }

// @0x57a5f8: while the minion is in its first state the bar empties at the rate of the data once its safe time has run out; the bar's fill goes to
// the interface every frame
void Minion::UpdateDespicablePoints() {
    if (SM_GetState() != SM_GetState(0)) return;
    Game *game = Game::GetInstance();
    const float dt = (float)game->GetUpdateTimeUs() / 1000.0f;
    m_despicableTimer -= dt;
    float depletion = 0;
    if (m_despicableTimer <= 0) {
        m_despicableTimer = 0;
        depletion = -(game->GetDespBarDepletionPerSec() * (dt * 0.001f));
    }
    IncreaseDespicablePoints(depletion, GetPosition());
    if (GS_Gameplay *gameplay = GS_Gameplay::GetInstance())
        if (Menu_Ingame *hud = gameplay->GetInterface()) {
            hud->SetDespicablePointsPgt(GetDespicablePointsPgt());
            hud->EnableDespicableBlinking(false);
        }
}

// @0x579e78: the minion passes the obstacle when it is in the air, its pacesetter is paused (or it has none), or its state is one of the list. An
// empty list gives that flag as it is.
bool Minion::CheckSuccessStateTypes(const std::vector<int> &types) const {
    const bool flag = m_inAir || !GetPacesetter() || GetPacesetter()->IsPaused();
    if (types.empty()) return flag;
    if (flag) return true;
    const ActorState *s = SM_GetState();
    if (!s) return false;
    for (size_t i = 0; i < types.size(); i++)
        if (s->m_type == types[i]) return true;
    return false;
}

// @0x579ee0: a hit obstacle crashes the minion unless the cheat invincibility is on (CheatsMgr::IsInvincible @0x55e080, checked first in the .so),
// it is invincible, has the invincible or the shield power up, or the minion cannot be hit (HitObstacles). The shield power up destroys the
// obstacle instead, and the shield goes when its bonus has no level left.
bool Minion::OnObstacleHitted(ObstacleTemplate *o) {
    PowerUpMgr *powerUps = PowerUpMgr::GetInstance();
    if (o->IsHittable() && !CheatsMgr::GetInstance()->IsInvincible() && !powerUps->IsPowerUpActive(EPowerUpDef::Invincible) &&
        !powerUps->IsPowerUpActive(EPowerUpDef::Shield) && HitObstacles()) {
        m_hitObstacleName = o->GetName();
        if (getenv("JET_DBGMINION")) fprintf(stderr, "obstacle hit %s id %s at (%.1f %.1f %.1f) minion (%.1f %.1f %.1f)\n", o->GetName().c_str(), o->GetObstacleId().c_str(), o->GetPosition().x, o->GetPosition().y, o->GetPosition().z, GetPosition().x, GetPosition().y, GetPosition().z);
        CountObtacleHit(o->GetObstacleId());
        o->PlaceTargetOnTriggerPos(this);
        SM_OnEngineEvent(o->GetEngineEventOnCrash());
        // the boss takes the hit of its obstacles (BossObstacleTemplate, not ported): GameLevel::GetBoss() is null while no boss is made
        for (int i = 0; i < 7; i++) {
            if (m_obstacleEffects[i]) EffectMgr::GetInstance()->Stop(m_obstacleEffects[i]);
            m_obstacleEffects[i] = 0;
        }
        return true;
    }
    if (powerUps->IsPowerUpActive(EPowerUpDef::Shield)) {
        o->Destroy(false, false);
        if (Bonus *shield = BonusSet::GetInstance()->GetBonus(7, true))
            if (shield->GetCurrentLevel() < 0) powerUps->DisablePowerUp(EPowerUpDef::Shield, true);
    }
    return false;
}

// @0x57a0bc: a near miss is counted when the minion can take the obstacle's hits (not invincible, nor with the invincible or the freeze ray
// power up, and HitObstacles). The engine event of the near miss goes to the minion, the statistics and the score count it, and the kind of
// obstacle is counted for the three kinds the data names.
bool Minion::OnObstacleNearMiss(ObstacleTemplate *o) {
    if (!o->IsHittable()) return false;
    // the cheat invincibility (@0x57a0e8 CheatsMgr::IsInvincible) takes no near miss either
    if (CheatsMgr::GetInstance()->IsInvincible()) return false;
    PowerUpMgr *powerUps = PowerUpMgr::GetInstance();
    if (powerUps->IsPowerUpActive(EPowerUpDef::Invincible) || powerUps->IsPowerUpActive(EPowerUpDef::Shield)) return false;
    if (!HitObstacles()) return false;
    if (o->GetEngineEventOnNearMiss() >= 0) SM_OnEngineEvent(o->GetEngineEventOnNearMiss());
    // the near miss's HUD notice (Menu_Ingame::OnNearMiss) is not ported
    Statistics *stats = Statistics::GetInstance();
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_nearMiss, 1);
    stats->IncreaseStatistic(StatisticsSaveData::k_statScope_score, (long long)Game::GetInstance()->GetScoreValue(Game::Score_NearMiss));
    const jet::String &id = o->GetObstacleId();
    if (id.Equals(ObstacleTemplate::k_typeEvilCage)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_nearMissRollingCagesCount, 1);
    if (id.Equals(ObstacleTemplate::k_typeSurfBoard)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_nearMissSurfBoardsCount, 1);
    if (id.Equals(ObstacleTemplate::k_typeAdvertisingPanels)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_nearMissAdvertisingPanelsCount, 1);
    return true;
}

// @0x57a290: the obstacle the minion passes on its lane becomes the last passed one (the one passed before is counted first). A jump or a
// slide keeps it for the jump over and the roll under counts, for the time of the data.
void Minion::OnObstaclePassed(ObstacleTemplate *o) {
    if (o == m_lastPassedObstacle) return;
    if (!o->IsOnLane((LaneIndex)GetPacesetter()->GetCurrentLane())) return;
    if (m_lastPassedObstacle) CountLastPassedObstacle();
    const ActorState *s = SM_GetState();
    if (!s) return;
    if (s->IsAnyJumpingState()) {
        m_lastPassedObstacle = o;
        m_lastPassedTime = 1000;
        m_lastPassedKind = 6;
    }
    if (s->m_type == 0x23) {
        m_lastPassedObstacle = o;
        m_lastPassedTime = 1000;
        m_lastPassedKind = s->m_type;
    }
}

// @0x578d9c: the last passed obstacle is counted for the kind it was passed in: a jump over counts the obstacles and the fire lips, sand castles
// and wet floor signs; a roll under counts the obstacles and the fire bowls, hammocks and red tapes. Then the last passed obstacle is forgotten.
void Minion::CountLastPassedObstacle() {
    Statistics *stats = Statistics::GetInstance();
    if (m_lastPassedKind == 6) {
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_jumpOverObstaclesCount, 1);
        if (m_lastPassedObstacle) {
            const jet::String &id = m_lastPassedObstacle->GetObstacleId();
            if (id.Equals(ObstacleTemplate::k_typeFireLips)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_jumpOverFireTonguesCount, 1);
            if (id.Equals(ObstacleTemplate::k_typeSandCastle)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_jumpOverSandCastlesCount, 1);
            if (id.Equals(ObstacleTemplate::k_typeWetfloorSigns)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_jumpOverWetfloorSignsCount, 1);
        }
    } else if (m_lastPassedKind == 0x23) {
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_rollUnderObstaclesCount, 1);
        if (m_lastPassedObstacle) {
            const jet::String &id = m_lastPassedObstacle->GetObstacleId();
            if (id.Equals(ObstacleTemplate::k_typeFireBowl)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_rollUnderFireBowlsCount, 1);
            if (id.Equals(ObstacleTemplate::k_typeHammock)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_rollUnderHammocksCount, 1);
            if (id.Equals(ObstacleTemplate::k_typeRedTapes)) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_rollUnderRedTapesCount, 1);
        }
    }
    m_lastPassedObstacle = 0;
    m_lastPassedKind = 0;
    m_lastPassedTime = 0;
}

// @0x574538: while the freeze ray is on, a projectile starts every k_freezingProjectile_Ratio ms from the gun's effect (or from the minion), as an
// effect of the bullet's definition. Each projectile flies k_freezingProjectile_Time ms along its lane: its two sides spread from the path of the
// pacesetter, and the lane's freeze distance is the traveled distance pushed on by the smooth step of its flight (k_freezingProjectile_distance).
// Each frame the lanes start again at the traveled distance, so the freezing reaches only what the projectiles are in front of.
void Minion::UpdateFreezingProjectiles(unsigned dt) {
    Pacesetter *pace = GetPacesetter();
    if (!pace) return;
    EffectMgr *effects = EffectMgr::GetInstance();
    if (PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::Weapon)) {
        m_freezeFireTime -= (int)dt;
        if (m_freezeFireTime <= 0) {
            m_freezeFireTime = k_freezingProjectile_Ratio;
            // the gun's effect (the seventh of the obstacle effects) places the projectile at its root node when there is one
            math::vec3<float> from = GetPosition();
            const math::quat<float> rot = GetRotation();
            if (EffectControl *gun = m_obstacleEffects[6])
                if (gun->model) {
                    jet::scene::Node &root = gun->model->GetRootNode();
                    root.UpdateAbsoluteTransform();
                    from = root.GetAbsolutePosition();
                }
            if (effects->Add(Object::m_effectLayerId, m_freezingBulletEffect, from, rot, math::vec3<float>(1, 1, 1), false, 0, -1, true, 0)) {
                // the two sides of the projectile are the minion's up and right, turned with its rotation, as far as the minion is from the pacesetter
                const math::vec3<float> offset = from - pace->GetPosition();
                const math::vec3<float> up = (rot * jet::scene::SceneMgr::GetUpVector()) * offset;
                const math::vec3<float> right = (rot * jet::scene::SceneMgr::GetRightVector()) * offset;
                m_freezingProjectiles.push_back(FreezingProjectileCtrl{effects->GetLastAdded(), 0, up.getLength(), right.getLength(), (int)pace->GetCurrentLane()});
                if (m_freezingProjectiles.size() > (size_t)k_freezingProjectile_Max - 1) {
                    effects->Release(m_freezingProjectiles.front().effect);
                    m_freezingProjectiles.erase(m_freezingProjectiles.begin());
                }
            }
        }
    }
    if (m_freezingProjectiles.empty()) return;
    // the lanes reach the traveled distance again, then each projectile pushes the reach of its lane on
    const float traveled = (float)(m_distanceBase + m_distanceRun);
    const float reach = traveled + k_freezingProjectile_distance;
    for (int lane = 0; lane < 3; lane++) m_freezeDistances[lane] = traveled;
    for (size_t i = 0; i < m_freezingProjectiles.size();) {
        FreezingProjectileCtrl &p = m_freezingProjectiles[i];
        p.time += (int)dt;
        float &laneReach = m_freezeDistances[p.lane];
        if (p.time >= k_freezingProjectile_Time) {
            if (reach >= laneReach) laneReach = reach;
            effects->Release(p.effect);
            m_freezingProjectiles.erase(m_freezingProjectiles.begin() + i);
            continue;
        }
        // the smooth step of the flight (0 at the start, 1 at the end)
        const float t = (float)p.time * (1.0f / (float)k_freezingProjectile_Time);
        const float ss = t * t * (3.0f - 2.0f * t);
        const float advance = ss * k_freezingProjectile_distance;
        if (traveled + advance >= laneReach) laneReach = traveled + advance;
        // the projectile is on the pacesetter's path, a little farther on, and its sides are placed from there
        math::vec3<float> pos;
        math::quat<float> r;
        pace->GetAdvancedTransformInPath(advance, pos, r, (LaneIndex)p.lane);
        pos = pos + (r * jet::scene::SceneMgr::GetUpVector()) * p.from;
        pos = pos + ((r * jet::scene::SceneMgr::GetRightVector()) * p.to) * (1.0f - ss);
        effects->SetPosition(p.effect, pos);
        effects->SetRotation(p.effect, r);
        i++;
    }
}

// @0x578d38: the last passed obstacle is counted once its time (a second, the data's) has run out
void Minion::UpdatePassedObjects() {
    const int dt = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    if (!m_lastPassedObstacle) return;
    m_lastPassedTime -= dt;
    if (m_lastPassedTime > 0) return;
    CountLastPassedObstacle();
    m_lastPassedTime = 0;
}

// @0x55bdec: the minion is hit by the obstacles it runs into, unless it is a rocket, engages or rides a ride, is frozen, rides a vehicle or falls
// out of the level
bool Minion::HitObstacles() const {
    if (m_rocketMode) return false;
    if (IsEngagingOrDisengagingRide()) return false;
    if (m_frozenRun) return false;
    if (m_riding) return false;
    return !m_fallOutFlag;
}

// @0x578f70: the hit is counted by the kind of obstacle: fire lips and sand castles at once, and fire bowls and barbecues while the minion jumps
void Minion::CountObtacleHit(const jet::String &type) const {
    Statistics *stats = Statistics::GetInstance();
    if (type.Equals(ObstacleTemplate::k_typeFireLips)) {
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_fireLipsCount, 1);
    } else if (type.Equals(ObstacleTemplate::k_typeSandCastle)) {
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_sandCastleHitCount, 1);
    }
    const ActorState *s = SM_GetState();
    if (!s || !s->IsAnyJumpingState()) return;
    if (type.Equals(ObstacleTemplate::k_typeFireBowl) || type.Equals(ObstacleTemplate::k_typeBarbecue))
        stats->IncreaseStatistic(StatisticsSaveData::k_statScope_fireLipsCount, 1);
}

// @0x57a764: the sledge begins: the power ups go off but the banana multiplier, and the state label 0x16 or 0x15 is set (by whether the
// sledge state has gravity)
void Minion::BeginSledge() {
    PowerUpMgr::GetInstance()->DisableAllButOne(EPowerUpDef::BananaMultiplier, true);
    const ActorState *s = SM_GetState();
    SM_SetStateLabel(s && s->IsGravityState() ? 0x16 : 0x15);
}

// @0x57a7b8
void Minion::EndSledge() { SM_OnEngineEvent(0x2e); }

// @0x57b578: after a death the speed goes back to the interval the data says, and the minion runs again from the first state of label 0x23
void Minion::Revive() {
    MinionSpeedMgr::GetInstance()->OnMinionRevive();
    PowerUpMgr::GetInstance()->DisableAll(true);
    SM_SetStateLabel(0x23);
    m_inAir = false;
    if (GetDeco()) GetDeco()->SetVisible(true);
}

// @0x57769c
void Minion::OnGroundHitted() { SM_OnEngineEvent(Event_Land); }

// @0x57b464, @0x57b464: the jump the pacesetter shares
void Minion::SetCurrentJumpDefinition(JumpDefinition *j) {
    m_jump = j;
    if (Pacesetter *pace = GetPacesetter()) pace->SetJump(j, j ? &m_jumpCtrl : 0);
}

// @0x57b494
void Minion::StartJumpDefinition(JumpDefinition *j, const math::vec3<float> &pos, const math::quat<float> &rot) {
    Actor::StartJumpDefinition(j, pos, rot);
    if (m_jump && GetPacesetter()) m_jump->InitCtrl(m_jumpCtrl, GetPacesetter()->GetSpeed());
}

// @0x57abf0: the pusher takes the minion: the state of the pusher plays, the pacesetter pauses and the minion is linked to the pusher's deco (its
// "Movement_Dummy" link point). The animation runs at the speed the pacesetter's speed gives between the two speed pairs (x: the speed, y: the
// animation speed in percent). An exit point makes a location transition start when the run is at its last template
void Minion::OnInteractiveAnimatedPusherStart(Object *pusher, const jet::String &state, const math::vec2<float> &minSpeed, const math::vec2<float> &maxSpeed,
                                              clara::Entity *exitPoint) {
    SM_SetState(state);
    GetPacesetter()->SetPaused(true);
    LinkTo(pusher->GetDeco(), k_movementDummy, 0);
    const float t = (math::clamp(GetPacesetter()->GetSpeed(), minSpeed.x, maxSpeed.x) - minSpeed.x) / (maxSpeed.x - minSpeed.x);
    const float animSpeed = (minSpeed.y + t * (maxSpeed.y - minSpeed.y)) * 0.01f;
    pusher->GetDeco()->SetAnimationSpeed(animSpeed);
    GetDeco()->SetAnimationSpeed(animSpeed);
    if (!exitPoint) return;
    GameLevel *gl = GameLevel::GetInstance();
    LevelTemplateMgr *mgr = gl->GetLevelTemplateMgr();
    GameplayTemplateInstance *gti = mgr->GetCurrentLevelTemplateInstance();
    if (gti->IsTransitionTemplate() || mgr->GetNextLevelTemplateInstance(gti)) gl->OnLocationTransitionLoadingStarted();
}

// @0x57ada4: the pusher lets the minion go. The minion leaves the link and is put where the pacesetter is on its lane, with the rotation of
// the pacesetter. An exit point (a location transition ahead) records the way to it, and resumes the creation of the templates when the run
// reaches one. Then the minion takes the pusher's jump when it has one; else it is pushed up by the upward speed, and the pacesetter goes at
// the front speed for the restore time back to the speed it had
void Minion::OnInteractiveAnimatedPusherStop(float upSpeed, float frontSpeed, int restoreTime, JumpDefinition *jump, clara::Entity *exitPoint) {
    const math::vec3<float> pos = GetPosition();
    LinkTo(0, jet::String::null, 0);
    if (exitPoint) {
        m_pusherStartPosition = pos;
        m_pusherExitPosition = exitPoint->GetPosition();
        // the .so stores the exit's z over the minion's (it writes the start position's z after the exit's copy)
        m_pusherStartPosition.z = m_pusherExitPosition.z;
        m_pusherDistance = (int)(math::distance(m_pusherStartPosition, m_pusherExitPosition) * 0.1f);
        LevelTemplateMgr *mgr = GameLevel::GetInstance()->GetLevelTemplateMgr();
        GameplayTemplateInstance *gti = mgr->GetCurrentLevelTemplateInstance();
        if (gti->IsTransitionTemplate() || mgr->GetNextLevelTemplateInstance(gti)) mgr->ResumeCreation();
    } else {
        GetPacesetter()->SetPaused(false);
    }
    GetPacesetter()->SetLanePosition(pos, 0, (LaneIndex)3, 0);
    SetRotation(GetPacesetter()->GetRotation());
    m_positionDelayer.SetCurrentValue(GetPosition());
    m_rotationDelayer.SetCurrentValue(GetRotation());
    m_positionDelayer.SetTargetValue(GetPosition());
    m_rotationDelayer.SetTargetValue(GetRotation());
    if (jump) {
        StartJumpDefinition(jump, GetPosition(), GetRotation());
        return;
    }
    if (upSpeed > 0) {
        math::vec3<float> dir = jet::scene::SceneMgr::GetUpVector() * (upSpeed * 0.001f);
        ResetGravity(dir, 1.0f);
    }
    const float previous = GetPacesetter()->GetTargetSpeed();
    if (frontSpeed != previous) {
        GetPacesetter()->SetSpeed(frontSpeed, 0);
        GetPacesetter()->SetSpeed(previous, restoreTime);
    }
}

// @0x55be48: the minion takes the jumpers unless it is in the rocket, riding or engaging a ride, frozen, in the cannon, falling out, or in
// the large minion's transition flag
bool Minion::InteractWithJumpers() const {
    if (m_rocketMode || IsEngagingOrDisengagingRide() || m_frozenRun || m_cannon || m_riding || m_fallOutFlag || m_largeMinionTransition) return false;
    return true;
}

// @0x579d8c
bool Minion::UseSpeeder() const {
    if (m_rocketMode) return false;
    if (IsEngagingOrDisengagingRide()) return false;
    if (m_fallOutFlag) return false;
    return !m_largeMinionTransition;
}

// @0x579cb0
bool Minion::PushUp(float upSpeed, float frontSpeed, int engineEvent, int restoreTime) {
    if (!SM_OnEngineEvent(engineEvent == -1 ? 0x3c : engineEvent)) return false;
    const math::vec3<float> up = jet::scene::SceneMgr::GetUpVector();
    const float s = upSpeed * 0.001f;
    math::vec3<float> dir(up.x * s, up.y * s, up.z * s);
    ResetGravity(dir, 1.0f);
    m_inAir = true;
    Pacesetter *pace = GetPacesetter();
    const float previous = pace->GetTargetSpeed();
    if (previous < frontSpeed) {
        pace->SetSpeed(frontSpeed, 0);
        pace->SetSpeed(previous, restoreTime);
    }
    return true;
}

// @0x579dd0
bool Minion::PushFront(float distance, float time, bool sendEngineEvent) {
    Pacesetter *pace = GetPacesetter();
    if (!pace) return false;
    m_rampingSpeed = true;
    if (sendEngineEvent) SM_OnEngineEvent(0x3d);
    const float t = time * 0.001f;
    const float v = pace->GetTargetSpeed();
    const float acceleration = (2.0f * v) / t - (2.0f * distance) / (t * t);
    pace->SetSpeed(v - acceleration * t, 0);
    pace->SetSpeed(v, (int)time);
    return true;
}

// @0x57b50c: an invincible minion takes the lane the bifurcation led to at once; one that rides the rocket (or a mount, which is not
// ported: IsEngagingOrDisengagingRide() is false) leaves it to the pacesetter's controller, and goes to it in a moment
void Minion::OnBifurcationResolved(LaneIndex lane) {
    if (!m_rocketMode) {
        if (!m_fastForward) return;
        GetPacesetter()->GoToLane(lane, 0);
        return;
    }
    if (GetPacesetter()->GetController()) return;
    GetPacesetter()->GoToLane(lane, 1);
}

// @0x57aafc: the way the minion runs, level (the front turned to the horizon) with its up
math::quat<float> Minion::GetTargetRotation() const {
    math::vec3<float> front = GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    front.z = 0;
    math::vec3<float> up = GetRotation() * jet::scene::SceneMgr::GetUpVector();
    front.normalize();
    up.normalize();
    return math::quatLookAt(front, up);
}

// @0x578300: how much the minion turns: the way it went since the last frame against the way the lane goes a little farther on, and
// to which side. (The lean animations the original blends in, through the IK delegate of the model, are not ported.)
void Minion::UpdateDirAnimBlend(const math::vec3<float> &forward) {
    if (m_lastDirection.getLengthSq() > 0) {
        math::vec3<float> ahead(0, 0, 0);
        math::quat<float> aheadRot(0, 0, 0, 1);
        GetPacesetter()->GetAdvancedTransformInPath(7.0f, ahead, aheadRot, (LaneIndex)3);
        ahead.z = m_lastPosition.z;
        math::vec3<float> wanted = ahead - m_lastPosition;
        wanted.normalize();
        const math::quat<float> looking = math::quatLookAt(m_lastDirection, jet::scene::SceneMgr::GetUpVector());
        const math::vec3<float> front = looking * jet::scene::SceneMgr::GetFrontVector();
        const math::vec3<float> right = looking * jet::scene::SceneMgr::GetRightVector();
        if (wanted.getLengthSq() != 0) {
            const bool toRight = math::dot(right, wanted) >= 0;
            const float turn = std::min(std::max((1.0f - math::dot(wanted, front)) * 10.0f, 0.0f), 1.0f);
            m_lean = toRight ? turn : -turn;
        }
    }
    math::vec3<float> flat(forward.x, forward.y, 0);
    flat.normalize();
    m_lastDirection = flat;
}

// @0x57b4c8
clara::DataEntity *Minion::GetSpeedDefinition() const {
    const clara::Param *p = FindParamByName(k_speedDefinitionParam);
    return static_cast<clara::DataEntity*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)));
}

// @0x57966c, @0x5796f0
void Minion::ChangePhysicsValue(int which, float v) {
    if ((unsigned)which < Physics_Count) m_physics[which] = v;
}
// @0x5796f0
float Minion::GetPhysicsValue(int which) const { return (unsigned)which < Physics_Count ? m_physics[which] : -1.0f; }

// @0x5776d4: the pacesetter's minion reads the swipes (in the touch navigation, with the sledge, the rocket and the large minion they
// are the accelerometer's, which is not ported)
void Minion::UpdateInput() {
    if (!GetPacesetter()) return;
    if (m_navigation == 0) InputMgr::GetInstance()->Update((int)(Game::GetInstance()->GetUpdateTimeUs() / 1000));
}

// @0x579240: the run starts: the movement is free, the state machine goes to the first state of label 1, the level script is told and the
// physics world takes its first step
void Minion::InitGameplay() {
    if (!m_movementDisabled) return;
    m_movementDisabled = false;
    SM_SetStateLabel(1);
    LevelDef::GetInstance()->OnStartGamePlay();
    GameLevel::GetInstance()->GetPhysicsWorld()->Update((int)(Game::GetInstance()->GetUpdateTimeUs() / 1000));
}

// @0x57905c: the minion stands on the ground below where the level put it, and the pacesetter is brought to it
void Minion::ProjectInitialPositionToGround() {
    math::vec3<float> pos = GetPosition();
    if (!ProjectOnGround(pos)) return;
    const math::vec3<float> moved = GetPosition() - pos;
    if (moved.getLength() <= 0) return;
    // (the start movies of the level -- Ref_UIMovie_MMToCannon, Ref_UIMovie_MMInit, Ref_Movie_NoChekpointInit -- are moved by the same offset in the
    // original: clara::Movie has no transform here)
    (void)moved;
    m_initialPosition = pos;
    SetPosition(pos);
    m_positionDelayer.ResetValues(pos, pos);
    m_pacesetterPosition = pos;
    if (GetPacesetter()) GetPacesetter()->SetLanePosition(pos, 0, (LaneIndex)3, 0);
}

// @0x577064 etc.: the state of the machine by the kind it is
bool Minion::IsIdle() const { return SM_GetState() && (SM_GetState()->m_type == 0 || SM_GetState()->m_type == 0x33); }
// @0x577088
bool Minion::IsJumping() const { return SM_GetState() ? SM_GetState()->IsAnyJumpingState() && !m_inAir : false; }
// @0x5770b0
bool Minion::IsRunning() const {
    const ActorState *s = SM_GetState();
    if (!s) return false;
    const int t = s->m_type;
    return t == 1 || t == 2 || t == 3 || t == 0 || t == 0x33 || t == 0x14 || t == 7;
}
// @0x577104
bool Minion::IsSliding() const { return SM_GetState() && SM_GetState()->m_type == 0x23; }
// @0x577200
bool Minion::IsLanding() const { return SM_GetState() && SM_GetState()->m_type == 7; }
// @0x577220
bool Minion::IsCrashed() const { return SM_GetState() && SM_GetState()->IsAnyCrashState(); }
// @0x577230
bool Minion::IsDead() const { return SM_GetState() && SM_GetState()->m_type == 0x1e; }
// @0x577284
bool Minion::IsDying() const { return SM_GetState() && SM_GetState()->IsDieState(); }

// the menus: a state of a set of the database
bool Minion::SetStateMachine(const jet::String &set, const jet::String &state) {
    if (!StateDatabase::GetInstance()->FindStateSetDataByName(set)) {
        fprintf(stderr, "ERROR: no state set %s\n", set.c_str());
        return false;
    }
    SM_SetStateSet(set, false);
    return SM_SetState(state);
}

// @0x57b26c: a copy of the costume's deco takes the place of the minion's; the state machine goes on with the new model
void Minion::SetCostume(Deco3d *ref, MinionCostume *costume) {
    m_costume = costume;
    SM_SetAnimatedCostume(costume);
    if (!ref) return;
    Deco3d *wear = new Deco3d(ref, 0);
    if (GetDeco()) {
        wear->SetTransform(GetDeco()->GetPosition(), GetDeco()->GetRotation(), GetDeco()->GetScale());
        AdoptDeco3d(wear);
    } else
        m_deco = wear;
    ShadowLightPoint::SetModelRefPoint(GetDeco()->GetModel());  // @0x57b2f4
    SM_ChangeModel(GetDeco()->GetModel());
}

// @0x57b698: the .so compares the deco it was made from with the equipped costume's (the Deco3d here does not keep what it
// was copied from), so the costume remembered by SetCostume stands for it
void Minion::ReloadCostumeIfNeeded() {
    CostumeMgr *costumes = CostumeMgr::GetInstance();
    MinionCostume *c = costumes->GetEquippedCostumeRef();
    if (!c->IsShopElement())
        if (MinionCostume *found = costumes->FindCostumeRefByName(costumes->GetEquippedCostumeName())) c = found;
    Deco3d *deco = c->GetCostumeDecoReference();
    if (!deco || !deco->CanLoadModel()) return;
    if (m_costume == (c->CanLoadModel() && c->IsShopElement() ? c : 0)) return;
    costumes->SetEquippedCostume(c, true);
}
