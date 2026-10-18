// source/game/entities/BossObstacleTemplate.cpp
#include "BossObstacleTemplate.h"
#include <algorithm>
#include <cmath>
#include "Boss.h"
#include "BossMeena.h"
#include "BossMacho.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../gamestates/GS_Gameplay.h"
#include "../dynamicobjects/EffectMgr.h"
#include "../dynamicobjects/EffectDef.h"
#include "param.h"
#include "project.h"
#include "scene/scenemgr.h"
#include "scene/model.h"
#include "scene/node.h"

const jet::String BossObstacleTemplate::k_spawnTime("spawnTime"), BossObstacleTemplate::k_preWarningTime("preWarningAttackTime"),
    BossObstacleTemplate::k_warningTime("warningAttackTime"), BossObstacleTemplate::k_startAttackTime("attackTime"),
    BossObstacleTemplate::k_startVanishTime("vanishTime"), BossObstacleTemplate::k_spawningEffect("spawningEffect"),
    BossObstacleTemplate::k_preWarningEffect("preWarningEffect"), BossObstacleTemplate::k_warningEffect("warningEffect"),
    BossObstacleTemplate::k_attackEffect("attackEffect"), BossObstacleTemplate::k_impactEffect("impactEffect"),
    BossObstacleTemplate::k_touchedEffect("touchedEffect"), BossObstacleTemplate::k_behavior("bossObstacleBehavior"),
    BossObstacleTemplate::k_bossDistance("bossDistance"), BossObstacleTemplate::k_bossDamage("bossDamage"),
    BossObstacleTemplate::k_despicablePointsOnHit("despicablePointsOnHit"), BossObstacleTemplate::k_closingLaneStartTime("closingLaneStartTime"),
    BossObstacleTemplate::k_closingLaneEndTime("closingLaneEndTime"), BossObstacleTemplate::k_closingLaneEffect("closingLaneEffect"),
    BossObstacleTemplate::k_canBeReplacedByItem("canBeReplacedByItem"), BossObstacleTemplate::k_bodyParts("bodyParts"),
    BossObstacleTemplate::k_missileSpawnPosOffset("missileSpawnPosOffset");

// the .so's BodyPartOwner class of the boss obstacles (the touch of the minion's class mask finds them)
static const int k_bossObstacleClassType = 4;  // @0x5a78cc
// the missile's flight speed: the time (ms) of the flight per meter (@0x5a9410)
static const float k_returnMsPerMeter = 111.11111f;
// the time (ms) of the stop's parabola: its progress per ms (@0x5a89bc)
static const float k_stopParaboleProgressPerMs = 0.002f;
// the distance from the boss's plane the missile starts at when it is stopped (@0x5a9458)
static const float k_stopStartBossDistance = 100.0f;
// the hit sphere's growth when the obstacle is active (@0x5a85b4)
static const float k_hitSphereScale = 5.0f;

// the first reference of a parameter (@0x5a7b98..0x5a7c90: the path's entity, or none)
static clara::DataEntity *FirstReference(const clara::DataEntity *e, const jet::String &name) {
    if (!e->HasParam(name)) return 0;
    const clara::Path &path = e->FindParamByName(name)->GetAsPath(0);
    return path.IsEmpty() ? 0 : clara::Project::GetInstance()->FindEntityByPath(path);
}

// @0x5a78cc
BossObstacleTemplate::BossObstacleTemplate(const clara::Template *t)
    : ObstacleTemplate(t), m_spawnTime(0), m_preWarningTime(0), m_warningTime(0), m_startAttackTime(0), m_startVanishTime(0), m_lastDt(0),
      m_offsetDelayer(), m_missileSpawnPosOffset(0), m_behavior(-1), m_state(State::Idle), m_hasBossParams(false), m_spawningEffect(0),
      m_preWarningEffect(0), m_warningEffect(0), m_attackEffect(0), m_impactEffect(0), m_touchedEffect(0), m_spawningEffectCtrl(0),
      m_preWarningEffectCtrl(0), m_warningEffectCtrl(0), m_attackEffectCtrl(0), m_impactEffectCtrl(0), m_bossDistance(0), m_bossDamage(0),
      m_despicablePointsOnHit(0), m_returnFrom(0, 0, 0), m_returnTo(0, 0, 0), m_returnTime(0), m_returnTimeCount(0), m_missileOffset(0, 0, 0),
      m_missileOffsetStep(0, 0, 0), m_missileStopPos(0, 0, 0), m_stopParaboleTimer(0), m_stopParaboleState(0), m_stopLastBossDistance(0),
      m_firstUpdate(true), m_active(true), m_closingLaneStartTime(-1), m_closingLaneEndTime(-1), m_closingLaneEffect(0), m_closingLaneEffectCtrl(0),
      m_canBeReplacedByItem(false) {
    SetClassType((BodyPartOwner::BodyPartOwnerClassType)k_bossObstacleClassType);
}

