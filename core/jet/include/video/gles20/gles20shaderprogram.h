// core/jet/include/video/gles20/gles20shaderprogram.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20SHADERPROGRAM_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20SHADERPROGRAM_H

#include <string>
#include <map>
#include "video/shaderprogram.h"
#include "video/gles20/gles20shaderprogramflavor.h"
#include "video/gles20/gles20interface.h"
#include "video/shaderdata.h"

namespace pugi { class xml_node; }

namespace jet::video {

// A simplified (but real, GL-compiling) rewrite of GLES20ShaderProgram: one compiled flavor per
// ShaderProgramFlavor::TypeDef (Normal, Skinned1..4 by node weights per vertex), selected via a
// #define injected before the shader source, rather than the original's full "decision uniform"
// combinatorial flavor cache (GLES20FlavorSet, keyed by a 64-bit hash of per-material uniform
// values -- an optimization for generating many shader variants from one material's authored
// uniforms, not implemented here). Compilation/linking/uniform upload themselves are real GL.
class GLES20ShaderProgram : public ShaderProgram {
public:
    explicit GLES20ShaderProgram(RenderTechnique*);
    virtual ~GLES20ShaderProgram();
    virtual void SetShaderData(ShaderData*, ShaderData*);
    virtual void Link();
    virtual void Relink();
    virtual ShaderProgramFlavor *GetFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>, const Material&, unsigned, const std::vector<ShaderUniform>&);
    virtual bool IsFlavorCompiled(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    virtual const std::string &GetFlavorVertexSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    virtual const std::string &GetFlavorFragmentSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    virtual void SetVertexSource(const std::string&);
    virtual const std::string &GetVertexSource();
    virtual void SetFragmentSource(const std::string&);
    virtual const std::string &GetFragmentSource();
    static void UploadUniform(gles::Interface&, int location, const ShaderUniform&);
protected:
    virtual void LoadV100(const pugi::xml_node&);
    bool CompileFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    static const char *FlavorDefine(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    unsigned CompileStage(gles::Interface&, unsigned stage, const std::string&, const char *define);
    std::string PreprocessShaderFlavorType(const std::string &vs, unsigned weights) const;
private:
    std::string m_vshaderSrc;
    std::string m_fshaderSrc;
    std::vector<std::string> m_alphaSamplers;
    ShaderData *m_shaderData;
    ShaderData *m_autoShaderData;
    std::map<int, GLES20ShaderProgramFlavor*> m_flavors;  // keyed by TypeDef::type
};
}

#endif
