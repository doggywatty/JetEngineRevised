// source/game/entities/Enemy.cpp
#include "Enemy.h"
#include "Minion.h"
#include "Deco3d.h"
#include "../actors/statemachine/ActorState.h"

jet::String Enemy::k_tmplName("Enemy");

// @0x5610f8
Enemy::Enemy(const clara::Template *t) : Actor(t), m_minionHitted(0) {}

// @0x561144
Enemy::~Enemy() {}

// @0x5611c4
void Enemy::Init() {
    if (m_flags & F_LIBRARY) return;
    Actor::Init();
}

// @0x5611d4: the deco is transferred to the interface copies, and the enemy is updated and rendered
void Enemy::PostInit() {
    if (m_flags & F_LIBRARY) return;
    m_decoTransferred = true;
    Actor::PostInit();
    RegisterForRender(true);
    RegisterForUpdate(true);
}

// @0x561210
void Enemy::Update() { Actor::Update(); }

// @0x561214: empty in the .so
void Enemy::PushEntity(const math::vec2<float>&) {}

// @0x561218
void Enemy::OnMinionTouch(Minion *m) {
    m_minionHitted = m;
    SM_OnEngineEvent(3);
}

// @0x561238
void Enemy::OnMinionHit(Minion*) { SM_OnEngineEvent(6); }

// @0x561254: the state the enemy leaves as a hit one (0x10) to an idle or a dying one (0 or 0x33) hides it with its deco; the state 9 is
// the attack
void Enemy::SM_OnChangeState(ActorState *from, ActorState *to) {
    SM_ResetStateTracks();
    if (!to || !from) return;
    if (from->m_type == 0x10 && (to->m_type == 0 || to->m_type == 0x33)) {
        RegisterForUpdate(false);
        RegisterForRender(false);
        m_deco->RegisterForUpdate(false);
        m_deco->RegisterForRender(false);
    }
    if (from->m_type != 9 && to->m_type == 9) ExecuteAttack();
}