// @0x5a7a00
BossObstacleTemplate::~BossObstacleTemplate() {
    ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
    ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
    ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
    ReleaseEffectCtrl(&m_warningEffectCtrl, true);
    ReleaseEffectCtrl(&m_attackEffectCtrl, true);
    ReleaseEffectCtrl(&m_impactEffectCtrl, true);
}

// @0x5a7af8
void BossObstacleTemplate::PostInit() {
    ObstacleTemplate::PostInit();
    if (m_flags & F_LIBRARY) return;
    GetParam(k_spawnTime, m_spawnTime);
    GetParam(k_preWarningTime, m_preWarningTime);
    GetParam(k_warningTime, m_warningTime);
    GetParam(k_startAttackTime, m_startAttackTime);
    GetParam(k_startVanishTime, m_startVanishTime);
    m_spawningEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_spawningEffect));
    m_preWarningEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_preWarningEffect));
    m_warningEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_warningEffect));
    m_attackEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_attackEffect));
    m_impactEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_impactEffect));
    m_touchedEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_touchedEffect));
    if (m_deco) DestroyDeco3d();
    GetParam(k_behavior, m_behavior);
    // the behaviours 0, 2 and 4 take the body parts and the boss's damage
    if (m_behavior == 0 || m_behavior == 2 || m_behavior == 4) {
        BPO_LoadBodyPartsArray(BPO_GetDefinitions(), FindParamByName(k_bodyParts));
        GetParam(k_bossDistance, m_bossDistance);
        GetParam(k_bossDamage, m_bossDamage);
        GetParam(k_despicablePointsOnHit, m_despicablePointsOnHit);
        m_hasBossParams = true;
    }
    // the missile is made this far in front of the boss in the behaviours 0 to 5
    if (m_behavior >= 0 && m_behavior <= 5) GetParam(k_missileSpawnPosOffset, m_missileSpawnPosOffset);
    if (m_behavior == 5) {
        GetParam(k_closingLaneStartTime, m_closingLaneStartTime);
        GetParam(k_closingLaneEndTime, m_closingLaneEndTime);
        m_closingLaneEffect = dynamic_cast<EffectDef*>(FirstReference(this, k_closingLaneEffect));
    }
    GetParam(k_canBeReplacedByItem, m_canBeReplacedByItem);
}

