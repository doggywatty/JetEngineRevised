// core/particles/src/emitter.cpp
#include "emitter.h"
#include "particlesystem.h"

namespace ps {

// @0x682838: visible, with room for 8 particles
Emitter::Emitter()
    : m_particleSystem(0), m_shape(0), m_visible(true), m_cameraDistance(0), m_lodDistance(0), m_invLodDistance(0), m_lodFactor(1.0f) {
    SetLodDistance(100.0f);
    m_particles.reserve(8);
}

// @0x682a20
Emitter::~Emitter() {
    RemoveAllInfluences();
    KillAllParticles();
    delete m_shape;
}

// @0x682a94: the definition is shared, the shape and the influences are copies
Emitter *Emitter::Clone(ParticleSystem *ps) const {
    Emitter *e = new Emitter;
    e->SetParticleSystem(ps);
    e->SetLodDistance(m_lodDistance);
    e->SetParticleDef(m_particleDef);
    e->SetShape(m_shape ? m_shape->Clone() : 0);
    for (size_t i = 0; i < m_influences.size(); i++) e->AddInfluence(m_influences[i]->Clone(e));
    e->SetVisible(m_visible);
    return e;
}

// @0x6830bc
void Emitter::SetShape(Shape *s) {
    if (s != m_shape) {
        delete m_shape;
        m_shape = s;
    }
    if (s) s->SetEmitter(this);
}

// @0x683010: the ones that can be batched are summed once for all the particles
void Emitter::AddInfluence(Influence *i) {
    boost::shared_ptr<Influence> sp(i);
    i->SetEmitter(this);
    if (i->CanBeBatched()) m_batchedInfluences.push_back(sp);
    m_influences.push_back(sp);
}

// @0x682f90
void Emitter::RemoveInfluence(Influence *i) {
    for (size_t k = 0; k < m_influences.size(); k++)
        if (m_influences[k].get() == i) { m_influences.erase(m_influences.begin() + k); break; }
    for (size_t k = 0; k < m_batchedInfluences.size(); k++)
        if (m_batchedInfluences[k].get() == i) { m_batchedInfluences.erase(m_batchedInfluences.begin() + k); break; }
}

// @0x682fec
void Emitter::RemoveAllInfluences() {
    m_influences.clear();
    m_batchedInfluences.clear();
}

// @0x68313c: farther than the lod distance, fewer particles
void Emitter::SetCameraDistance(float d) {
    m_cameraDistance = d;
    m_lodFactor = d > m_lodDistance ? 1.0f / (d * m_invLodDistance) : 1.0f;
}

// @0x68316c
void Emitter::SetLodDistance(float d) {
    if (d < 1.0f) d = 1.0f;
    m_lodDistance = d;
    m_invLodDistance = 1.0f / d;
}

// @0x682924
unsigned Emitter::Spawn() {
    Particle p;
    m_particleDef->Init(p);
    m_particles.push_back(p);
    return (unsigned)m_particles.size() - 1;
}

// @0x682bf8: the shape spawns, the influences push, the particles age and the ones that lived their time are
// replaced by the last one
void Emitter::Update(float dt, unsigned dtMs) {
    m_shape->Update(dt, dtMs);
    math::vec3<float> batched(0, 0, 0);
    for (size_t i = 0; i < m_batchedInfluences.size(); i++) batched += m_batchedInfluences[i]->GetFinalForce(dt);
    for (size_t i = 0; i < m_influences.size(); i++)
        if (!m_influences[i]->CanBeBatched()) m_influences[i]->Apply(m_particles, dt);
    for (size_t i = 0; i < m_particles.size();) {
        Particle &p = m_particles[i];
        p.velocity += batched;
        p.Update(*m_particleDef, dt, dtMs);
        if (p.life < p.duration) { i++; continue; }
        std::swap(m_particles[i], m_particles.back());
        m_particles.pop_back();
    }
}

// @0x682ec8: every particle has one update left
void Emitter::DelayedKillAllParticles() {
    for (size_t i = 0; i < m_particles.size(); i++) {
        int d = m_particles[i].duration - 1;
        m_particles[i].life = d < 0 ? 0 : d;
    }
}

// @0x682f60
void Emitter::Reset() {
    if (m_shape) m_shape->Reset();
    m_particles.clear();
}

// @0x682f00
void Emitter::Clear() {
    KillAllParticles();
    RemoveAllInfluences();
    m_particleDef.reset();
    delete m_shape;
    m_shape = 0;
}

}
