// source/game/entities/ObstacleTemplate.cpp
#include "ObstacleTemplate.h"
#include <algorithm>
#include "Deco3d.h"
#include "InteractiveObjectTemplate.h"
#include "Pacesetter.h"
#include "Minion.h"
#include "../GameLevel.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../dynamicobjects/EffectMgr.h"
#include "../dynamicobjects/EffectDef.h"
#include "../Statistics.h"
#include "../Game.h"
#include "../gamemanagers/CostumeMgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "MinionCostume.h"
#include "param.h"
#include "project.h"
#include "core/util.h"
#include "scene/scenemgr.h"
#include "system.h"
#include "video/driver.h"
#include "scene/model.h"
#include "scene/node.h"

jet::String ObstacleTemplate::k_tmplName("ObstacleTemplate");
const jet::String ObstacleTemplate::k_enableStateCheck("EnableStateCheck"), ObstacleTemplate::k_useInRight("useInRight"), ObstacleTemplate::k_useInLeft("useInLeft"),
    ObstacleTemplate::k_triggerDistance("triggerDistance"), ObstacleTemplate::k_nearMissDistance("nearMissDistance"),
    ObstacleTemplate::k_successStateType("SuccessStateType"), ObstacleTemplate::k_successStateTypeOnPlatform("SuccessStateTypeOnPlatform"),
    ObstacleTemplate::k_collisionTiles("collisionTiles"), ObstacleTemplate::k_effectOnDestroyed("effectOnDestroyed"),
    ObstacleTemplate::k_nodeRefOnDestroyed("NodeRefOnDestroyed"), ObstacleTemplate::k_effectOnIdle("EffectOnIdle"),
    ObstacleTemplate::k_nodeRefOnIdle("NodeRefOnIdle"), ObstacleTemplate::k_engineEventOnCrash("engineEventOnCrash"),
    ObstacleTemplate::k_useLocalization("UseLocalization"), ObstacleTemplate::k_localizationArray("LocalObstaclePairs"),
    ObstacleTemplate::k_engineEventNearMiss("engineEventNearMiss"), ObstacleTemplate::k_effectOnCrash("effectOnCrash"),
    ObstacleTemplate::k_animOnCrash("animOnCrash"), ObstacleTemplate::k_hideOnCrash("hideOnCrash"),
    ObstacleTemplate::k_uselinkedInteractive("uselinkedInteractive"), ObstacleTemplate::k_linkedInteractive("linkedInteractive"),
    ObstacleTemplate::k_freezeObjectDeco("freezeObjectDeco"), ObstacleTemplate::k_freezeEffectOnDestroyed("freezeEffectOnDestroyed"),
    ObstacleTemplate::k_freezeEffect("freezeEffect"), ObstacleTemplate::k_param_obstacleId("obstacleId");
std::vector<ObstacleTarget*> ObstacleTemplate::s_obstacleTargets;
// the obstacle ids the data gives (the .so's ObstacleTemplate::k_type* strings; the cage's id is "rollingCage" in the data)
const jet::String ObstacleTemplate::k_typeFireLips("fireLips"), ObstacleTemplate::k_typeEvilCage("rollingCage"), ObstacleTemplate::k_typeSandCastle("sandCastle"),
    ObstacleTemplate::k_typeFireBowl("fireBowl"), ObstacleTemplate::k_typeBarbecue("barbecue"), ObstacleTemplate::k_typeHammock("hammock"),
    ObstacleTemplate::k_typeRedTapes("redTapes"), ObstacleTemplate::k_typeWetfloorSigns("wetfloorSigns"), ObstacleTemplate::k_typeSurfBoard("surfBoard"),
    ObstacleTemplate::k_typeAdvertisingPanels("advertisingPanels");
const float ObstacleTemplate::k_weaponPowerUp_distance = 30.0f;

// @0x57fe14
ObstacleTemplate::ObstacleTemplate(const clara::Template *t)
    : Object(t), m_triggerOnRail(false), m_triggerDistance(0), m_nearMissDistance(0), m_collisionStart(0), m_collisionTiles(0), m_gameplayTemplateInstance(0),
      m_effectOnDestroyed(0), m_effectOnIdle(0), m_engineEventOnCrash(-1), m_engineEventNearMiss(-1), m_useInRight(true), m_useInLeft(false), m_nearMiss(false),
      m_state(ObstacleState::Idle), m_lane((LaneIndex)3), m_collisionLaneStart(0), m_collisionLaneEnd(0), m_freezeDecoChanged(false), m_freezeDeco(0),
      m_freezeEffectOnDestroyed(0), m_freezeEffect(0), m_effectOnCrash(0), m_hideOnCrash(false), m_useLocalization(false), m_linkedInteractiveTemplate(0),
      m_linkedInteractiveInstance(0) {
    m_type = 0x16;
}

