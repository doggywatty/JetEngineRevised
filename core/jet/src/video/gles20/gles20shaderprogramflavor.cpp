// core/jet/src/video/gles20/gles20shaderprogramflavor.cpp
#include "video/gles20/gles20shaderprogramflavor.h"

namespace jet::video {

GLES20ShaderProgramFlavor::GLES20ShaderProgramFlavor(safe_enum<TypeDef,TypeDef::type> type)
    : m_type(type), m_maxSkinNodeCount(0), m_vshaderId(0), m_fshaderId(0), m_programId(0),
      m_samplersUsed(0), m_alphaSamplersUsed(0) {
}
GLES20ShaderProgramFlavor::~GLES20ShaderProgramFlavor() {}

safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> GLES20ShaderProgramFlavor::GetType() { return m_type; }
unsigned GLES20ShaderProgramFlavor::GetGLId() { return m_programId; }
const ShaderAttribute &GLES20ShaderProgramFlavor::GetAttribute(unsigned i) { return m_attributes[i]; }
ShaderUniform &GLES20ShaderProgramFlavor::GetUniform(unsigned i) { return m_uniforms[i]; }
unsigned GLES20ShaderProgramFlavor::GetUniformCount() { return (unsigned)m_uniforms.size(); }
const ShaderSampler &GLES20ShaderProgramFlavor::GetSampler(unsigned i) { return m_samplers[i]; }
const ShaderSampler &GLES20ShaderProgramFlavor::GetAlphaSampler(unsigned i) { return m_alphaSamplers[i]; }
unsigned GLES20ShaderProgramFlavor::GetSamplersUsed() { return m_samplersUsed; }
unsigned GLES20ShaderProgramFlavor::GetAlphaSamplersUsed() { return m_alphaSamplersUsed; }
unsigned GLES20ShaderProgramFlavor::GetMaxSkinNodeCount() { return m_maxSkinNodeCount; }

void GLES20ShaderProgramFlavor::SetProgramId(unsigned vs, unsigned fs, unsigned program) {
    m_vshaderId = vs; m_fshaderId = fs; m_programId = program;
}
void GLES20ShaderProgramFlavor::SetMaxSkinNodeCount(unsigned c) { m_maxSkinNodeCount = c; }
void GLES20ShaderProgramFlavor::AddAttribute(const ShaderAttribute &a) { m_attributes.push_back(a); }
void GLES20ShaderProgramFlavor::AddUniform(const ShaderUniform &u) { m_uniforms.push_back(u); }
void GLES20ShaderProgramFlavor::SetSamplers(const std::vector<ShaderSampler> &s, unsigned used) { m_samplers = s; m_samplersUsed = used; }
void GLES20ShaderProgramFlavor::SetAlphaSamplers(const std::vector<ShaderSampler> &s, unsigned used) { m_alphaSamplers = s; m_alphaSamplersUsed = used; }

}
