// core/particles/include/shape.h
#ifndef CORE_PARTICLES_INCLUDE_SHAPE_H
#define CORE_PARTICLES_INCLUDE_SHAPE_H

#include "smath.h"
#include "jtypes.h"
#include "particle.h"

namespace ps {
class Emitter;

// Where and how an emitter spawns its particles. The four shapes (disk, sphere, box, rectangle) differ only in
// Update (and the sizes they hold); the rest is here.
class Shape {
public:
    struct EmitSourceDef { enum type { Surface, Volume }; };

    Shape();                        // @0x68a480
    Shape(const Shape &o);          // @0x68a514 (not the emitter, the life nor the emitted particles)
    virtual ~Shape() {}
    virtual Shape *Clone() const { return 0; }                     // vtable +0x10
    virtual void Update(float dt, unsigned dtMs) = 0;              // vtable +0x14
    virtual void _UpdateTransform() const;                         // vtable +0x18 @0x68a6fc

    void SetEmitter(Emitter *e);                                   // @0x68a600
    // @0x6826cc: refreshed when the particle system moved
    void UpdateTransform() const;
    // @0x6826f8: how many particles to emit for dt (and the emitter's lod factor)
    float GetParticlesToEmit(float dt, float lodFactor) const;
    void Reset() { SetLife(0); SetDone(false); }                   // @0x68a624
    void SetDone(bool v) { m_done = v; }
    bool IsDone() const { return m_done; }
    void SetLife(unsigned v) { m_life = v; }
    unsigned GetLife() const { return m_life; }
    void SetDuration(unsigned v) { m_duration = v; }
    void SetWarmUpDuration(unsigned v) { m_warmUpDuration = v; }
    unsigned GetWarmUpDuration() const { return m_warmUpDuration; }
    void SetEmitInterval(unsigned start, unsigned end);            // @0x68a644
    void SetLooped(bool v) { m_looped = v; }
    void SetPosition(const math::vec3<float> &p) { m_position = p; m_cachedVersion = ~0u; }
    void SetParticleCount(unsigned n) { m_particleCount = n ? (float)n : 1.0f; }
    void SetSpeed(float s) { m_speed = s; }
    void SetEmitSource(unsigned s) { m_emitSource = s; }
    const math::vec3<float> &GetWorldPosition() const { return m_worldPosition; }

protected:
    Emitter *m_emitter;                  // 0x4
    unsigned m_duration;                 // 0x8
    unsigned m_warmUpDuration;           // 0xc
    mutable unsigned m_life;             // 0x10
    unsigned m_emitStart, m_emitEnd;     // 0x14 0x18
    float m_emitRate;                    // 0x1c 1 / (end - start)
    math::vec3<float> m_position;        // 0x20 relative to the system
    mutable math::vec3<float> m_worldPosition;   // 0x2c
    mutable math::vec3<float> m_spawnPosition;   // 0x38 the world position, or the local one for local particles
    float m_particleCount;               // 0x44
    float m_speed;                       // 0x48
    unsigned m_emitSource;               // 0x4c
    bool m_done;                         // 0x50
    bool m_looped;                       // 0x52
    const unsigned *m_versionPtr;        // 0x54 the particle system's transform version
    mutable unsigned m_cachedVersion;    // 0x58
    friend class Emitter;
};

// Common state of the shapes that spawn along a rotated frame (0x5c on)
class OrientedShape : public Shape {
public:
    void SetRotation(const math::quat<float> &q) { m_localRotation = q; m_cachedVersion = ~0u; }
protected:
    OrientedShape();
    OrientedShape(const OrientedShape &o);
    // the accumulated fraction of a particle, the count emitted so far, the LCG, the rotations (0x70 local, 0x80 with
    // the system's, 0x90 the one used to place the particles)
    float m_accumulator;                 // 0x5c
    unsigned m_emitted;                  // 0x60
    unsigned m_seed;                     // 0x6c / 0x70 (box)
    math::quat<float> m_localRotation;
    mutable math::quat<float> m_worldRotation, m_shapeRotation;
    void UpdateRotations() const;        // the part of _UpdateTransform of the four
    unsigned BeginUpdate(float dt, unsigned dtMs);
    math::vec3<float> SpawnAcceleration() const;
    void FinishParticle(Particle &p, const math::vec3<float> &pos, const math::vec3<float> &vel, const math::vec3<float> &accel);
    friend class Emitter;
};

class Disk : public OrientedShape {  // sizeof 0xb8
public:
    Disk();                                       // @0x68acc4
    Disk(const Disk &o);                          // @0x68ad9c
    Shape *Clone() const override;                // @0x68b25c
    void Update(float dt, unsigned dtMs) override;    // @0x68af1c
    void _UpdateTransform() const override;       // @0x68aebc
    void SetRadius(float r) { m_radius = r; }
    void SetAngle(float a) { m_angle = a; }
private:
    float m_radius, m_angle;                      // 0x64 0x68
    math::vec3<float> m_front, m_up;              // 0xa0 0xac
};

class Sphere : public OrientedShape {  // sizeof 0xb8
public:
    Sphere();                                     // @0x68b78c
    Sphere(const Sphere &o);                      // @0x68b864
    Shape *Clone() const override;                // @0x68bcc0
    void Update(float dt, unsigned dtMs) override;    // @0x68b9f0
    void _UpdateTransform() const override;       // @0x68b98c
    void SetRadius(float r) { m_radius = r; }
    void SetAngle(float a) { m_angle = a; }
private:
    float m_radius, m_angle;                      // 0x64 0x68
    math::vec3<float> m_front, m_up;              // 0xa0 0xac
};

class Box : public OrientedShape {  // sizeof 0xa4
public:
    Box();                                        // @0x68a7a4
    Box(const Box &o);                            // @0x68a828
    Shape *Clone() const override;                // @0x68ac90
    void Update(float dt, unsigned dtMs) override;    // @0x68a980
    void _UpdateTransform() const override;       // @0x68a920
    void SetSize(const math::vec3<float> &s) { m_size = s; }
private:
    math::vec3<float> m_size;                     // 0x64
};

class Rectangle : public OrientedShape {  // sizeof 0xa0
public:
    Rectangle();                                  // @0x68b290
    Rectangle(const Rectangle &o);                // @0x68b310
    Shape *Clone() const override;                // @0x68b758
    void Update(float dt, unsigned dtMs) override;    // @0x68b454
    void _UpdateTransform() const override;       // @0x68b3f4
    void SetSize(const math::vec2<float> &s) { m_size = s; }
private:
    math::vec2<float> m_size;                     // 0x64
};
}

#endif