// @0x57ff14
ObstacleTemplate::~ObstacleTemplate() {
    if (m_gameplayTemplateInstance) {
        m_gameplayTemplateInstance->UnregisterListener(this);
        m_gameplayTemplateInstance = 0;
    }
}

// @0x580084
void ObstacleTemplate::Init() {
    if (m_flags & F_LIBRARY) return;
    Object::Init();
}

// @0x58096c: the enum values of a parameter
void ObstacleTemplate::LoadArray(const jet::String &param, std::vector<int> &out) {
    out.clear();
    const clara::Param *p = FindParamByName(param);
    if (!p) return;
    for (unsigned i = 0; i < p->GetComponentCount(); i++) out.push_back((int)p->GetAsEnum(i));
}

// the first of a parameter's references, or none
static clara::DataEntity *FirstReference(const clara::DataEntity *e, const jet::String &name) {
    if (!e->HasParam(name)) return 0;
    const clara::Path &path = e->FindParamByName(name)->GetAsPath(0);
    return path.IsEmpty() ? 0 : clara::Project::GetInstance()->FindEntityByPath(path);
}

// @0x580094
void ObstacleTemplate::PostInit() {
    bool useLinked = false;
    if (HasParam(k_uselinkedInteractive)) {
        GetParam(k_uselinkedInteractive, useLinked);
        if (useLinked) m_linkedInteractiveTemplate = dynamic_cast<GameEntity*>(FirstReference(this, k_linkedInteractive));
    }
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    bool stateCheck = false;
    GetParam(k_enableStateCheck, stateCheck);
    m_triggerDistance = m_nearMissDistance = 0;
    if (stateCheck) {
        GetParam(k_useInRight, m_useInRight);
        GetParam(k_useInLeft, m_useInLeft);
        GetParam(k_triggerDistance, m_triggerDistance);
        GetParam(k_nearMissDistance, m_nearMissDistance);
        LoadArray(k_successStateType, m_successStateTypeOnGround);
        LoadArray(k_successStateTypeOnPlatform, m_successStateTypeOnPlatform);
    }
    int tiles = 0;
    GetParam(k_collisionTiles, tiles);
    m_collisionTiles = (unsigned)tiles;
    GetParam(k_param_obstacleId, m_obstacleId);
    m_effectOnDestroyed = FirstReference(this, k_effectOnDestroyed);
    if (HasParam(k_engineEventOnCrash)) GetParam(k_engineEventOnCrash, m_engineEventOnCrash);
    if (HasParam(k_engineEventNearMiss)) GetParam(k_engineEventNearMiss, m_engineEventNearMiss);
    if (HasParam(k_freezeObjectDeco)) {
        const clara::Param *p = FindParamByName(k_freezeObjectDeco);
        const clara::Path &path = p->GetAsPath(m_decoIndex < p->GetComponentCount() ? m_decoIndex : 0);
        if (!path.IsEmpty()) m_freezeDeco = clara::Project::GetInstance()->FindEntityByPath(path);
    }
    m_freezeEffectOnDestroyed = FirstReference(this, k_freezeEffectOnDestroyed);
    m_freezeEffect = FirstReference(this, k_freezeEffect);
    m_effectOnIdle = FirstReference(this, k_effectOnIdle);
    RegisterForUpdate(true);
    if (HasParam(k_nodeRefOnIdle)) GetParam(k_nodeRefOnIdle, m_nodeRefOnIdle);
    if (const clara::Param *p = FindParamByName(k_effectOnCrash)) {
        const unsigned i = jet::core::Rand() % p->GetComponentCount();
        m_effectOnCrash = clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(i));
    }
    if (const clara::Param *p = FindParamByName(k_animOnCrash)) {
        const unsigned i = jet::core::Rand() % p->GetComponentCount();
        m_animOnCrash = p->GetAsEnumName(i);
    }
    if (HasParam(k_hideOnCrash)) GetParam(k_hideOnCrash, m_hideOnCrash);
    if (HasParam(k_nodeRefOnDestroyed)) GetParam(k_nodeRefOnDestroyed, m_nodeRefOnDestroyed);
    m_lane = (LaneIndex)3;
    SynchronizedObject::Init(this);
    AddSyncMovementDistance(m_nearMissDistance + m_triggerDistance);
    if (m_deco) m_idleAnim = m_deco->GetAnimationName();
}

