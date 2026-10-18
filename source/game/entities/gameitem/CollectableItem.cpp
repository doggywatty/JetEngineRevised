// source/game/entities/gameitem/CollectableItem.cpp
#include "CollectableItem.h"
#include "../Deco3d.h"
#include "param.h"

const jet::String CollectableItem::k_appearProbability("AppearProbability");

// @0x5662e0
CollectableItem::CollectableItem(const clara::Template *t) : Object(t), m_state(CollectableStateDef::Idle) { m_type = 4; }

// @0x56632c
CollectableItem::~CollectableItem() {}

// @0x566394
void CollectableItem::Init() {
    if (m_flags & F_LIBRARY) return;
    Object::Init();
    if (HasParam(k_appearProbability)) {
        int p = 100;
        GetParam(k_appearProbability, p);
        m_appearProbability = p;
    }
}

// @0x566400
void CollectableItem::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    RegisterForRender(true);
    RegisterForUpdate(true);
}

// @0x566434: a collected item is hidden with what is linked to it
void CollectableItem::Update() {
    Object::Update();
    if (m_state != CollectableStateDef::Collected) return;
    if (!m_linkedDecos.empty() && m_linkedDecos.front()) m_linkedDecos.front()->SetEnabled(false);
    RegisterForRender(false);
    m_deco->RegisterForRender(false);
    m_deco->RegisterForUpdate(false);
    for (Deco3d *d : m_linkedDecos) {
        d->RegisterForRender(false);
        d->RegisterForUpdate(false);
    }
}
