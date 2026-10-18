// core/jet/include/video/renderpass.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERPASS_H
#define CORE_JET_INCLUDE_VIDEO_RENDERPASS_H

#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>
#include "jenum.h"
#include "core/pstring.h"
#include "video/shaderdata.h"
#include "video/shaderprogram.h"
#include "video/renderstate.h"

namespace pugi { class xml_node; }

namespace jet::video {
class RenderTechnique;

class RenderPass : public boost::noncopyable {
public:
    // <pass type="auto|quad">: a regular geometry pass, or a fullscreen quad (post effects)
    struct TypeDef { enum type { Auto, Quad }; };

    RenderPass(RenderTechnique*, const jet::String&);
    virtual ~RenderPass();
    void Load(const pugi::xml_node&);
    void Link();
    const jet::String &GetName() const;
    void SetName(const jet::String&);
    unsigned GetTag() const;
    ShaderData &GetAutoShaderData();
    const ShaderData &GetAutoShaderData() const;
    ShaderData &GetShaderData();
    const ShaderData &GetShaderData() const;
    RenderState &GetRenderState();
    const RenderState &GetRenderState() const;
    unsigned GetRenderTargetIdx() const;
    virtual void SetRenderTargetIdx(unsigned);
    ShaderProgram *GetShaderProgram();
    unsigned GetLodCount() const;
    float GetLodDistance(unsigned) const;
    unsigned FindLodByDistance(float) const;
    safe_enum<TypeDef,TypeDef::type> GetType() const;
protected:
    void LoadV100(const pugi::xml_node&);
    jet::String m_name;
    unsigned m_tag;
    RenderTechnique *m_technique;
    std::vector<float> m_lodDistances;
    ShaderData m_shaderData;
    ShaderData m_autoShaderData;
    std::unique_ptr<ShaderProgram> m_shaderProgram;
    RenderState m_renderState;
    unsigned m_renderTargetIdx;
    safe_enum<TypeDef,TypeDef::type> m_type;
};
}

#endif
