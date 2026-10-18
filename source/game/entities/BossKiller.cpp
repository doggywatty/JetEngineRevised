// source/game/entities/BossKiller.cpp
#include <cmath>
#include "BossKiller.h"
#include "Boss.h"
#include "Minion.h"
#include "BossGameplay.h"
#include "../GameLevel.h"
#include "scene/scenemgr.h"

jet::String BossKiller::k_tmplName("BossKiller");
const jet::String BossKiller::k_exitStateParamName("ExitState");
const jet::String BossKiller::k_defeatedExitStateParamName("DefeatedExitState");

// the distance from the killer's plane under which the boss is at the plane (@0x5a6474)
static const float k_killPlaneDistance = 0.5f;

// @0x5a62bc
BossKiller::BossKiller(const clara::Template *t) : Object(t) {}

// @0x5a6304
BossKiller::~BossKiller() {}

// @0x5a6380: the exit states are the data's states (the state's second name); a killer that is a library is not set up
void BossKiller::PostInit() {
    Object::PostInit();
    if (m_flags & F_LIBRARY) return;
    if (const clara::Param *p = FindParamByName(k_exitStateParamName)) m_exitStateName = p->GetAsState(0).b;
    if (const clara::Param *p = FindParamByName(k_defeatedExitStateParamName)) m_defeatedExitStateName = p->GetAsState(0).b;
}

// @0x5a640c: while the level's boss is active or defeated and the minion is in the game, the boss gameplay is killed when the killer's plane
// (through the killer, facing the minion's direction) is at the boss
void BossKiller::Update() {
    Object::Update();
    GameLevel *level = GameLevel::GetInstance();
    Boss *boss = level->GetBoss();
    Minion *minion = level->GetMinion();
    if (!boss || !minion) return;
    const Boss::BossState state = boss->GetBehaviorState();
    if (state != Boss::BossState_Active && state != Boss::BossState_Defeated) return;
    const math::vec3<float> normal = minion->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    const math::plane<float> plane(GetPosition(), normal);
    if (std::fabs(plane.getDistanceTo(boss->GetPosition())) <= k_killPlaneDistance) level->GetBossGameplay()->KillBoss(m_exitStateName, m_defeatedExitStateName);
}