// @0x56f520
void ObstacleTemplate::SetSpawnLane(int lane) { SynchronizedObject::SetSpawnLane((LaneIndex)lane); }

// @0x581e1c
void ObstacleTemplate::ConstructFrom(const GameEntity *o) { GameEntity::ConstructFrom(o); }

// @0x581360: the lanes of its collision: its own, and the neighbours it is used in (left of it, right of it)
void ObstacleTemplate::InitCollisionLanes() {
    int start = m_lane, end = m_lane;
    if (m_useInLeft && m_lane > 0) start = m_lane - 1;
    if (m_useInRight && m_lane <= 1) end = m_lane + 1;
    m_collisionLaneStart = start;
    m_collisionLaneEnd = end;
}

// @0x5813dc
void ObstacleTemplate::EnableTileCollisions(bool on) {
    if (!m_gameplayTemplateInstance) return;
    for (unsigned tile = 0; tile < m_collisionTiles; tile++)
        for (int lane = m_collisionLaneStart; lane <= m_collisionLaneEnd; lane++)
            m_gameplayTemplateInstance->SetCollisionOnLane((LaneIndex)lane, m_collisionStart + tile, on);
}

// the template of the obstacle: it hears when it goes (@0x5188d8 registers it)
void ObstacleTemplate::SetGameplayTemplateInstance(GameplayTemplateInstance *g) {
    m_gameplayTemplateInstance = g;
    g->RegisterListener(this);
}

// @0x5813b0: the template is gone, the obstacle does not block it any more
void ObstacleTemplate::OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent &e) {
    if (e.type != GameplayTemplateInstanceEvent::Destroyed) return;
    EnableTileCollisions(false);
    m_gameplayTemplateInstance = 0;
}

// @0x5813a0
void ObstacleTemplate::OnGameplayTemplateInstanceDestroyed() { SynchronizedObject::OnGameplayTemplateInstanceDestroyed(); }

// @0x5817fc
void ObstacleTemplate::Destroy(bool freezeDeco, bool frozen) {
    m_freezeDecoChanged = freezeDeco;
    m_state = frozen ? ObstacleState::DestroyedFrozen : ObstacleState::Destroyed;
}

// @0x581dfc
ObstacleTemplate *ObstacleTemplate::GetLocalizedObstacleTemplate(const jet::String &name) {
    std::map<jet::String, ObstacleTemplate*>::iterator it = m_localizationPairs.find(name);
    return it == m_localizationPairs.end() || !it->second ? this : it->second;
}

// @0x518a7c's pair
// @0x518b5c: a target that goes (a runner deleted with its level) leaves the list, so no obstacle reads it afterwards
ObstacleTarget::~ObstacleTarget() { ObstacleTemplate::RemoveTarget(this); }

void ObstacleTemplate::AddTarget(ObstacleTarget *t) { s_obstacleTargets.push_back(t); }
// @0x518a7c
void ObstacleTemplate::RemoveTarget(ObstacleTarget *t) { s_obstacleTargets.erase(std::remove(s_obstacleTargets.begin(), s_obstacleTargets.end(), t), s_obstacleTargets.end()); }

// @0x580a84: the interactive object linked to the obstacle blows up with it
void ObstacleTemplate::ExplodeLinkedInteractiveObject() {
    if (!m_linkedInteractiveInstance) return;
    if (InteractiveObjectTemplate *i = dynamic_cast<InteractiveObjectTemplate*>(m_linkedInteractiveInstance)) i->OnFreeze();
    m_linkedInteractiveInstance = 0;
    m_linkedInteractiveTemplate = 0;
}

