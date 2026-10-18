// core/particles/include/influence.h
#ifndef CORE_PARTICLES_INCLUDE_INFLUENCE_H
#define CORE_PARTICLES_INCLUDE_INFLUENCE_H

#include <vector>
#include "smath.h"
#include "particle.h"

namespace ps {
class Emitter;

// What acts on the particles of an emitter every update. The ones that give every particle the same push
// (force, gravity, wind) are batched: the emitter adds their sum to every particle once.
class Influence {
public:
    Influence();                                                   // @0x6831ac
    virtual ~Influence() {}
    virtual Influence *Clone(Emitter *e) const = 0;                // vtable +0x1c
    virtual bool CanBeBatched() const { return false; }            // +0x20
    virtual void Apply(std::vector<Particle> &particles, float dt) = 0;   // +0x18
    virtual math::vec3<float> GetFinalForce(float) const { return math::vec3<float>(0, 0, 0); }  // +0x24 @0x68280c
    virtual void _UpdateTransform() const {}                       // +0x28 @0x6827d4
    void SetEmitter(Emitter *e);                                   // @0x683224
    // @0x6827d8
    void UpdateTransform() const;
protected:
    Emitter *m_emitter;                  // 0x4
    const unsigned *m_versionPtr;        // 0x8
    mutable unsigned m_cachedVersion;    // 0xc
};

class Force : public Influence {  // 0x50
public:
    Force();                                                       // @0x6836cc
    Influence *Clone(Emitter *e) const override;                   // @0x683860
    bool CanBeBatched() const override { return true; }
    void Apply(std::vector<Particle> &particles, float dt) override;   // @0x6837f4
    math::vec3<float> GetFinalForce(float dt) const override;      // @0x683680
    void _UpdateTransform() const override;                        // @0x68375c
    void SetForce(float f);                                        // @0x683900
    void SetRotation(const math::quat<float> &q) { m_rotation = q; m_cachedVersion = ~0u; }   // @0x6838e4
private:
    float m_force;                                // 0x10
    math::quat<float> m_rotation;                 // 0x14
    mutable math::quat<float> m_worldRotation;    // 0x24
    mutable math::quat<float> m_finalRotation;    // 0x34
    mutable math::vec3<float> m_push;             // 0x44
};

class Gravity : public Influence {  // 0x54
public:
    Gravity();                                                     // @0x6839d0
    Influence *Clone(Emitter *e) const override;                   // @0x683bd4
    bool CanBeBatched() const override { return true; }
    void Apply(std::vector<Particle> &particles, float dt) override;   // @0x683b88
    math::vec3<float> GetFinalForce(float dt) const override;      // @0x683ad4
    void _UpdateTransform() const override;                        // @0x683a2c
    void SetForce(float f);                                        // @0x683c44
private:
    float m_force;                                // 0x10
    mutable math::mat4<float> m_transform;        // 0x14
};

class Wind : public Influence {  // 0x54
public:
    Wind();                                                        // @0x684140
    Influence *Clone(Emitter *e) const override;                   // @0x684340
    bool CanBeBatched() const override { return true; }
    void Apply(std::vector<Particle> &particles, float dt) override;   // @0x6842f4
    math::vec3<float> GetFinalForce(float dt) const override;      // @0x684240
    void _UpdateTransform() const override;                        // @0x684198
    void SetForce(float f);                                        // @0x6843b0
private:
    float m_force;                                // 0x10
    mutable math::mat4<float> m_transform;        // 0x14
};

class Magnet : public Influence {  // 0x3c
public:
    Magnet();                                                      // @0x683d48
    Influence *Clone(Emitter *e) const override;                   // @0x683ebc
    void Apply(std::vector<Particle> &particles, float dt) override;   // @0x683dd0
    void _UpdateTransform() const override;                        // @0x683fdc
    void SetForce(float f);                                        // @0x683f80
    void SetMaxForce(float f);                                     // @0x684074
    void SetPosition(const math::vec3<float> &p) { m_position = p; m_cachedVersion = ~0u; }   // @0x683f54
    const math::vec3<float> &GetPosition() const { return m_position; }
private:
    float m_force, m_maxForce;                    // 0x10 0x14
    math::vec3<float> m_position;                 // 0x18
    mutable math::vec3<float> m_worldPosition;    // 0x24
    mutable math::vec3<float> m_target;           // 0x30
};

class Brownian : public Influence {  // 0x24
public:
    Brownian();                                                    // @0x6832e8
    Influence *Clone(Emitter *e) const override;                   // @0x683510
    void Apply(std::vector<Particle> &particles, float dt) override;   // @0x683354
    void SetForceInterval(float min, float max);                   // @0x683580
    void SetTimeInterval(unsigned min, unsigned max);              // @0x6835b4
private:
    float m_minForce, m_maxForce;                 // 0x10 0x14
    unsigned m_minTime, m_maxTime;                // 0x18 0x1c ms between two kicks
    unsigned m_seed;                              // 0x20
};
}

#endif
