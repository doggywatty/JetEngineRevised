// core/particles/include/particle.h
#ifndef CORE_PARTICLES_INCLUDE_PARTICLE_H
#define CORE_PARTICLES_INCLUDE_PARTICLE_H

#include "smath.h"
#include "jtypes.h"

namespace ps {
class ParticleDef;

// One live particle (0x40 bytes, an element of Emitter's vector). The shapes fill it in when they spawn it
// (position, velocity, constant acceleration, seed); ParticleDef::Init sets the duration and the rotation.
struct Particle {
    int life;                          // 0x0 ms lived
    int duration;                      // 0x4 the definition's frame count << 6
    math::vec3<float> position;        // 0x8
    math::vec3<float> velocity;        // 0x14
    math::vec3<float> acceleration;    // 0x20
    unsigned seed;                     // 0x2c the LCG of its random size / rotation speed
    int rotationFP;                    // 0x30 rotation, 1024 steps of 2 pi per 1 << 16
    short rotationSign;                // 0x34 1 or -1
    unsigned char axes[6];             // 0x36 the quantized right (x, y, z) and up (x, y, z) vectors of the quad
    short brownianTime;                // 0x3c ms left before the next Brownian kick

    Particle();
    // @0x6824f0
    void Update(const ParticleDef &def, float dt, unsigned dtMs);
};
static_assert(sizeof(Particle) == 0x40, "ps::Particle is 0x40 bytes");
}

#endif
