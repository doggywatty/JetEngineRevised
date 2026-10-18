// source/game/entities/BossActionTrigger.cpp
#include "BossActionTrigger.h"
#include "project.h"
#include "GameEntity.h"
#include "Boss.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "../GameLevel.h"
#include "scene/scenemgr.h"

jet::String BossActionTrigger::k_tmplName("BossActionTrigger");
const jet::String BossActionTrigger::k_engineEventParam("engineEvent");

// the distance (squared) of the trigger from the pacesetter's position at which the trigger fires (@0x5a5014, the literal 20.0)
static const float k_fireDistanceSq = 20.0f;

// @0x5a4e1c
BossActionTrigger::BossActionTrigger(const clara::Template *t) : GameEntity(t), m_engineEvent(0), m_state(TriggerState_Waiting) {}

// @0x5a4e50
BossActionTrigger::~BossActionTrigger() {}

// @0x5a4ea0: a library trigger does nothing at init
void BossActionTrigger::Init() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::Init();
}

// @0x5a4eb0: the trigger is updated, and its engine event is read from the data
void BossActionTrigger::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::PostInit();
    RegisterForUpdate(true);
    m_state = TriggerState_Waiting;
    GetParam(k_engineEventParam, m_engineEvent);
}

// @0x5a4f08: when the minion's pacesetter has passed the trigger's plane (the plane through the trigger facing the run) and is within
// reach of it, the boss gets the engine event, once
void BossActionTrigger::Update() {
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Pacesetter *ps = level->GetMinion()->GetPacesetter();
    if (m_state != TriggerState_Waiting) return;
    if (!boss || !ps) return;
    const math::vec3<float> forward = ps->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    const math::plane<float> plane(GetPosition(), forward);
    if (plane.getDistanceTo(ps->GetPosition()) <= 0.0f) return;
    const math::vec3<float> d = GetPosition() - ps->GetPosition();
    if (d.getLengthSq() >= k_fireDistanceSq) return;
    boss->SM_OnEngineEvent(m_engineEvent);
    m_state = TriggerState_Fired;
}
