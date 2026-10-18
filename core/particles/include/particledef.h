// core/particles/include/particledef.h
#ifndef CORE_PARTICLES_INCLUDE_PARTICLEDEF_H
#define CORE_PARTICLES_INCLUDE_PARTICLEDEF_H

#include <vector>
#include "smath.h"
#include "jtypes.h"
#include "core/pstring.h"
#include "video/material.h"

namespace ps {
struct Particle;

// One animation frame of a particle (0x28 bytes, loaded by ParticleSystem::LoadParticleDef @0x68972c): a particle
// is in frame life >> 6.
struct ParticleDefFrame {
    unsigned color;        // 0x0 the vertex colour (RGBA bytes, the file's red and blue swapped)
    float drag;            // 0x4 the velocity is multiplied by it every update
    float speedScale;      // 0x8 the velocity is multiplied by it to move the particle
    float sizeX, sizeY;    // 0xc 0x10 the quad's size (before the particle's random variation)
    float pivotX, pivotY;  // 0x14 0x18 where in the quad the particle's position is (in quad sizes)
    int rotationSpeed;     // 0x1c the rotation speed (the signed value in the upper 16 bits)
    unsigned short extra;  // 0x20 the second texture coordinate of the vertices
    float quarterSize;     // 0x24 (sizeX + sizeY) / 4
};
static_assert(sizeof(ParticleDefFrame) == 0x28, "ParticleDefFrame is 0x28 bytes");

struct ParticleDefUVFrame { float u0, v0, u1, v1; };                      // 0x10 bytes
struct ParticleDefQuantizedUVFrame { unsigned short u0, v0, u1, v1; };    // 8 bytes

class ParticleDef {
public:
    struct AlignmentDef { enum type { Camera, VelocityToCamera, VelocityAxis, CustomFront, CustomFrame }; };
    // SetRenderTechnique @0x684458 picks the blend formula and the fade of the technique from it
    struct BlendingDef { enum type { Additive, Alpha, InvColor, Screen, DestColor }; };

    ParticleDef();
    // @0x6843d0: the duration, the rotation direction and the initial rotation of a spawned particle
    void Init(Particle &p) const;
    // @0x6823f4
    void SetFrameCount(unsigned count);
    unsigned GetFrameCount() const { return (unsigned)m_frames.size(); }
    // @0x684458: the technique with the camera distance fade patched in, and the blending of the material
    void SetRenderTechnique(const jet::String &name, unsigned blending);
    // @0x68498c / @0x684a10: the uniforms of the fade of the material
    void LinkMaterialUniforms();
    void SetMaterialUniforms();
    int GetMaterialUniforms() const { return m_cameraPositionUniform; }

    const ParticleDefFrame &GetFrame(unsigned i) const { return m_frames[i]; }
    ParticleDefFrame &GetFrame(unsigned i) { return m_frames[i]; }
    ParticleDefUVFrame &GetUVFrame(unsigned i) { return m_uvFrames[i]; }
    const ParticleDefUVFrame &GetUVFrame(unsigned i) const { return m_uvFrames[i]; }
    ParticleDefQuantizedUVFrame &GetQuantizedUVFrame(unsigned i) { return m_quantizedUVFrames[i]; }
    const ParticleDefQuantizedUVFrame &GetQuantizedUVFrame(unsigned i) const { return m_quantizedUVFrames[i]; }
    unsigned GetDuration() const { return m_duration; }
    jet::video::Material &GetMaterial() { return m_material; }
    const jet::video::Material &GetMaterial() const { return m_material; }

    bool HasRotationSpeedVariation() const { return m_hasRotationSpeedVariation; }
    float GetRotationSpeedVariation() const { return m_rotationSpeedVariation; }
    unsigned GetAlignment() const { return m_alignment; }
    float GetLodDistance() const { return m_lodDistance; }
    bool GetLocalParticles() const { return m_localParticles; }

    // loaded straight from the file (ParticleSystem::LoadParticleDef)
    bool m_localParticles;                // 0x171
    unsigned m_alignment;                 // 0x118
    float m_alignmentSpeed;               // 0x11c
    int m_initialRotationMin;             // 0x120 (rotation steps << 16)
    int m_initialRotationMax;             // 0x124
    bool m_randomRotationDirection;       // 0x128
    unsigned m_blending;                  // 0x130
    float m_linearMomentumFactor;         // 0x134 how much of the system's velocity the particles take
    math::vec3<float> m_customFront;      // 0x138
    math::quat<float> m_customFrame;      // 0x144
    float m_lodDistance;                  // 0x154
    float m_minDistance;                  // 0x158 nearer than this to the camera they are not drawn
    float m_maxDistance;                  // 0x15c farther than this they are not
    float m_fadeRange;                    // 0x160
    math::vec2<float> m_sizeVariation;    // 0x164
    float m_rotationSpeedVariation;       // 0x16c
    bool m_hasRotationSpeedVariation;     // 0x170

    static bool s_uvQuantization;         // 0xd00aec: the uv frames are 16 bit integers

private:
    std::vector<ParticleDefFrame> m_frames;                           // 0x0
    std::vector<ParticleDefUVFrame> m_uvFrames;                       // 0xc
    std::vector<ParticleDefQuantizedUVFrame> m_quantizedUVFrames;     // 0x18
    unsigned m_duration;                                              // 0x24 frames << 6
    jet::video::Material m_material;                                  // 0x28
    int m_cameraPositionUniform;                                      // 0x174 _ps_camera_position_
    int m_minCameraDistancesUniform;                                  // 0x178 _ps_min_camera_distances_
};
}

#endif
