// core/jet/include/video/shaderprogramflavor.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERPROGRAMFLAVOR_H
#define CORE_JET_INCLUDE_VIDEO_SHADERPROGRAMFLAVOR_H

#include <boost/noncopyable.hpp>
#include "jenum.h"

namespace jet::video {
class ShaderAttribute;
class ShaderUniform;
class ShaderSampler;

// A compiled variant of a shader program specialized for one combination of features (skinning,
// vertex color, alpha test, ...); GLES20ShaderProgramFlavor is the only concrete subclass.
class ShaderProgramFlavor : public boost::noncopyable {
public:
    // Normal, or hardware skinned with 1..4 node weights per vertex (PreprocessShaderFlavorType)
    struct TypeDef { enum type { Normal, Skinned1, Skinned2, Skinned3, Skinned4 }; };

    virtual ~ShaderProgramFlavor() {}
    virtual safe_enum<TypeDef,TypeDef::type> GetType() = 0;
    virtual unsigned GetGLId() = 0;
    virtual const ShaderAttribute &GetAttribute(unsigned) = 0;
    virtual ShaderUniform &GetUniform(unsigned) = 0;
    virtual const ShaderSampler &GetSampler(unsigned) = 0;
    virtual const ShaderSampler &GetAlphaSampler(unsigned) = 0;
    virtual unsigned GetSamplersUsed() = 0;
    virtual unsigned GetAlphaSamplersUsed() = 0;
    virtual unsigned GetMaxSkinNodeCount() = 0;
};
}

#endif
