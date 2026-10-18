// core/jet/include/video/shaderprogram.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERPROGRAM_H
#define CORE_JET_INCLUDE_VIDEO_SHADERPROGRAM_H

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "jenum.h"
#include "core/pstring.h"
#include "video/shaderuniform.h"
#include "video/shaderprogramflavor.h"

namespace pugi { class xml_node; }

namespace jet::video {
class RenderTechnique;
class ShaderData;
class Material;

class ShaderProgram : public boost::noncopyable {
public:
    static ShaderProgram *New(RenderTechnique*);
    virtual ~ShaderProgram();
    void Load(const pugi::xml_node&);
    virtual void SetShaderData(ShaderData*, ShaderData*) = 0;
    virtual void Link() = 0;
    virtual void Relink() = 0;
    void SetDebugName(const jet::String&);
    const jet::String &GetDebugName() const;
    bool IsReady() const;
    virtual ShaderProgramFlavor *GetFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>, const Material&, unsigned, const std::vector<ShaderUniform>&) = 0;
    virtual bool IsFlavorCompiled(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>) = 0;
    virtual const std::string &GetFlavorVertexSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>) = 0;
    virtual const std::string &GetFlavorFragmentSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>) = 0;
    virtual void SetVertexSource(const std::string&) = 0;
    virtual const std::string &GetVertexSource() = 0;
    virtual void SetFragmentSource(const std::string&) = 0;
    virtual const std::string &GetFragmentSource() = 0;
protected:
    ShaderProgram(RenderTechnique*);
    virtual void LoadV100(const pugi::xml_node&) = 0;
    jet::String m_debugName;
    RenderTechnique *m_technique;
    bool m_isReady;
};
}

#endif
