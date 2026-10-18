// core/particles/src/particledef.cpp
#include "particledef.h"
#include "particle.h"
#include "core/util.h"
#include "video/blendformula.h"
#include "video/rendertechniqueloader.h"

namespace ps {

bool ParticleDef::s_uvQuantization = true;

ParticleDef::ParticleDef()
    : m_localParticles(false), m_alignment(0), m_alignmentSpeed(0), m_initialRotationMin(0), m_initialRotationMax(0),
      m_randomRotationDirection(false), m_blending(0), m_linearMomentumFactor(0), m_customFront(0, 0, 0),
      m_customFrame(0, 0, 0, 1), m_lodDistance(0), m_minDistance(0), m_maxDistance(0), m_fadeRange(0), m_sizeVariation(0, 0),
      m_rotationSpeedVariation(0), m_hasRotationSpeedVariation(false), m_duration(0), m_cameraPositionUniform(-1),
      m_minCameraDistancesUniform(-1) {}

// @0x6843d0
void ParticleDef::Init(Particle &p) const {
    p.duration = (int)m_duration;
    if (m_randomRotationDirection && (jet::core::Rand() & 0xffff) >= 0x8000) p.rotationSign = -1;
    else p.rotationSign = 1;
    int lo = m_initialRotationMin < m_initialRotationMax ? m_initialRotationMin : m_initialRotationMax;
    int hi = m_initialRotationMin < m_initialRotationMax ? m_initialRotationMax : m_initialRotationMin;
    int span = hi - lo;
    int rot = lo;
    unsigned r = jet::core::Rand();
    if (span != 0) rot = lo + (int)((long long)((int)r >> 2) % span);
    p.rotationFP = rot;
}

// @0x6823f4: the frames, the uv frames (integers when quantized) and the duration (frames << 6)
void ParticleDef::SetFrameCount(unsigned count) {
    m_frames.resize(count);
    if (s_uvQuantization) {
        if (count > m_quantizedUVFrames.size()) m_quantizedUVFrames.resize(count);
        else if (count < m_quantizedUVFrames.size()) m_quantizedUVFrames.resize(count);
    } else {
        if (count > m_uvFrames.size()) m_uvFrames.resize(count);
        else if (count < m_uvFrames.size()) m_uvFrames.resize(count);
    }
    m_duration = count << 6;
}

// @0x6843e0..: the text the vertex shader gets in front (declares the camera distances and the fade factor), then
// the function that applies the fade: of the alpha for the alpha blending, of the colour for the others
static const char *const k_fadeHeader =
    "uniform highp vec3 _ps_camera_position_;\n"
    "uniform highp vec2 _ps_min_camera_distances_; //x - distance full transparent, y - distance for fully visible. So y - x is the visibility range \n"
    "float _ps_computeFadeFactor_()\n"
    "{\n"
    "\thighp float range = _ps_min_camera_distances_.y - _ps_min_camera_distances_.x;\n"
    "\thighp float factor = (gl_Position.w - _ps_min_camera_distances_.x) / range;\n"
    "\treturn clamp(factor, 0.0, 1.0);\n"
    "}\n";
static const char *const k_fadeAlpha = "lowp vec4 _ps_fadeOut_(lowp vec4 color) { color.a *= _ps_computeFadeFactor_(); return color; }\n";
static const char *const k_fadeColor = "lowp vec4 _ps_fadeOut_(lowp vec4 color) { color.rgb *= _ps_computeFadeFactor_(); return color; }\n";

// @0x684458
void ParticleDef::SetRenderTechnique(const jet::String &name, unsigned blending) {
    m_blending = blending;
    jet::String vertexPatch(k_fadeHeader);
    jet::String fragmentPatch;
    switch (blending) {
    case BlendingDef::Additive: vertexPatch.append(k_fadeColor); break;
    case BlendingDef::Alpha: vertexPatch.append(k_fadeAlpha); break;
    case BlendingDef::DestColor: vertexPatch.append(k_fadeColor); break;
    case BlendingDef::Screen: vertexPatch.append(k_fadeColor); break;
    case BlendingDef::InvColor: vertexPatch.append(k_fadeColor); break;
    default: break;
    }
    m_material.SetRenderTechnique(jet::video::RenderTechniqueLoader::GetInstance()->LoadAndPatch(name, vertexPatch, fragmentPatch));
    if (!m_material.IsValid()) return;
    jet::video::RenderState &rs = m_material.GetRenderState(0);
    typedef jet::video::BlendFormula BF;
    switch (blending) {
    case BlendingDef::Additive: rs.SetBlending(true); rs.SetBlendFormula(BF(BF::ComboDef::Additive)); break;
    case BlendingDef::Alpha: rs.SetBlending(true); rs.SetBlendFormula(BF(BF::ComboDef::Alpha)); break;
    case BlendingDef::DestColor: rs.SetBlending(true); rs.SetBlendFormula(BF(BF::ComboDef::DestColorAdd)); break;
    case BlendingDef::Screen: rs.SetBlending(true); rs.SetBlendFormula(BF(BF::ComboDef::Screen)); break;
    case BlendingDef::InvColor: rs.SetBlending(true); rs.SetBlendFormula(BF(BF::ComboDef::ZeroInvSrcColor)); break;
    default: break;
    }
    LinkMaterialUniforms();
    SetMaterialUniforms();
}

// @0x68498c
void ParticleDef::LinkMaterialUniforms() {
    m_cameraPositionUniform = m_material.FindUniformIdxByName(0, jet::String("_ps_camera_position_"));
    m_minCameraDistancesUniform = m_material.FindUniformIdxByName(0, jet::String("_ps_min_camera_distances_"));
}

// @0x684a10: the distances of the fade: nearer than the minimum distance nothing shows, it is fully there fade range farther
void ParticleDef::SetMaterialUniforms() {
    if (m_minCameraDistancesUniform < 0) return;
    math::vec2<float> distances(m_minDistance, m_minDistance + m_fadeRange);
    m_material.SetUniform(0, (unsigned)m_minCameraDistancesUniform, jet::video::ShaderUniform(distances));
}

}
