// source/game/entities/Puppet.cpp
#include "Puppet.h"
#include "Boss.h"
#include "Deco3d.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "project.h"

jet::String Puppet::k_tmplName("Puppet");
const jet::String Puppet::k_stateMachine("StateMachine");

// the place and rotation an entity has relative to its link point (its own when it is not linked) (clara::Entity::GetLocalPosition @0x517f7c)
static math::vec3<float> LocalPosition(const clara::Entity *e) {
    return e->GetLinkData() ? e->GetLinkData()->position : e->GetPosition();
}
static math::quat<float> LocalRotation(const clara::Entity *e) {
    return e->GetLinkData() ? e->GetLinkData()->rotation : e->GetRotation();
}

// @0x51b90c
Puppet::Puppet(const clara::Template *t) : Object(t), StateMachine() {}

// @0x51b968
Puppet::~Puppet() {}

// @0x51ba44: the puppet's state machine starts in the state the data names, with the deco's model
void Puppet::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    const clara::Param::StateValue &state = FindParamByName(k_stateMachine)->GetAsState(0);
    SM_Init(m_deco->GetModel(), state);
}

// @0x51bac4: a linked entity is linked to the same entity here, at its local place and rotation; the rest is the game entity's
void Puppet::ConstructFrom(const GameEntity *e) {
    if (e->IsLinked()) {
        LinkTo(e->GetLinkee(), e->GetLinkPointName(), 0);
        SetTransform(LocalPosition(e), LocalRotation(e));
    }
    GameEntity::ConstructFrom(e);
}

// @0x51bb3c: the object moves and the state machine goes on (the port's state machine takes its time from the frame)
void Puppet::Update() {
    Object::Update();
    SM_Update();
}

// @0x51bb84: the puppet is linked to the boss's deco (once), at the place it had; the boss gets the puppet and its state machine is
// linked to the puppet's, so the boss's engine events and state changes reach the puppet
void Puppet::LinkToBoss(Boss *boss) {
    if (!boss || !boss->GetDeco()) return;
    if (IsLinkedTo(boss->GetDeco())) return;
    const math::vec3<float> localPos = LocalPosition(this);
    const math::quat<float> localRot = LocalRotation(this);
    SetTransform(math::vec3<float>(0.0f, 0.0f, 0.0f), math::quat<float>(0.0f, 0.0f, 0.0f, 1.0f));
    LinkTo(boss->GetDeco(), GetLinkPointName(), 0);
    SetTransform(localPos, localRot);
    boss->SetPuppet(this);  // the boss's state machine is linked to this one (SetPuppet)
}

// @0x51bc5c: the puppet linked to a boss's deco is not out of the scene
bool Puppet::IsOutOfScene() const {
    if (IsLinked()) return false;
    return Object::IsOutOfScene();
}
