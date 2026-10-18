// core/particles/src/shape.cpp
#include "shape.h"
#include "emitter.h"
#include "particlemgr.h"
#include "particlesystem.h"
#include "core/util.h"
#include <cstring>

namespace ps {

using jet::core::Randf2;
using jet::core::RandPosf2;

static float FloatBits(unsigned bits) {
    float f;
    memcpy(&f, &bits, 4);
    return f;
}
// the angle from which a disk / sphere is whole (the .so compares with 3.0916 at 0x68b24c / 0x68bcb8)
static const unsigned k_fullAngleBits = 0x4045dca8;

// @0x68a480
Shape::Shape()
    : m_emitter(0), m_duration(0), m_warmUpDuration(0), m_life(0), m_emitStart(0), m_emitEnd(0), m_emitRate(0),
      m_position(0, 0, 0), m_worldPosition(0, 0, 0), m_spawnPosition(0, 0, 0), m_particleCount(100.0f), m_speed(1.0f),
      m_emitSource(0), m_done(false), m_looped(false), m_versionPtr(0), m_cachedVersion(~0u) {}

// @0x68a514
Shape::Shape(const Shape &o)
    : m_emitter(0), m_duration(o.m_duration), m_warmUpDuration(0), m_life(o.m_life), m_emitStart(o.m_emitStart),
      m_emitEnd(o.m_emitEnd), m_emitRate(o.m_emitRate), m_position(o.m_position), m_worldPosition(0, 0, 0),
      m_spawnPosition(0, 0, 0), m_particleCount(o.m_particleCount), m_speed(o.m_speed), m_emitSource(o.m_emitSource),
      m_done(o.m_done), m_looped(o.m_looped), m_versionPtr(0), m_cachedVersion(o.m_cachedVersion) {
}

// @0x68a600
void Shape::SetEmitter(Emitter *e) {
    m_emitter = e;
    m_versionPtr = e->GetParticleSystem().GetReferenceTransformVersion();
    m_cachedVersion = ~0u;
}

// @0x6826cc
void Shape::UpdateTransform() const {
    if (!m_versionPtr || *m_versionPtr == m_cachedVersion) return;
    m_cachedVersion = *m_versionPtr;
    _UpdateTransform();
}

// @0x68a6fc: the position in the world, and where particles are placed (that one stays local for local particles)
void Shape::_UpdateTransform() const {
    ParticleSystem &ps = m_emitter->GetParticleSystem();
    math::quat<float> rot = ps.GetRotation();
    math::vec3<float> world = ps.GetPosition() + rot * m_position;
    m_worldPosition = world;
    if (!m_emitter->GetParticleDef().GetLocalParticles()) m_spawnPosition = world;
    else m_spawnPosition = m_position;
}

// @0x68a644
void Shape::SetEmitInterval(unsigned start, unsigned end) {
    unsigned lo = start < m_duration ? start : m_duration;
    if (lo > end) lo = end;
    unsigned hi = end < start ? start : end;
    if (hi > m_duration) hi = m_duration;
    m_emitEnd = hi;
    m_emitStart = lo;
    int span = (int)(hi - lo);
    if (span < 0) span = 0;
    m_emitRate = 1000.0f / (float)span;
}

// @0x6826f8
float Shape::GetParticlesToEmit(float dt, float lodFactor) const {
    if (m_life < m_emitStart || m_life >= m_emitEnd) return 0.0f;
    float n = m_particleCount;
    if (m_emitStart == m_emitEnd) return lodFactor * n;
    float r = n * m_emitRate * dt * lodFactor;
    return r > n ? n : r;
}

OrientedShape::OrientedShape()
    : m_accumulator(0), m_emitted(0), m_seed(jet::core::Rand()), m_localRotation(0, 0, 0, 1), m_worldRotation(0, 0, 0, 1),
      m_shapeRotation(0, 0, 0, 1) {}

OrientedShape::OrientedShape(const OrientedShape &o)
    : Shape(o), m_accumulator(0), m_emitted(0), m_seed(jet::core::Rand()), m_localRotation(o.m_localRotation),
      m_worldRotation(o.m_worldRotation), m_shapeRotation(o.m_shapeRotation) {}

// the rotation of the system times the shape's, and the one the particles use (the shape's own for local particles)
void OrientedShape::UpdateRotations() const {
    math::quat<float> psRot = m_emitter->GetParticleSystem().GetRotation();
    m_worldRotation = psRot * m_localRotation;
    m_shapeRotation = m_emitter->GetParticleDef().GetLocalParticles() ? m_localRotation : m_worldRotation;
}

// the start of the four Update: time, looping and how many particles are to spawn now
unsigned OrientedShape::BeginUpdate(float dt, unsigned dtMs) {
    if (m_done) return 0;
    m_accumulator += GetParticlesToEmit(dt, m_emitter->GetLodFactor());
    SetLife(m_life + dtMs);
    if (m_life >= m_duration) {
        if (m_looped) SetLife(0);
        else SetDone(true);
    }
    if (m_accumulator < 1.0f) return 0;
    unsigned n = (unsigned)m_accumulator;
    m_accumulator -= (float)n;
    m_emitted += n;
    return n;
}

// the acceleration every spawned particle gets: the system's velocity times the definition's factor
math::vec3<float> OrientedShape::SpawnAcceleration() const {
    float k = m_emitter->GetParticleDef().m_linearMomentumFactor;
    const math::vec3<float> &v = m_emitter->GetParticleSystem().GetLinearVelocity();
    return math::vec3<float>(k * v.x, k * v.y, k * v.z);
}

// what ends the spawn of a particle: the seed advances (the LCG @0x41a7) and is the particle's own
void OrientedShape::FinishParticle(Particle &p, const math::vec3<float> &pos, const math::vec3<float> &vel, const math::vec3<float> &accel) {
    p.position = pos;
    p.velocity = vel;
    p.acceleration = accel;
    m_seed *= 0x41a7;
    p.seed = m_seed;
}

// ---- Disk

// @0x68acc4
Disk::Disk() : m_radius(0), m_angle(FloatBits(0x40490fdb)), m_front(ParticleMgr::GetFrontVector()), m_up(ParticleMgr::GetUpVector()) {}
// @0x68ad9c
Disk::Disk(const Disk &o) : OrientedShape(o), m_radius(o.m_radius), m_angle(o.m_angle), m_front(ParticleMgr::GetFrontVector()), m_up(ParticleMgr::GetUpVector()) {}
// @0x68b25c
Shape *Disk::Clone() const { return new Disk(*this); }

// @0x68aebc
void Disk::_UpdateTransform() const {
    Shape::_UpdateTransform();
    UpdateRotations();
}

// @0x68af1c: on the circle (or at a random radius in the disk) with the velocity out of it (or along its normal)
void Disk::Update(float dt, unsigned dtMs) {
    unsigned n = BeginUpdate(dt, dtMs);
    if (!n) return;
    UpdateTransform();
    math::vec3<float> normal = m_shapeRotation * m_up;
    const math::vec3<float> accel = SpawnAcceleration();
    for (unsigned i = 0; i < n; i++) {
        unsigned idx = m_emitter->Spawn();
        Particle &p = m_emitter->GetAllParticles()[idx];
        math::vec3<float> dir;
        if (m_angle >= FloatBits(k_fullAngleBits)) {
            float a = Randf2(m_seed);
            float b = Randf2(m_seed);
            dir = math::vec3<float>(a, b, 0.0f);
            dir.normalize();
            UpdateTransform();
            dir = m_shapeRotation * dir;
        } else {
            math::quat<float> q;
            q.setFromAngleAxis(Randf2(m_seed) * m_angle, m_up);
            UpdateTransform();
            math::quat<float> total = m_shapeRotation * q;
            dir = total * m_front;
        }
        UpdateTransform();
        math::vec3<float> pos, vel;
        if (m_emitSource == 0) {
            pos = m_spawnPosition + math::vec3<float>(m_radius * dir.x, m_radius * dir.y, m_radius * dir.z);
            vel = math::vec3<float>(m_speed * dir.x, m_speed * dir.y, m_speed * dir.z);
        } else {
            UpdateTransform();
            float r = RandPosf2(m_seed) * m_radius;
            pos = m_spawnPosition + math::vec3<float>(r * dir.x, r * dir.y, r * dir.z);
            vel = math::vec3<float>(m_speed * normal.x, m_speed * normal.y, m_speed * normal.z);
        }
        FinishParticle(p, pos, vel, accel);
    }
}

// ---- Sphere

// @0x68b78c
Sphere::Sphere() : m_radius(0), m_angle(FloatBits(0x40490fdb)), m_front(ParticleMgr::GetFrontVector()), m_up(ParticleMgr::GetUpVector()) {}
// @0x68b864
Sphere::Sphere(const Sphere &o) : OrientedShape(o), m_radius(o.m_radius), m_angle(o.m_angle), m_front(ParticleMgr::GetFrontVector()), m_up(ParticleMgr::GetUpVector()) {}
// @0x68bcc0
Shape *Sphere::Clone() const { return new Sphere(*this); }

// @0x68b98c
void Sphere::_UpdateTransform() const {
    UpdateRotations();
    Shape::_UpdateTransform();
}

// @0x68b9f0: on the sphere (or in it), the whole one or the cap an angle wide, moving away from the center
void Sphere::Update(float dt, unsigned dtMs) {
    unsigned n = BeginUpdate(dt, dtMs);
    if (!n) return;
    const math::vec3<float> accel = SpawnAcceleration();
    for (unsigned i = 0; i < n; i++) {
        unsigned idx = m_emitter->Spawn();
        Particle &p = m_emitter->GetAllParticles()[idx];
        math::vec3<float> dir;
        if (m_angle >= FloatBits(k_fullAngleBits)) {
            float a = Randf2(m_seed);
            float b = Randf2(m_seed);
            float c = Randf2(m_seed);
            dir = math::vec3<float>(a, b, c);
            dir.normalize();
        } else {
            math::quat<float> q1, q2;
            q1.setFromAngleAxis(Randf2(m_seed) * FloatBits(0x40490fdb), m_front);
            q2.setFromAngleAxis(Randf2(m_seed) * m_angle, m_up);
            UpdateTransform();
            math::quat<float> total = (m_shapeRotation * q1) * q2;
            dir = total * m_front;
        }
        float r = m_emitSource == 0 ? m_radius : RandPosf2(m_seed) * m_radius;
        UpdateTransform();
        math::vec3<float> pos = m_spawnPosition + math::vec3<float>(r * dir.x, r * dir.y, r * dir.z);
        math::vec3<float> vel(m_speed * dir.x, m_speed * dir.y, m_speed * dir.z);
        FinishParticle(p, pos, vel, accel);
    }
}

// ---- the ray of Box and Rectangle (math::rayIntersectAABB<float> @0x68cdd4): the slabs of the box center +- half
// along the ray from o; false when it misses
static bool RayIntersectBox(const math::vec3<float> &o, const math::vec3<float> &dir, const math::vec3<float> &center,
                            const math::vec3<float> &half, float &t0, float &t1) {
    const math::vec3<float> bounds[2] = { center - half, center + half };
    const float inv[3] = { 1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z };
    const float oc[3] = { o.x, o.y, o.z };
    float lo = 0, hi = 0;
    for (int a = 0; a < 3; a++) {
        int s = inv[a] < 0 ? 1 : 0;
        float n = ((&bounds[s].x)[a] - oc[a]) * inv[a];
        float f = ((&bounds[1 - s].x)[a] - oc[a]) * inv[a];
        if (a == 0) { lo = n; hi = f; continue; }
        if (lo > f) return false;
        if (n > hi) return false;
        if (n > lo) lo = n;
        if (f < hi) hi = f;
    }
    t0 = lo;
    t1 = hi;
    return true;
}

// ---- Box

// @0x68a7a4
Box::Box() : m_size(1, 1, 1) {}
// @0x68a828
Box::Box(const Box &o) : OrientedShape(o), m_size(o.m_size) {}
// @0x68ac90
Shape *Box::Clone() const { return new Box(*this); }

// @0x68a920
void Box::_UpdateTransform() const {
    Shape::_UpdateTransform();
    UpdateRotations();
}

// @0x68a980: inside the box, or on its surface where the ray from the center through the point leaves it
void Box::Update(float dt, unsigned dtMs) {
    unsigned n = BeginUpdate(dt, dtMs);
    if (!n) return;
    const math::vec3<float> accel = SpawnAcceleration();
    UpdateTransform();
    const math::vec3<float> half(m_size.x * 0.5f, m_size.y * 0.5f, m_size.z * 0.5f);
    const math::vec3<float> zero(0, 0, 0);
    for (unsigned i = 0; i < n; i++) {
        unsigned idx = m_emitter->Spawn();
        Particle &p = m_emitter->GetAllParticles()[idx];
        float x = RandPosf2(m_seed) * (half.x + half.x) - half.x;
        float y = RandPosf2(m_seed) * (half.y + half.y) - half.y;
        float z = RandPosf2(m_seed) * (half.z + half.z) - half.z;
        math::vec3<float> point(x, y, z);
        math::vec3<float> dir = math::normalizedFast(point);
        math::vec3<float> pos, vel;
        if (m_emitSource == 0) {
            float t0 = 0, t1 = 0;
            RayIntersectBox(zero, dir, zero, half, t0, t1);
            UpdateTransform();
            math::vec3<float> rd = m_shapeRotation * dir;
            pos = m_spawnPosition + math::vec3<float>(t0 * rd.x, t0 * rd.y, t0 * rd.z);
            float s = -m_speed;
            vel = math::vec3<float>(s * rd.x, s * rd.y, s * rd.z);
        } else {
            UpdateTransform();
            math::vec3<float> rp = m_shapeRotation * point;
            pos = m_spawnPosition + rp;
            math::vec3<float> rd = m_shapeRotation * dir;
            vel = math::vec3<float>(m_speed * rd.x, m_speed * rd.y, m_speed * rd.z);
        }
        FinishParticle(p, pos, vel, accel);
    }
}

// ---- Rectangle

// @0x68b290
Rectangle::Rectangle() : m_size(1, 1) {}
// @0x68b310
Rectangle::Rectangle(const Rectangle &o) : OrientedShape(o), m_size(o.m_size) {}
// @0x68b758
Shape *Rectangle::Clone() const { return new Rectangle(*this); }

// @0x68b3f4
void Rectangle::_UpdateTransform() const {
    Shape::_UpdateTransform();
    UpdateRotations();
}

// @0x68b454: as the box, in the plane; the particles of the filled rectangle move along its normal
void Rectangle::Update(float dt, unsigned dtMs) {
    unsigned n = BeginUpdate(dt, dtMs);
    if (!n) return;
    const math::vec3<float> accel = SpawnAcceleration();
    UpdateTransform();
    math::vec3<float> normal = m_shapeRotation * ParticleMgr::GetUpVector();
    const math::vec3<float> half(m_size.x * 0.5f, m_size.y * 0.5f, 0.0f);
    const math::vec3<float> zero(0, 0, 0);
    for (unsigned i = 0; i < n; i++) {
        unsigned idx = m_emitter->Spawn();
        Particle &p = m_emitter->GetAllParticles()[idx];
        float x = RandPosf2(m_seed) * (half.x + half.x) - half.x;
        float y = RandPosf2(m_seed) * (half.y + half.y) - half.y;
        math::vec3<float> point(x, y, 0.0f);
        math::vec3<float> pos, vel;
        if (m_emitSource == 0) {
            point.normalize();
            float t0 = 0, t1 = 0;
            RayIntersectBox(zero, point, zero, half, t0, t1);
            UpdateTransform();
            math::vec3<float> rd = m_shapeRotation * point;
            pos = m_spawnPosition + math::vec3<float>(t0 * rd.x, t0 * rd.y, t0 * rd.z);
            float s = -m_speed;
            vel = math::vec3<float>(s * rd.x, s * rd.y, s * rd.z);
        } else {
            UpdateTransform();
            pos = m_spawnPosition + m_shapeRotation * point;
            vel = math::vec3<float>(m_speed * normal.x, m_speed * normal.y, m_speed * normal.z);
        }
        FinishParticle(p, pos, vel, accel);
    }
}

}
