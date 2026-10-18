// core/jet/include/video/gles20/gles20shaderprogramflavor.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20SHADERPROGRAMFLAVOR_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20SHADERPROGRAMFLAVOR_H

#include <vector>
#include "video/shaderprogramflavor.h"
#include "video/shaderattribute.h"
#include "video/shaderuniform.h"
#include "video/shadersampler.h"

namespace jet::video {
class GLES20ShaderProgramFlavor : public ShaderProgramFlavor {
public:
    explicit GLES20ShaderProgramFlavor(safe_enum<TypeDef,TypeDef::type> type);
    virtual ~GLES20ShaderProgramFlavor();
    virtual safe_enum<TypeDef,TypeDef::type> GetType();
    unsigned GetGLId();
    virtual const ShaderAttribute &GetAttribute(unsigned);
    unsigned GetAttributeCount() const { return (unsigned)m_attributes.size(); }
    virtual ShaderUniform &GetUniform(unsigned);
    unsigned GetUniformCount();
    virtual const ShaderSampler &GetSampler(unsigned);
    virtual const ShaderSampler &GetAlphaSampler(unsigned);
    virtual unsigned GetSamplersUsed();
    virtual unsigned GetAlphaSamplersUsed();
    virtual unsigned GetMaxSkinNodeCount();

    void SetProgramId(unsigned vs, unsigned fs, unsigned program);
    void SetMaxSkinNodeCount(unsigned);
    void AddAttribute(const ShaderAttribute&);
    void AddUniform(const ShaderUniform&);
    void SetSamplers(const std::vector<ShaderSampler>&, unsigned used);
    void SetAlphaSamplers(const std::vector<ShaderSampler>&, unsigned used);
protected:
    safe_enum<TypeDef,TypeDef::type> m_type;
    unsigned m_maxSkinNodeCount;
    unsigned m_vshaderId;
    unsigned m_fshaderId;
    unsigned m_programId;
    unsigned m_samplersUsed;
    unsigned m_alphaSamplersUsed;
    std::vector<ShaderAttribute> m_attributes;
    std::vector<ShaderUniform> m_uniforms;
    std::vector<ShaderSampler> m_samplers;
    std::vector<ShaderSampler> m_alphaSamplers;
};
}

#endif