// @0x581814
bool ObstacleTemplate::IsTargetOnRail(Pacesetter *p) {
    bool onRail = m_useInLeft ? m_useInRight : false;
    if (!p) return onRail;
    if (m_lane <= 2) {
        const int current = (int)p->GetCurrentLane();
        if (current == (int)m_lane) onRail = true;
        if (!m_useInLeft) {
            if (m_useInRight && m_lane != 2 && current == (int)m_lane + 1) onRail = true;
        } else if (!m_useInRight && m_lane > 0 && current == (int)m_lane - 1)
            onRail = true;
        return onRail;
    }
    // a lane that is none of the three: the obstacle must be beside the pacesetter's way
    math::vec3<float> d = GetPosition() - p->GetPosition();
    d.normalize();
    const math::vec3<float> right = p->GetRotation() * jet::scene::SceneMgr::GetRightVector();
    const float side = math::dot(right, d);
    return (side > -0.5f && side < 0.5f) || onRail;
}

// @0x58198c: 1 the pacesetter hits it on its rail, 2 it passes near it, 3 it is in reach on another rail
int ObstacleTemplate::CheckTrigger(Pacesetter *p) {
    const math::vec3<float> pacePos = p->GetPosition();
    math::vec3<float> pos = GetPosition();
    const bool onRail = IsTargetOnRail(p);
    if (m_triggerOnRail && onRail) return CheckTriggerResult::Hit;
    if (math::distanceSq(pacePos, pos) > 100.0f) return CheckTriggerResult::None;
    const math::vec3<float> forward = p->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) {
        math::quat<float> rot(0, 0, 0, 1);
        GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
    }
    // the plane of the obstacle, across the pacesetter's way
    const math::plane<float> plane(pos, forward);
    const math::vec3<float> diff = plane.projectPoint(pacePos) - pacePos;
    const float ahead = math::dot(forward, diff), lengthSq = diff.getLengthSq();
    bool inTrigger = false, inNear = false;
    if (ahead > -0.3f) inTrigger = lengthSq <= m_triggerDistance * m_triggerDistance || ahead < 0.3f;
    if (ahead > -0.9f) {
        const float reach = m_nearMissDistance + m_triggerDistance;
        inNear = lengthSq <= reach * reach || ahead < 0.9f;
    }
    if (inTrigger && onRail) return CheckTriggerResult::Hit;
    if (inNear && onRail) return CheckTriggerResult::NearMiss;
    return inTrigger && !onRail ? CheckTriggerResult::OffRail : CheckTriggerResult::None;
}

// @0x581758: the targets (the runners) meet the obstacle too
void ObstacleTemplate::UpdateTargetsTrigger() {
    if (m_state == ObstacleState::Destroyed || m_state == ObstacleState::DestroyedFrozen || m_state == ObstacleState::Gone) return;
    for (size_t i = 0; i < s_obstacleTargets.size(); i++) {
        if (CheckTrigger(s_obstacleTargets[i]->GetTargetPacesetter()) != CheckTriggerResult::Hit) continue;
        if (m_state == ObstacleState::WeaponPowerUp) Destroy(false, false);
        else s_obstacleTargets[i]->OnObstacleHit(this);
    }
}