// @0x5a7ea0: the timeline of the obstacle: dt is the time left to its attack (it counts down to 0, then the obstacle vanishes for its vanish
// time). Its state follows the timeline, and the state's effects and movement are played. The body parts are sized while it is active.
void BossObstacleTemplate::Update(int dt) {
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Minion *minion = level->GetMinion();
    if (!m_firstUpdate) {
        dt = std::min(dt, m_lastDt);
        m_lastDt = dt;
    }
    if (m_active) {
        if (dt > 0) {
            const int total = m_spawnTime + m_preWarningTime + m_warningTime + m_startAttackTime;
            if (dt >= total || dt <= m_startAttackTime) {
                SetBossObstacleState(dt < m_startAttackTime ? State::Attack : State::Idle);
            } else if (dt > m_startAttackTime + m_warningTime + m_preWarningTime) {
                SetBossObstacleState(State::Spawning);
            } else if (dt > m_startAttackTime + m_warningTime) {
                SetBossObstacleState(State::PreWarning);
            } else {
                SetBossObstacleState(State::Warning);
            }
        } else {
            SetBossObstacleState(m_startVanishTime > 0 && -dt < m_startVanishTime ? State::Vanishing : State::Idle);
        }
    }
    // the closing lane's behaviour does not hit the rail of the minion
    if (m_behavior == 5) m_triggerOnRail = false;
    // the time of a frame, in ms
    const int ms = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    switch (m_state) {
    case State::Spawning:
    case State::PreWarning:
    case State::Warning: {
        if (m_firstUpdate) {
            // the approach starts: the boss shoots, and the offset delayer goes from the time to the attack's time
            if (!boss->CanShoot()) {
                SetBossObstacleState(State::Done);
                return;
            }
            if (m_behavior != 3) boss->OnShoot();
            m_lastDt = dt;
            const int total = m_preWarningTime + m_warningTime + m_startAttackTime;
            m_offsetDelayer.SetDelay((int)((float)dt - (float)total));
            m_offsetDelayer.SetCurrentValue((float)dt);
            m_offsetDelayer.SetTargetValue((float)total);
            m_firstUpdate = false;
        }
        // the minion's frame: the obstacle's place on the minion's plane, and the spawn offset in front of the boss on the same plane
        const math::quat<float> minionRot = minion->GetRotation();
        const math::vec3<float> forward = minionRot * jet::scene::SceneMgr::GetFrontVector();
        const math::plane<float> minionPlane(minion->GetPosition(), forward);
        const math::vec3<float> thisPos = GetPosition();
        const math::vec3<float> onMinion = minionPlane.projectPoint(thisPos);
        const math::vec3<float> bossPos = boss->GetPosition();
        const math::vec3<float> spawnOffset = forward * m_missileSpawnPosOffset;
        const math::plane<float> bossPlane(bossPos - spawnOffset, forward);
        math::vec3<float> onBoss = bossPlane.projectPoint(thisPos);
        if (m_state == State::Spawning) {
            // the spawn: the offset's progress from the spawn's time, and the boss's shoot position reached by a smoothstep of its z
            float t;
            if (m_spawnTime > 0) {
                const float left = m_offsetDelayer.GetCurrentValue() - (float)m_preWarningTime - (float)m_warningTime - (float)m_startAttackTime;
                t = 1.0f - left / (float)m_spawnTime;
            } else {
                t = 1.0f;
            }
            t = math::clamp(t, 0.0f, 1.0f);
            m_offsetDelayer.Update(ms);
            const math::vec3<float> shootPos = m_behavior == 4 ? boss->GetBombShootPosition() : boss->GetShootPosition();
            onBoss.z = shootPos.z + t * t * (onBoss.z - shootPos.z) * (3.0f - 2.0f * t);
            const float ease = 1.0f - (1.0f - t) * (1.0f - t);
            const math::vec3<float> pos = math::lerp(shootPos, onBoss, ease);
            UpdateEffect(m_spawningEffect, m_spawningEffectCtrl, pos, 1.0f, false);
        } else if (m_state == State::PreWarning) {
            if (!EnableClosingLaneAttack(dt)) {
                ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
                UpdateEffect(m_preWarningEffect, m_preWarningEffectCtrl, onBoss, 1.0f, false);
            } else {
                m_triggerOnRail = true;
                UpdateEffect(m_closingLaneEffect, m_closingLaneEffectCtrl, onBoss, 1.0f, false);
                ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
            }
        } else {
            // the warning: the missile's offset is eased from the boss's plane toward the minion's, by the warning time's progress
            float u;
            if (m_warningTime > 0) u = 1.0f - (float)(dt - m_startAttackTime) / (float)m_warningTime;
            else u = 1.0f;
            u = math::clamp(u, 0.0f, 1.0f);
            const math::vec3<float> oldOffset = m_missileOffset;
            m_missileOffset = math::lerp(onBoss, onMinion, u * u);
            m_missileOffsetStep = m_missileOffset - oldOffset;
            const bool bodyPartModel = m_behavior == 0 || m_behavior == 2 || m_behavior == 4;
            if (EnableClosingLaneAttack(dt)) {
                m_triggerOnRail = true;
                UpdateEffect(m_closingLaneEffect, m_closingLaneEffectCtrl, m_missileOffset, 1.0f, bodyPartModel);
                ReleaseEffectCtrl(&m_warningEffectCtrl, true);
            } else {
                ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
                UpdateEffect(m_warningEffect, m_warningEffectCtrl, m_missileOffset, 1.0f, bodyPartModel);
            }
        }
        break;
    }
    case State::Attack:
        // the attack: the missile moves on, and the body parts are off in the behaviours that have them
        if (m_startVanishTime != 0) {
            m_missileOffset += m_missileOffsetStep;
            UpdateEffect(m_warningEffect, m_warningEffectCtrl, m_missileOffset, 1.0f, false);
        }
        if (m_behavior == 0 || m_behavior == 2 || m_behavior == 4) {
            BPO_Enable(false);
            BPO_Release();
        }
        break;
    case State::Vanishing:
        m_missileOffset += m_missileOffsetStep;
        UpdateEffect(m_warningEffect, m_warningEffectCtrl, m_missileOffset, 1.0f, false);
        break;
    case State::FlyToBoss:
    case State::FlyPastBoss:
        UpdateToBoss(ms);
        break;
    case State::Stopped:
        UpdateStop(ms);
        break;
    default:
        break;
    }
    // while the boss's behaviour has its body parts and the obstacle is active, the body parts are on and the hit sphere grows
    if (m_hasBossParams && m_active) {
        GS_Gameplay::GetInstance()->EnableShootingArea(4);
        BPO_Update(ms);
        m_hitSphereRadius *= k_hitSphereScale;
    }
}

