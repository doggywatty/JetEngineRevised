// core/particles/src/particle.cpp
#include "particle.h"
#include "particledef.h"
#include "core/util.h"
#include <cstring>

namespace ps {

static float FloatBits(unsigned bits) {
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

// @0x682498
Particle::Particle()
    : life(0), duration(0), position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0), seed(0), rotationFP(0),
      rotationSign(0), brownianTime(0) {
    for (int i = 0; i < 6; i++) axes[i] = 0;
}

// @0x6824f0: the position moves by the frame's speed scale times the velocity plus the acceleration, the velocity
// is scaled by the frame's drag; the rotation advances by the frame's rotation speed (or one of the particle's own
// when the definition varies it)
void Particle::Update(const ParticleDef &def, float dt, unsigned dtMs) {
    const ParticleDefFrame &frame = def.GetFrame((unsigned)life >> 6);
    math::vec3<float> scaled(frame.speedScale * velocity.x, frame.speedScale * velocity.y, frame.speedScale * velocity.z);
    math::vec3<float> total = scaled + acceleration;
    position += math::vec3<float>(dt * total.x, dt * total.y, dt * total.z);
    velocity *= frame.drag;
    unsigned step;
    int dir;
    if (def.HasRotationSpeedVariation()) {
        u32 s = seed;
        float rate = (float)(short)(frame.rotationSpeed >> 16) * FloatBits(0x3bc90e2b);  // 2 pi / 1024
        rate += jet::core::Randf2(s) * def.GetRotationSpeedVariation();
        rate *= FloatBits(0x4322fae1);  // 1024 / 2 pi
        int t = (int)rate << 16;
        dir = (t <= 0 ? -1 : 1) * (int)rotationSign;
        unsigned mag = (unsigned)(t < 0 ? -t : t);
        step = (dtMs * mag) >> 10;
    } else {
        dir = (int)rotationSign;
        step = ((unsigned)frame.rotationSpeed * dtMs) >> 10;
    }
    rotationFP = (int)((unsigned)rotationFP + (unsigned)dir * step);
    life += (int)dtMs;
}

}