// @0x581484: the minion met the obstacle: it stops the pacesetter, and a crash plays its crash animation (the deco's idle animation is kept), hides the
// deco when the data says so and starts the crash effect where the deco is (or where the obstacle is)
void ObstacleTemplate::OnHit(bool crashed) {
    SynchronizedObject::StopPacesetter(true);
    if (!crashed) return;
    if (!m_animOnCrash.empty() && m_deco) {
        m_idleAnim = m_deco->GetAnimationName();
        m_deco->SetAnimation(m_animOnCrash, false, Deco3d::AnimInterpolation::IfPlaying);
    }
    if (m_hideOnCrash && m_deco) m_deco->SetVisible(false);
    if (!m_effectOnCrash) return;
    math::vec3<float> pos = GetPosition();
    math::quat<float> rot = GetRotation();
    if (m_deco) {
        pos = m_deco->GetPosition();
        rot = m_deco->GetRotation();
    }
    EffectMgr::GetInstance()->Add(m_effectLayerId, reinterpret_cast<const EffectDef*>(m_effectOnCrash), pos, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
}

// @0x581624: the minion lives again: a hit obstacle is as it was, a destroyed one is destroyed again
void ObstacleTemplate::OnMinionRevive() {
    const bool wasStopped = SynchronizedObject::OnMinionRevive();
    if (wasStopped && m_state != ObstacleState::Destroyed && m_state != ObstacleState::DestroyedFrozen && m_state != ObstacleState::Gone) {
        if (!m_deco) return;
        if (!m_deco->IsVisible()) m_deco->SetVisible(true);
        if (!m_idleAnim.empty()) {
            if (m_deco->GetAnimationName().empty()) m_deco->SetAnimationTime(0);
            m_deco->SetAnimation(m_idleAnim, true, Deco3d::AnimInterpolation::None);
        }
        return;
    }
    if (m_state == ObstacleState::Destroyed || m_state == ObstacleState::DestroyedFrozen) return;
    Destroy(false, true);
    if (m_deco)
        for (unsigned i = 0; i < m_deco->GetLinkedEntitiesCount(); i++)
            if (GameEntity *g = dynamic_cast<GameEntity*>(m_deco->GetLinkedEntity(i))) g->OnMinionRevive();
}

// @0x58091c: the control is released (or stopped) and forgotten
void ObstacleTemplate::ReleaseEffectCtrl(EffectControl **ctrl, bool release) {
    if (!*ctrl) return;
    if (release) EffectMgr::GetInstance()->Release(*ctrl);
    else EffectMgr::GetInstance()->Stop(*ctrl);
    *ctrl = 0;
}

// @0x5806a0
void ObstacleTemplate::OnWorldChange() { ReleaseEffectCtrl(&m_effectOnIdleCtrl, true); }

// @0x5806ac: the effect of the obstacle is played on its node while the obstacle is in front of the camera and within 30 m of it: it is released
// when it is too far or behind the camera, and started (kept in the control, not auto-removed) when there is none and the node exists
void ObstacleTemplate::UpdateEffectCtrl(EffectControl **ctrl, const EffectDef *def, const jet::String &nodeRef) {
    jet::scene::Camera *camera = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World)->GetCamera();
    math::vec3<float> toObstacle = GetPosition() - camera->GetPosition();
    const float distanceSq = toObstacle.getLengthSq();
    if (distanceSq > 900.0f) {
        ReleaseEffectCtrl(ctrl, true);
        return;
    }
    if (distanceSq >= 25.0f) {
        toObstacle.normalize();
        if (math::dot(toObstacle, camera->GetFrontVector()) < 0) {
            ReleaseEffectCtrl(ctrl, true);
            return;
        }
    }
    if (*ctrl || nodeRef.empty() || !m_deco) return;
    const jet::scene::Model *model = m_deco->GetModel();
    const jet::scene::Node *node = model ? model->FindNodeByName(nodeRef) : 0;
    if (!node) return;
    EffectMgr *mgr = EffectMgr::GetInstance();
    mgr->Add(m_effectLayerId, def, node->GetAbsolutePosition(), node->GetAbsoluteRotation(), math::vec3<float>(1, 1, 1), false, node, -1, true, 0);
    *ctrl = mgr->GetLastAdded();
}

// @0x581010: a destroyed obstacle plays its destruction effect where its node is, or, when the freeze ray froze it, the freezing effect at its deco
// (the freezing count of the statistics goes up); its deco is hidden, the idle effect is let go, the collision is taken off the template and the
// obstacle is gone. The frozen destruction (state 3) only hides the deco. Not ported: the despicable points of the freezing (the minion's value
// at +0x2C4 is not decoded) and the score the freezing adds (the Game value at +0x6EC is not decoded).
void ObstacleTemplate::UpdateDestroyedEffects() {
    if (m_deco) {
        if (m_state != ObstacleState::DestroyedFrozen) {
            if (m_effectOnDestroyed && !m_freezeDecoChanged) {
                const jet::scene::Model *model = m_deco->GetModel();
                const jet::scene::Node *node = model ? model->FindNodeByName(m_nodeRefOnDestroyed) : 0;
                if (node)
                    EffectMgr::GetInstance()->Add(m_effectLayerId, reinterpret_cast<const EffectDef*>(m_effectOnDestroyed), node->GetAbsolutePosition(),
                                                  node->GetAbsoluteRotation(), math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
            }
            if (m_freezeEffectOnDestroyed && m_freezeDecoChanged) {
                math::vec3<float> pos = m_deco->GetPosition();
                math::quat<float> rot = m_deco->GetRotation();
                if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
                EffectMgr::GetInstance()->Add(m_effectLayerId, reinterpret_cast<const EffectDef*>(m_freezeEffectOnDestroyed), pos, rot, math::vec3<float>(1, 1, 1),
                                              true, 0, -1, true, 0);
                // the frozen obstacle gives the minion the freezing points (the despicable bar) and the score of the freeze gun's kill
                if (Minion *minion = GameLevel::GetInstance()->GetMinion()) minion->IncreaseDespicablePoints(minion->GetFreezeDespicablePoints(), pos);
                Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_score, (long long)Game::GetInstance()->GetScoreValue(Game::Score_FreezeGunDestroyObstacle));
                Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_freezeRayObjDestroyed, 1);
            }
        }
        m_deco->SetVisible(false);
    }
    ReleaseEffectCtrl(&m_effectOnIdleCtrl, false);
    m_effectOnIdle = 0;
    EnableTileCollisions(false);
    RegisterForRender(false);
    m_state = ObstacleState::Gone;
}

