// core/particles/src/influence.cpp
#include "influence.h"
#include "emitter.h"
#include "particlemgr.h"
#include "particlesystem.h"
#include "core/util.h"
#include <cmath>

namespace ps {

static const float k_minForce = 0.01f;  // the constants at 0x68391c, 0x683fd8, 0x6843cc, 0x6835b0 and 0x683eb8

// @0x6831ac
Influence::Influence() : m_emitter(0), m_versionPtr(0), m_cachedVersion(~0u) {}

// @0x683224
void Influence::SetEmitter(Emitter *e) {
    m_emitter = e;
    m_versionPtr = e->GetParticleSystem().GetReferenceTransformVersion();
    m_cachedVersion = ~0u;
}

// @0x6827d8
void Influence::UpdateTransform() const {
    if (!m_versionPtr || *m_versionPtr == m_cachedVersion) return;
    m_cachedVersion = *m_versionPtr;
    _UpdateTransform();
}

// the signed force of a gravity / magnet: sign * max(|f|, 0.01), nothing for 0
static float SignedForce(float f) {
    float sign = f < 0 ? -1.0f : (f == 0 ? 0.0f : 1.0f);
    float mag = std::fabs(f);
    if (mag < k_minForce) mag = k_minForce;
    return sign * mag;
}

// the transform of the gravity and wind: the inverse of the system's rotation when the particles are local
static math::mat4<float> LocalSpaceTransform(Emitter *e) {
    if (e->GetParticleDef().GetLocalParticles()) {
        const math::quat<float> &r = e->GetParticleSystem().GetRotation();
        math::quat<float> inv(-r.x, -r.y, -r.z, r.w);
        return math::mat4<float>(inv.getAsMat3());
    }
    return math::mat4<float>();
}

static math::vec3<float> ScaledVector(float force, const math::vec3<float> &v, float dt) {
    return math::vec3<float>((force * v.x) * dt, (force * v.y) * dt, (force * v.z) * dt);
}

// ---- Force

// @0x6836cc
Force::Force() : m_force(0), m_rotation(0, 0, 0, 1), m_worldRotation(0, 0, 0, 1), m_finalRotation(0, 0, 0, 1), m_push(0, 0, 0) { SetForce(1.0f); }
// @0x683860
Influence *Force::Clone(Emitter *e) const {
    Force *f = new Force;
    f->m_force = m_force;
    f->m_rotation = m_rotation;
    f->m_worldRotation = m_worldRotation;
    f->m_finalRotation = m_finalRotation;
    f->m_push = m_push;
    f->SetEmitter(e);
    return f;
}
// @0x683900
void Force::SetForce(float f) { m_force = f >= k_minForce ? f : k_minForce; }
// @0x68375c
void Force::_UpdateTransform() const {
    math::quat<float> psRot = m_emitter->GetParticleSystem().GetRotation();
    m_worldRotation = psRot * m_rotation;
    m_finalRotation = m_emitter->GetParticleDef().GetLocalParticles() ? m_rotation : m_worldRotation;
    math::vec3<float> v = m_finalRotation * ParticleMgr::GetFrontVector();
    m_push = math::vec3<float>(m_force * v.x, m_force * v.y, m_force * v.z);
}
// @0x683680
math::vec3<float> Force::GetFinalForce(float dt) const {
    UpdateTransform();
    return math::vec3<float>(dt * m_push.x, dt * m_push.y, dt * m_push.z);
}
// @0x6837f4
void Force::Apply(std::vector<Particle> &particles, float dt) {
    UpdateTransform();
    math::vec3<float> v(dt * m_push.x, dt * m_push.y, dt * m_push.z);
    for (size_t i = 0; i < particles.size(); i++) particles[i].velocity += v;
}

// ---- Gravity

// @0x6839d0
Gravity::Gravity() : m_force(0) { SetForce(9.81f); }
// @0x683bd4
Influence *Gravity::Clone(Emitter *e) const {
    Gravity *g = new Gravity;
    g->m_force = m_force;
    g->m_transform = m_transform;
    g->SetEmitter(e);
    return g;
}
// @0x683c44
void Gravity::SetForce(float f) { m_force = SignedForce(f); }
// @0x683a2c
void Gravity::_UpdateTransform() const { m_transform = LocalSpaceTransform(m_emitter); }
// @0x683ad4
math::vec3<float> Gravity::GetFinalForce(float dt) const {
    if (m_emitter->GetParticleDef().GetLocalParticles()) {
        UpdateTransform();
        return ScaledVector(m_force, math::transform(m_transform, ParticleMgr::GetGravityVector()), dt);
    }
    return ScaledVector(m_force, ParticleMgr::GetGravityVector(), dt);
}
// @0x683b88
void Gravity::Apply(std::vector<Particle> &particles, float dt) {
    math::vec3<float> v = GetFinalForce(dt);
    for (size_t i = 0; i < particles.size(); i++) particles[i].velocity += v;
}

// ---- Wind

// @0x684140
Wind::Wind() : m_force(0) { SetForce(1.0f); }
// @0x684340
Influence *Wind::Clone(Emitter *e) const {
    Wind *w = new Wind;
    w->m_force = m_force;
    w->m_transform = m_transform;
    w->SetEmitter(e);
    return w;
}
// @0x6843b0
void Wind::SetForce(float f) { m_force = f >= k_minForce ? f : k_minForce; }
// @0x684198
void Wind::_UpdateTransform() const { m_transform = LocalSpaceTransform(m_emitter); }
// @0x684240
math::vec3<float> Wind::GetFinalForce(float dt) const {
    if (m_emitter->GetParticleDef().GetLocalParticles()) {
        UpdateTransform();
        return ScaledVector(m_force, math::transform(m_transform, ParticleMgr::GetWindVector()), dt);
    }
    return ScaledVector(m_force, ParticleMgr::GetWindVector(), dt);
}
// @0x6842f4
void Wind::Apply(std::vector<Particle> &particles, float dt) {
    math::vec3<float> v = GetFinalForce(dt);
    for (size_t i = 0; i < particles.size(); i++) particles[i].velocity += v;
}

// ---- Magnet

// @0x683d48
Magnet::Magnet() : m_force(0), m_maxForce(0), m_position(0, 0, 0), m_worldPosition(0, 0, 0), m_target(0, 0, 0) {
    SetForce(1.0f);
    SetMaxForce(100.0f);
}
// @0x683ebc
Influence *Magnet::Clone(Emitter *e) const {
    Magnet *m = new Magnet;
    m->m_force = m_force;
    m->m_maxForce = m_maxForce;
    m->m_position = m_position;
    m->m_worldPosition = m_worldPosition;
    m->m_target = m_target;
    m->SetEmitter(e);
    return m;
}
// @0x683f80
void Magnet::SetForce(float f) { m_force = SignedForce(f); }
// @0x684074
void Magnet::SetMaxForce(float f) { m_maxForce = f >= 1.0f ? f : 1.0f; }
// @0x683fdc
void Magnet::_UpdateTransform() const {
    ParticleSystem &ps = m_emitter->GetParticleSystem();
    math::quat<float> psRot = ps.GetRotation();
    m_worldPosition = ps.GetPosition() + psRot * m_position;
    m_target = m_emitter->GetParticleDef().GetLocalParticles() ? m_position : m_worldPosition;
}
// @0x683dd0: every particle is pulled toward the magnet with the square of the inverse distance, up to the maximum
void Magnet::Apply(std::vector<Particle> &particles, float dt) {
    const float force = dt * m_force, maxForce = dt * m_maxForce;
    for (size_t i = 0; i < particles.size(); i++) {
        Particle &p = particles[i];
        UpdateTransform();
        math::vec3<float> d(m_target.x - p.position.x, m_target.y - p.position.y, m_target.z - p.position.z);
        float lenSq = d.getLengthSq();
        if (lenSq <= k_minForce) continue;
        float inv = 1.0f / std::sqrt(lenSq);
        float f = inv * inv * force;
        if (f > maxForce) f = maxForce;
        f *= inv;
        p.velocity += math::vec3<float>(f * d.x, f * d.y, f * d.z);
    }
}

// ---- Brownian

// @0x6832e8
Brownian::Brownian() : m_minForce(0), m_maxForce(0), m_minTime(0), m_maxTime(0), m_seed(jet::core::Rand()) {
    SetForceInterval(0.5f, 1.0f);
    SetTimeInterval(500, 1000);
}
// @0x683510
Influence *Brownian::Clone(Emitter *e) const {
    Brownian *b = new Brownian;
    b->m_minForce = m_minForce;
    b->m_maxForce = m_maxForce;
    b->m_minTime = m_minTime;
    b->m_maxTime = m_maxTime;
    b->m_seed = m_seed;
    b->SetEmitter(e);
    b->m_seed = jet::core::Rand();
    return b;
}
// @0x683580
void Brownian::SetForceInterval(float min, float max) {
    float lo = min >= k_minForce ? min : k_minForce;
    m_minForce = lo;
    m_maxForce = lo < max ? max : lo;
}
// @0x6835b4
void Brownian::SetTimeInterval(unsigned min, unsigned max) {
    if (min == 0) min = 1;
    m_minTime = min;
    m_maxTime = min >= max ? min : max;
}
// @0x683354: when a particle's time runs out it gets a random kick (the force interval scaled by the time step) and a
// new random time
void Brownian::Apply(std::vector<Particle> &particles, float dt) {
    const float lo = dt * m_minForce, hi = dt * m_maxForce;
    const short ms = (short)(int)(dt * 1000.0f);
    for (size_t i = 0; i < particles.size(); i++) {
        Particle &p = particles[i];
        int t = (int)p.brownianTime - (int)ms;
        if (t <= 0) {
            const float range = hi - lo;
            t = (int)(unsigned)jet::core::Rand2((int)m_minTime, (int)m_maxTime);
            float a = jet::core::Randf2(m_seed);
            float b = jet::core::Randf2(m_seed);
            float c = jet::core::Randf2(m_seed);
            float s = lo + jet::core::RandPosf2(m_seed) * range;
            p.velocity += math::vec3<float>(s * a, s * b, s * c);
        }
        p.brownianTime = (short)t;
    }
}

}
