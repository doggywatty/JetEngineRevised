// core/particles/include/particlesystem.h
#ifndef CORE_PARTICLES_INCLUDE_PARTICLESYSTEM_H
#define CORE_PARTICLES_INCLUDE_PARTICLESYSTEM_H

#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "res/resource.h"
#include "stream/istream.h"
#include "emitter.h"

namespace ps {

// A set of emitters loaded from a .ps file, placed in the world with a position and a rotation. Loaded systems are
// cloned from a base one by ParticleMgr::Load. A system with auto rendering / auto update is drawn / updated by the
// manager every frame (it holds it in its lists); the others by whoever owns them (EffectMgr, the menus).
class ParticleSystem : public jet::res::Resource, public boost::enable_shared_from_this<ParticleSystem> {  // sizeof 0x90
public:
    ParticleSystem();                                  // @0x688a30
    ~ParticleSystem();                                 // @0x688ac8
    boost::shared_ptr<ParticleSystem> Clone() const;   // @0x688b5c
    void RelinkToBase(const ParticleSystem &base);     // @0x688c24
    void SetLodDistance(float d);                      // @0x688c8c
    void Stop(bool immediate);                         // @0x688cc8
    void KillAllParticles();                           // @0x688d14
    bool IsEmitting() const;                           // @0x688d54
    unsigned AddEmitter(Emitter *e);                   // @0x688d9c
    void RemoveEmitter(unsigned i);                    // @0x688dd8
    void Clear();                                      // @0x688e28
    unsigned GetEmitterCount() const { return (unsigned)m_emitters.size(); }   // @0x68a41c
    Emitter &GetEmitter(unsigned i) { return *m_emitters[i]; }
    const Emitter &GetEmitter(unsigned i) const { return *m_emitters[i]; }

    void SetPosition(const math::vec3<float> &p);      // @0x688ea8
    const math::vec3<float> &GetPosition() const { return m_transform.getTranslation(); }   // @0x68a448
    void SetRotation(const math::quat<float> &q);      // @0x688f10
    const math::quat<float> &GetRotation() const { return m_rotation; }   // @0x68a450
    void SetTransform(const math::vec3<float> &p, const math::quat<float> &q);   // @0x688f4c
    const math::mat4<float> &GetTransform() const;     // @0x68a458
    void SetLinearVelocity(const math::vec3<float> &v) { m_linearVelocity = v; }   // @0x6892a4
    const math::vec3<float> &GetLinearVelocity() const { return m_linearVelocity; }
    const unsigned *GetReferenceTransformVersion() const { return &m_transformVersion; }

    void Update();                                     // @0x688fe0: the game's update time
    void Update(unsigned dtMs);                        // @0x689020
    void Render();                                     // @0x6891c0
    void Reset();                                      // @0x689230
    void SetEnabled(bool v) { m_isEnabled = v; }       // @0x68a2a4
    bool IsEnabled() const { return m_isEnabled; }
    void SetDone(bool v) { m_isDone = v; }
    bool IsDone() const { return m_isDone; }
    void SetWarmUpDone(bool v) { m_isWarmUpDone = v; }
    bool IsWarmUpDone() const { return m_isWarmUpDone; }
    void SetAutoRendering(bool v);                     // @0x68a2cc
    bool IsAutoRendering() const { return m_autoRendering; }
    void SetAutoUpdate(bool v);                        // @0x68a374
    bool IsAutoUpdate() const { return m_autoUpdate; }

    // jet::res::Resource
    virtual bool Init(jet::stream::IStream *s) { SetStream(s); return true; }   // @0x6892c0
    virtual bool Load();                               // @0x6892cc
    virtual void Unload() { Clear(); }                 // @0x689408

private:
    bool LoadV100(jet::stream::IStream &s);                                 // @0x689370
    void LoadEmitter(Emitter &e, jet::stream::IStream &s);                  // @0x689410
    void LoadParticleDef(ParticleDef &d, jet::stream::IStream &s);          // @0x68972c
    void LoadShape(Shape &sh, jet::stream::IStream &s);                     // @0x689e8c
    void _UpdateTransform() const;                                          // @0x688e40

    std::vector<Emitter *> m_emitters;                 // 0x18
    math::vec3<float> m_linearVelocity;                // 0x24
    bool m_isEnabled;                                  // 0x30
    bool m_isDone;                                     // 0x31
    bool m_isWarmUpDone;                               // 0x32
    bool m_autoRendering;                              // 0x33
    bool m_autoUpdate;                                 // 0x34
    math::quat<float> m_rotation;                      // 0x38
    mutable math::mat4<float> m_transform;             // 0x48 (the translation is the position)
    unsigned m_transformVersion;                       // 0x88
    mutable unsigned m_cachedTransformVersion;         // 0x8c
};
}

#endif