// @0x5a8cac: the minion's pacesetter gives the time the obstacle needs to reach its plane (the boss must be active); with no pacesetter the
// minion is dead
void BossObstacleTemplate::Update() {
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Minion *minion = level->GetMinion();
    if (m_active) ObstacleTemplate::Update();
    if (!boss || !minion || boss->GetBehaviorState() != Boss::BossState_Active) return;
    Pacesetter *ps = minion->GetPacesetter();
    if (!ps) {
        UpdateMinionIsDead();
        return;
    }
    const int timeToPlane = (int)ps->GetTimeToReachPlane(GetPosition(), GetRotation(), m_timeCache, 0);
    Update(timeToPlane);
}

// @0x5a8d90: the state changes; the effects of the states it leaves are released, and a done obstacle is no longer active
void BossObstacleTemplate::SetBossObstacleState(int s) {
    if (s == m_state) return;
    m_state = s;
    switch (s) {
    case State::Idle:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
        ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
        ReleaseEffectCtrl(&m_warningEffectCtrl, true);
        ReleaseEffectCtrl(&m_attackEffectCtrl, true);
        break;
    case State::PreWarning:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        break;
    case State::Warning:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
        ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
        break;
    case State::Attack:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
        ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
        if (m_startVanishTime == 0) ReleaseEffectCtrl(&m_warningEffectCtrl, true);
        break;
    case State::FlyToBoss:
    case State::FlyPastBoss:
    case State::Stopped:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
        ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
        ReleaseEffectCtrl(&m_warningEffectCtrl, true);
        ReleaseEffectCtrl(&m_attackEffectCtrl, true);
        m_active = false;
        break;
    case State::Done:
        ReleaseEffectCtrl(&m_spawningEffectCtrl, true);
        ReleaseEffectCtrl(&m_preWarningEffectCtrl, true);
        ReleaseEffectCtrl(&m_closingLaneEffectCtrl, true);
        ReleaseEffectCtrl(&m_warningEffectCtrl, true);
        ReleaseEffectCtrl(&m_attackEffectCtrl, true);
        ReleaseEffectCtrl(&m_impactEffectCtrl, true);
        m_active = false;
        break;
    default:
        break;
    }
}

// @0x5a8eb8: the effect is started (not removed by the manager) on the first call with a definition, at the obstacle's place and rotation; then
// it is put at the given position plus the definition's own position, its model's root is scaled by the definition's scale, and the body parts
// take its model when asked
void BossObstacleTemplate::UpdateEffect(const EffectDef *def, EffectControl *&ctrl, const math::vec3<float> &pos, float scale, bool setBodyPartModel) {
    EffectMgr *mgr = EffectMgr::GetInstance();
    if (def && !ctrl) {
        ctrl = mgr->Add(m_effectLayerId, def);
        if (ctrl) {
            ctrl->autoRemove = false;
            mgr->SetPosition(ctrl, GetPosition());
            mgr->SetRotation(ctrl, GetRotation());
        }
    }
    if (!ctrl || !ctrl->model) return;
    mgr->SetPosition(ctrl, pos + ctrl->def->GetPosition());
    if (def && m_behavior != 3) {
        const math::vec3<float> &s = def->GetScale();
        ctrl->model->GetRootNode().SetScale(math::vec3<float>(s.x * scale, s.y * scale, s.z * scale));
    }
    if (setBodyPartModel) BPO_SetModel(ctrl->model);
}