// @0x580ed4: an obstacle that waits (state 0) starts its weapon power up when the freeze ray is on and the minion is in reach. Otherwise, unless the
// minion has the god mode, the minion meets it: the trigger of the pacesetter (CheckTrigger) and the success states of the minion decide. A success
// passes the obstacle (the near miss waiting for it is counted first), a failure on the rail is the crash, and a near miss waits for the next one.
void ObstacleTemplate::UpdateIdle() {
    PowerUpMgr *powerUps = PowerUpMgr::GetInstance();
    if (powerUps->IsPowerUpActive(EPowerUpDef::Weapon)) {
        if (MustStartWeaponPowerUp()) {
            m_state = ObstacleState::WeaponPowerUp;
            UpdateWeaponPowerUp();
        }
        return;
    }
    if (powerUps->IsPowerUpActive(EPowerUpDef::GodMode)) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    const int trigger = CheckTrigger(minion->GetPacesetter());
    if (trigger == CheckTriggerResult::None) return;
    const std::vector<int> &types = minion->IsOnGround() ? m_successStateTypeOnGround : m_successStateTypeOnPlatform;
    if (types.empty()) return;
    const bool success = minion->CheckSuccessStateTypes(types);
    if (success || trigger == CheckTriggerResult::OffRail) {
        if (trigger != CheckTriggerResult::NearMiss && m_nearMiss) {
            minion->OnObstacleNearMiss(this);
            m_nearMiss = false;
        }
        if (success) minion->OnObstaclePassed(this);
        return;
    }
    if (trigger == CheckTriggerResult::Hit) {
        if (getenv("JET_DBGMINION"))
            fprintf(stderr, "obstacle trigger %s lane %d useInLeft %d useInRight %d triggerDistance %.2f nearMissDistance %.2f triggerOnRail %d success %d minion lane %d pacesetter x %.2f y %.2f minion x %.2f y %.2f\n", m_deco ? m_deco->GetName().c_str() : "-", (int)m_lane, (int)m_useInLeft, (int)m_useInRight, m_triggerDistance, m_nearMissDistance, (int)m_triggerOnRail, (int)success, (int)minion->GetPacesetter()->GetCurrentLane(), minion->GetPacesetter()->GetPosition().x, minion->GetPacesetter()->GetPosition().y, minion->GetPosition().x, minion->GetPosition().y);
        m_nearMiss = false;
        OnHit(minion->OnObstacleHitted(this));
    } else if (trigger == CheckTriggerResult::NearMiss) {
        m_nearMiss = true;
    }
}

// @0x5809f4: the freeze ray starts when the obstacle is on the minion's rail and the minion is within k_weaponPowerUp_distance of it
bool ObstacleTemplate::MustStartWeaponPowerUp() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!IsTargetOnRail(minion->GetPacesetter())) return false;
    return math::distance(minion->GetPosition(), GetPosition()) <= k_weaponPowerUp_distance;
}

// @0x580adc: the freeze reaches the obstacle when one of its lanes has a freeze distance (the minion's, per lane) at or beyond the obstacle's
// distance in the run
bool ObstacleTemplate::IsFreezingDistanceOk() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    for (int lane = m_collisionLaneStart; lane <= m_collisionLaneEnd; lane++)
        if (minion->GetFreezeDistance(lane) >= m_distanceInRun) return true;
    return false;
}

// @0x581c1c: the target is put on the plane across its way at the trigger distance before the obstacle (the obstacle's place, or its link point's
// when it is linked to a deco): a target that is still in front of that plane is moved onto it
void ObstacleTemplate::PlaceTargetOnTriggerPos(GameEntity *target) {
    if (!target) return;
    math::vec3<float> pos = GetPosition();
    if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) {
        math::quat<float> rot(0, 0, 0, 1);
        GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
    }
    const math::vec3<float> forward = target->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    const math::vec3<float> origin = pos - forward * m_triggerDistance;
    const math::plane<float> plane(origin, forward);
    const math::vec3<float> targetPos = target->GetPosition();
    const math::vec3<float> projected = plane.projectPoint(targetPos);
    math::vec3<float> toPlane = projected - targetPos;
    toPlane.normalize();
    if (math::dot(toPlane, forward) < 0) target->SetPosition(projected);
}

