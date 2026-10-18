// core/particles/include/emitter.h
#ifndef CORE_PARTICLES_INCLUDE_EMITTER_H
#define CORE_PARTICLES_INCLUDE_EMITTER_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "particle.h"
#include "particledef.h"
#include "shape.h"
#include "influence.h"

namespace ps {
class ParticleSystem;

// One stream of particles of a system: its definition, its shape, its influences and the live particles (0x48 bytes)
class Emitter {
public:
    Emitter();                                              // @0x682838
    ~Emitter();                                             // @0x682a20
    Emitter *Clone(ParticleSystem *ps) const;               // @0x682a94
    ParticleSystem &GetParticleSystem() const { return *m_particleSystem; }
    void SetParticleSystem(ParticleSystem *ps) { m_particleSystem = ps; }   // @0x683190
    void SetParticleDef(const boost::shared_ptr<ParticleDef> &d) { m_particleDef = d; }   // @0x6830f4
    const ParticleDef &GetParticleDef() const { return *m_particleDef; }
    ParticleDef &GetParticleDef() { return *m_particleDef; }
    const boost::shared_ptr<ParticleDef> &GetParticleDefPtr() const { return m_particleDef; }
    void SetShape(Shape *s);                                // @0x6830bc
    Shape *GetShape() const { return m_shape; }
    void AddInfluence(Influence *i);                        // @0x683010
    void RemoveInfluence(Influence *i);                     // @0x682f90
    void RemoveAllInfluences();                             // @0x682fec
    void SetVisible(bool v) { m_visible = v; }              // @0x6830b4
    bool IsVisible() const { return m_visible; }
    void SetCameraDistance(float d);                        // @0x68313c
    float GetCameraDistance() const { return m_cameraDistance; }
    void SetLodDistance(float d);                           // @0x68316c
    float GetLodDistance() const { return m_lodDistance; }
    float GetLodFactor() const { return m_lodFactor; }
    std::vector<Particle> &GetAllParticles() { return m_particles; }
    const std::vector<Particle> &GetAllParticles() const { return m_particles; }
    // @0x682924: a particle initialized from the definition (the shape fills its place and velocity in)
    unsigned Spawn();
    void Update(float dt, unsigned dtMs);                   // @0x682bf8
    void Stop() { if (m_shape) m_shape->SetDone(true); }    // @0x683198
    void Reset();                                           // @0x682f60
    void Clear();                                           // @0x682f00
    void KillAllParticles() { m_particles.clear(); }        // @0x682ebc
    void DelayedKillAllParticles();                         // @0x682ec8

private:
    ParticleSystem *m_particleSystem;                       // 0x0
    boost::shared_ptr<ParticleDef> m_particleDef;           // 0x4
    Shape *m_shape;                                         // 0xc
    std::vector<boost::shared_ptr<Influence> > m_batchedInfluences;   // 0x10
    std::vector<boost::shared_ptr<Influence> > m_influences;          // 0x1c
    bool m_visible;                                         // 0x28
    std::vector<Particle> m_particles;                      // 0x2c
    float m_cameraDistance;                                 // 0x38
    float m_lodDistance;                                    // 0x3c
    float m_invLodDistance;                                 // 0x40
    float m_lodFactor;                                      // 0x44
};
}

#endif