// @0x5a8604: the flight to the boss: the offset is the curve from the touch to the boss's plane, eased in its z, and it is the boss's place plus
// that offset. When the flight ends the boss takes the hit (FlyToBoss: the minion's points and the boss's damage, with the impact effect); the
// obstacle is done
void BossObstacleTemplate::UpdateToBoss(int dt) {
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Minion *minion = level->GetMinion();
    m_returnTimeCount = std::min(m_returnTimeCount + dt, m_returnTime);
    const float t = (float)m_returnTimeCount / (float)m_returnTime;
    const float ease = 1.0f - (1.0f - t) * (1.0f - t);
    const math::vec3<float> from(m_returnFrom.x, m_returnFrom.y, m_returnFrom.z + (m_returnTo.z - m_returnFrom.z) * ease);
    const math::vec3<float> offset = math::lerp(from, m_returnTo, t);
    const math::vec3<float> pos = boss->GetPosition() + offset;
    UpdateEffect(m_attackEffect, m_attackEffectCtrl, pos, 1.0f, false);
    if (t == 1.0f) {
        if (m_state == State::FlyToBoss) {
            const math::vec3<float> bossPos = boss->GetPosition();
            minion->IncreaseDespicablePoints(m_despicablePointsOnHit, bossPos);
            boss->IncreaseHealth(-m_bossDamage);
            EffectMgr::GetInstance()->Add(m_effectLayerId, m_impactEffect, pos, GetRotation(), math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
        }
        SetBossObstacleState(State::Done);
    }
}

// @0x5a87e0: the missile stopped at the lane of a Meena or Macho boss: its point follows a parabola (its height is the parabola over the
// stop's time; the lane moves it sideways at its ends), and when the point passes the boss's plane the boss is hit (a Meena or Macho is beaten by
// the obstacle as it is by the flight)
void BossObstacleTemplate::UpdateStop(int dt) {
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    if (!dynamic_cast<BossMeena*>(boss) && !dynamic_cast<BossMacho*>(boss)) return;
    m_stopParaboleTimer += dt;
    const float t = math::clamp((float)m_stopParaboleTimer * k_stopParaboleProgressPerMs, 0.0f, 1.0f);
    // the parabola: 0 at the stop's ends, 1 in the middle
    const float para = std::fabs(1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f));
    math::vec3<float> p(m_missileStopPos.x, m_missileStopPos.y, m_missileStopPos.z + para);
    if (m_lane == 0) p.y = m_missileStopPos.y - para;
    else if (m_lane == 2) p.y = m_missileStopPos.y + para;
    UpdateEffect(m_attackEffect, m_attackEffectCtrl, p, 1.0f, false);
    if (m_stopLastBossDistance < 0) return;
    Minion *minion = level->GetMinion();
    const math::vec3<float> forward = minion->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    const math::plane<float> bossPlane(boss->GetPosition(), forward);
    m_stopLastBossDistance = bossPlane.getDistanceTo(m_missileStopPos);
    if (m_stopLastBossDistance >= 0) return;
    const math::vec3<float> bossPos = boss->GetPosition();
    minion->IncreaseDespicablePoints(m_despicablePointsOnHit, bossPos);
    boss->IncreaseHealth(-m_bossDamage);
    EffectMgr::GetInstance()->Add(m_effectLayerId, m_impactEffect, bossPos, GetRotation(), math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
    SetBossObstacleState(State::Done);
}

// @0x5a8a5c: the minion is dead: the obstacle's impact effect is played where the first of its effects that has a model is (the warning, the
// pre-warning, the closing lane, the attack, the spawn and the impact, in the order the .so takes them), with that model's rotation; then the
// obstacle is done
void BossObstacleTemplate::UpdateMinionIsDead() {
    if (m_state == State::Done) return;
    EffectControl *found = 0;
    const jet::scene::Node *root = 0;
    EffectControl *candidates[] = {m_spawningEffectCtrl, m_preWarningEffectCtrl, m_closingLaneEffectCtrl, m_warningEffectCtrl, m_attackEffectCtrl};
    for (unsigned i = 0; i < sizeof(candidates) / sizeof(candidates[0]) && !root; i++) {
        found = candidates[i];
        if (found && found->model) root = &found->model->GetRootNode();
    }
    if (!root) {
        SetBossObstacleState(State::Done);
        return;
    }
    const math::vec3<float> pos = root->GetAbsolutePosition();
    math::quat<float> rot = root->GetAbsoluteRotation();
    EffectMgr::GetInstance()->Add(m_effectLayerId, m_impactEffect, pos, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
    SetBossObstacleState(State::Done);
}

// @0x599c5c: the closing lane attack (behaviour 5) is on while the timeline is between its start and its end times (counted down from the
// attack's total)
bool BossObstacleTemplate::EnableClosingLaneAttack(int dt) {
    if (m_behavior != 5) return false;
    const int total = m_spawnTime + m_preWarningTime + m_warningTime + m_startAttackTime;
    if (dt > total - m_closingLaneStartTime) return false;
    return dt >= total - m_closingLaneEndTime;
}

// @0x5a903c: the obstacle's crash, and the impact effect where it is
void BossObstacleTemplate::OnHit(bool crashed) {
    ObstacleTemplate::OnHit(crashed);
    if (!m_impactEffect) return;
    const math::vec3<float> pos = GetPosition();
    EffectMgr::GetInstance()->Add(m_effectLayerId, m_impactEffect, pos, GetRotation(), math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
}

// @0x599c38
void BossObstacleTemplate::BPO_OnBodyPartPressed(BodyPartSection*) {}

// @0x5a9110: the minion touches a body part: the touched effect plays where the touch is (the middle of the part, or its start when the part has
// no height), and the touch decides the boss's flight: the body parts of the boss's behaviours are off until the flight's end
void BossObstacleTemplate::BPO_OnBodyPartTouched(BodyPartSection *section) {
    math::vec3<float> touch(0, 0, 0);
    math::quat<float> rot(0, 0, 0, 1);
    if (section) {
        if (section->def->height != 0) {
            math::vec3<float> start, end;
            section->GetStartAndEnd(start, end);
            touch = math::lerp(start, end, 0.5f);
        } else {
            section->GetStart(touch);
        }
        jet::scene::Node *node = section->node;
        rot = node->GetAbsoluteRotation() * section->def->rotation;
    }
    EffectMgr::GetInstance()->Add(m_effectLayerId, m_touchedEffect, touch, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
    BPO_Enable(false);
    BPO_Release();
    m_hasBossParams = false;
    if (m_behavior != 0) {
        if (m_behavior == 2 || m_behavior == 4) {
            m_missileStopPos = m_missileOffset;
            SetBossObstacleState(State::Stopped);
            m_stopParaboleTimer = 0;
            m_stopParaboleState = 0;
            m_stopLastBossDistance = k_stopStartBossDistance;
        }
        return;
    }
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Minion *minion = level->GetMinion();
    if (!boss || !minion) return;
    // the flight: from the boss, the touch and its projection on the boss's plane (across the minion's right) are the ends of the curve
    const math::vec3<float> right = minion->GetRotation() * jet::scene::SceneMgr::GetRightVector();
    const math::vec3<float> bossPos = boss->GetPosition();
    const math::plane<float> plane(bossPos, right);
    math::vec3<float> projected = plane.projectPoint(touch);
    projected.z = bossPos.z;
    m_returnFrom = touch - bossPos;
    m_returnTo = projected - bossPos;
    m_returnTo.z -= m_attackEffect->GetPosition().z;
    if (math::distance(projected, bossPos) > m_bossDistance) {
        SetBossObstacleState(State::FlyPastBoss);
        m_returnTo += m_returnTo - m_returnFrom;
    } else {
        SetBossObstacleState(State::FlyToBoss);
    }
    m_returnTimeCount = 0;
    m_returnTime = (int)(math::distance(m_returnFrom, m_returnTo) * k_returnMsPerMeter);
}

// @0x5a9110 (the other body part events are not handled by the boss obstacle)
void BossObstacleTemplate::BPO_OnBodyPartDragged(BodyPartSection*) {}

// @0x599c50
void BossObstacleTemplate::BPO_OnBodyPartReleased(BodyPartSection*) {}