// @0x580b40: the weapon power up. An obstacle the minion meets on its rail is passed when the minion's success states allow it, and destroyed when
// they do not. Otherwise, with the freeze deco made, the freezing reaches the obstacle when its lane's freeze distance does: the freezing effect
// starts at the deco, the deco's copy takes its place (the link to the deco moves to the copy), and the costume may smash the obstacle apart.
// Without a freeze deco the deco blinks while the freeze ray is on.
void ObstacleTemplate::UpdateWeaponPowerUp() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    const int trigger = CheckTrigger(minion->GetPacesetter());
    if (trigger == CheckTriggerResult::Hit || trigger == CheckTriggerResult::NearMiss) {
        const std::vector<int> &types = minion->IsOnGround() ? m_successStateTypeOnGround : m_successStateTypeOnPlatform;
        if (minion->CheckSuccessStateTypes(types)) {
            minion->OnObstaclePassed(this);
            return;
        }
        Destroy(true, false);
        return;
    }
    if (!m_freezeDeco) {
        if (m_deco) {
            // the deco blinks every 128 ms of the driver's time (bit 7), and shows while the time is not yet running
            const int t = (int)jet::System::GetDriver()->GetTime();
            m_deco->SetVisible(t > 0 ? ((t >> 7) & 1) != 0 : true);
        }
        ExplodeLinkedInteractiveObject();
        return;
    }
    if (m_freezeDecoChanged || !IsFreezingDistanceOk()) return;
    // the place and the rotation the freezing effect is played at: the deco's, or the link point's of the deco it is linked to
    math::vec3<float> pos = m_deco->GetPosition();
    math::quat<float> rot = m_deco->GetRotation();
    if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) {
        GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
        pos = m_deco->GetPosition();
    }
    EffectMgr::GetInstance()->Add(m_effectLayerId, reinterpret_cast<const EffectDef*>(m_freezeEffect), pos, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
    m_freezeDecoChanged = true;
    Deco3d *frozen = new Deco3d(static_cast<Deco3d*>(m_freezeDeco), 0);
    if (Deco3d *first = GetLinkedDeco())
        if (first->GetLinkee() == static_cast<clara::Entity*>(m_deco)) {
            first->LinkTo(frozen, jet::String::null, 0);
            first->UpdateTransform(false);
        }
    AdoptDeco3d(frozen);
    ReleaseEffectCtrl(&m_effectOnIdleCtrl, false);
    m_effectOnIdle = 0;
    ExplodeLinkedInteractiveObject();
    if (CostumeMgr::GetInstance()->GetEquippedCostumeRef()->FreezeRaySmashesObstacleApart()) Destroy(true, false);
}

// @0x580e3c: in the run the obstacle plays its idle effect, meets the minion while it waits (UpdateIdle), runs its weapon power up (state 1) and
// the states it is destroyed in.
void ObstacleTemplate::Update() {
    GameLevel *level = GameLevel::GetInstance();
    Minion *minion = level->GetMinion();
    if (level->IsFluffyMode() || !minion || !minion->GetPacesetter()) {
        if (!GetPacesetter() || GetPacesetter()->IsPaused()) return;
        SynchronizedObject::Update();
        return;
    }
    Object::Update();
    UpdateTargetsTrigger();
    if (m_effectOnIdle) UpdateEffectCtrl(&m_effectOnIdleCtrl, reinterpret_cast<const EffectDef*>(m_effectOnIdle), m_nodeRefOnIdle);
    switch (m_state) {
    case ObstacleState::Idle: UpdateIdle(); break;
    case ObstacleState::WeaponPowerUp: UpdateWeaponPowerUp(); break;
    case ObstacleState::Destroyed:
    case ObstacleState::DestroyedFrozen:
        UpdateDestroyedEffects();
        break;
    case ObstacleState::Gone:
        if (m_deco && m_deco->IsVisible()) m_deco->SetVisible(false);
        break;
    default: break;
    }
    SynchronizedObject::Update();
}
