// source/game/entities/TriggerEntity.cpp
#include "TriggerEntity.h"
#include <map>
#include "GameEntity.h"
#include "Minion.h"
#include "../GameLevel.h"
#include "game/scripting/ScriptData.h"

jet::String TriggerEntity::k_tmplName("Trigger");
const jet::String TriggerEntity::k_onEnter("OnEnter");
const jet::String TriggerEntity::k_onExit("OnExit");

// @0x525390
TriggerEntity::TriggerEntity(const clara::Template *t) : Collision3d(t), m_scriptData(0) {}

// @0x5253e0
TriggerEntity::~TriggerEntity() { delete m_scriptData; }

// @0x525368
const jet::String &TriggerEntity::GetBaseTemplateName() { return k_tmplName; }

// @0x525384
GameEntity *TriggerEntity::GetAsEntity() { return this; }

// @0x5254a4: a library trigger does nothing at init; otherwise the collision is made, and the script functions of the trigger are set up on the level's Lua VM
void TriggerEntity::Init() {
    if (m_flags & F_LIBRARY) return;
    Collision3d::Init();
    m_scriptData = new ScriptData(GameLevel::GetInstance()->GetLuaVM());
    std::map<jet::String, jet::String> args;
    m_scriptData->Init(this, args);
}

// @0x525544: the collision tells this trigger what touches it, and the trigger is updated
void TriggerEntity::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Collision3d::PostInit();
    SetCollisionListener(this);
    RegisterForUpdate(true);
}

// @0x525578: a collision that was inside is out, or one that was out is in now: OnEnter or OnExit runs for it. The flags are cleared for the
// next update, which only the collisions that touched again set
void TriggerEntity::Update() {
    for (size_t i = 0; i < m_colliders.size(); i++) {
        Collider &c = m_colliders[i];
        if (c.inside == c.touched) continue;
        c.inside = c.touched;
        if (c.touched) OnEnter(c.entity);
        else OnExit(c.entity);
    }
    for (size_t i = 0; i < m_colliders.size(); i++) m_colliders[i].touched = false;
}

// @0x5255dc: the collision that touches the trigger is marked touched in the list (added the first time)
void TriggerEntity::CollidesWith(Collision3d *c, const CollisionPair&) {
    GameEntity *entity = static_cast<GameEntity*>(c);
    for (size_t i = 0; i < m_colliders.size(); i++) {
        if (m_colliders[i].entity == entity) {
            m_colliders[i].touched = true;
            return;
        }
    }
    Collider added = { entity, false, true };
    m_colliders.push_back(added);
}

// @0x52571c: the script's OnEnter runs with the entity as the VM's other pointer
void TriggerEntity::OnEnter(GameEntity *e) {
    GameLevel::GetInstance()->GetLuaVM()->SetOtherPointer(e);
    m_scriptData->CallScriptFunction(k_onEnter);
}

// @0x525760: the script's OnExit runs with the entity as the VM's other pointer
void TriggerEntity::OnExit(GameEntity *e) {
    GameLevel::GetInstance()->GetLuaVM()->SetOtherPointer(e);
    m_scriptData->CallScriptFunction(k_onExit);
}

// @0x5257a4: pending to delete, and the minion has run past the distance it was removed at
bool TriggerEntity::IsOutOfScene() {
    if (!GetPendingToDelete()) return false;
    return GameLevel::GetInstance()->GetMinion()->GetTraveledDistance() >= m_distanceToDelete;
}
