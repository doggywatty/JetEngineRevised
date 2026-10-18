// source/game/entities/ParticleSystemEntity.cpp
#include "ParticleSystemEntity.h"
#include "particlemgr.h"
#include "../Game.h"

jet::String ParticleSystemEntity::k_tmplName("ParticleSystem");

// @0x4f7674
void ParticleSystemEntity::Init() {
    if (!(m_flags & F_INIT)) clara::Entity::Init();
}

// @0x4f7678: the system is loaded unless this is a library template
void ParticleSystemEntity::PostInit() {
    GameEntity::PostInit();
    if (m_flags & F_LIBRARY) return;
    jet::String name;
    GetParam(jet::String("Template"), name);
    m_ps = ps::ParticleMgr::GetInstance()->Load(name);
    if (!m_ps) return;
    m_ps->Reset();
    m_ps->SetAutoRendering(false);
    m_ps->SetAutoUpdate(false);
    m_ps->SetEnabled(false);
    m_ps->SetTransform(GetPosition(), GetRotation());
    RegisterForUpdate(true);
}

// @0x4f77c0
void ParticleSystemEntity::SetVisible(bool v) {
    GameEntity::SetVisible(v);
    if (m_ps) m_ps->SetAutoRendering((m_flags & F_ENABLED) && v);
}

// @0x4f7800
void ParticleSystemEntity::SetEnabled(bool e) {
    if (e || m_isOnDeserialize) GameEntity::SetEnabled(e);
    if (!(m_flags & F_VISIBLE)) e = false;
    if (!m_ps) return;
    if (e) {
        m_ps->Reset();
        m_ps->SetAutoUpdate(true);
        return;
    }
    m_ps->Stop(false);
    m_ps->SetAutoUpdate(false);
    m_ps->SetEnabled(false);
}

// @0x4f7880: it takes the velocity of the entity
void ParticleSystemEntity::Update() {
    GameEntity::Update();
    if (!m_ps) return;
    math::vec3<float> p = GetPosition();
    float dt = Game::GetInstance()->GetUpdateTimeAsFloat();
    if (dt > 0.0f) {
        math::vec3<float> v = (p - m_oldPos) * (1000.0f / dt);
        m_ps->SetLinearVelocity(v);
    }
    m_oldPos = p;
    if (m_ps->IsEnabled()) m_ps->Update();
    if (m_ps->IsDone()) GameEntity::SetEnabled(false);
}

// @0x4f79a8
void ParticleSystemEntity::NotifyTransformChanged(bool) {
    if (m_ps) m_ps->SetTransform(GetPosition(), GetRotation());
}
