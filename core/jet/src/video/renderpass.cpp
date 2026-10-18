// core/jet/src/video/renderpass.cpp
#include "video/renderpass.h"
#include "pugixml.hpp"
#include <cstdlib>
#include <cstdio>
#include "video/driver.h"

namespace jet::video {

RenderPass::RenderPass(RenderTechnique *tech, const jet::String &name)
    : m_name(name), m_tag(0), m_technique(tech), m_renderTargetIdx(0), m_type(TypeDef::Auto) {
}

RenderPass::~RenderPass() {}

const jet::String &RenderPass::GetName() const { return m_name; }
void RenderPass::SetName(const jet::String &n) { m_name = n; }
unsigned RenderPass::GetTag() const { return m_tag; }

ShaderData &RenderPass::GetAutoShaderData() { return m_autoShaderData; }
const ShaderData &RenderPass::GetAutoShaderData() const { return m_autoShaderData; }
ShaderData &RenderPass::GetShaderData() { return m_shaderData; }
const ShaderData &RenderPass::GetShaderData() const { return m_shaderData; }

RenderState &RenderPass::GetRenderState() { return m_renderState; }
const RenderState &RenderPass::GetRenderState() const { return m_renderState; }

unsigned RenderPass::GetRenderTargetIdx() const { return m_renderTargetIdx; }
void RenderPass::SetRenderTargetIdx(unsigned i) { m_renderTargetIdx = i; }

ShaderProgram *RenderPass::GetShaderProgram() { return m_shaderProgram.get(); }

unsigned RenderPass::GetLodCount() const { return (unsigned)m_lodDistances.size(); }
float RenderPass::GetLodDistance(unsigned i) const { return m_lodDistances[i]; }
unsigned RenderPass::FindLodByDistance(float dist) const {
    for (unsigned i = 0; i < m_lodDistances.size(); i++)
        if (dist <= m_lodDistances[i]) return i;
    return m_lodDistances.empty() ? 0 : (unsigned)m_lodDistances.size() - 1;
}

safe_enum<RenderPass::TypeDef,RenderPass::TypeDef::type> RenderPass::GetType() const { return m_type; }

void RenderPass::Link() {
    if (!m_shaderProgram.get()) return;
    m_shaderProgram->SetShaderData(&m_shaderData, &m_autoShaderData);
    m_shaderProgram->Link();
}

// LoadV100 @0x6f0320. A pass element:
//   <pass name= type="auto|quad" tag=>
//     <rendertarget val=/>            (default "default")
//     <uniforms><var name= type= val= min= max= default=/>...</uniforms>
//         val is first offered to the driver as an engine value (Driver::LinkUniform); linked
//         uniforms go to the auto shader data, the others are material parameters with min/max
//     <attributes><var name= val="mesh.xxx"/>...</attributes>
//     <samplers><var name= val= wrapS= wrapT= filter= semantic= uvSet= anisotropy=/>...</samplers>
//         a val (binding) makes it an auto sampler
//     <renderstate>...</renderstate> <shader>...</shader> <lods><lod distance=/>...</lods>
void RenderPass::LoadV100(const pugi::xml_node &node) {
    if (pugi::xml_attribute a = node.attribute("name")) m_name = a.value();
    if (pugi::xml_attribute a = node.attribute("type")) {
        jet::String t(a.value());
        if (t.Equals("auto")) m_type = safe_enum<TypeDef,TypeDef::type>(TypeDef::Auto);
        else if (t.Equals("quad")) m_type = safe_enum<TypeDef,TypeDef::type>(TypeDef::Quad);
        else printf("Cannot load render pass of type %s\n", t.c_str());
    }
    if (pugi::xml_attribute a = node.attribute("tag")) {
        Driver *d = Driver::GetCurrent();
        m_tag = d ? d->GetTag(jet::String(a.value())) : 0;
    }
    {
        jet::String rt("default");
        if (pugi::xml_node n = node.child("rendertarget"))
            if (pugi::xml_attribute a = n.attribute("val")) rt = a.value();
        Driver *d = Driver::GetCurrent();
        int idx = d ? d->FindRenderTargetIdxByName(rt) : -1;
        m_renderTargetIdx = idx < 0 ? 0 : (unsigned)idx;
    }
    if (pugi::xml_node u = node.child("uniforms")) {
        for (pugi::xml_node v = u.child("var"); v; v = v.next_sibling("var")) {
            pugi::xml_attribute nameAttr = v.attribute("name");
            ShaderUniform uniform, uMin, uMax;
            if (pugi::xml_attribute a = v.attribute("type")) uniform.ParseType(jet::String(a.value()));
            pugi::xml_attribute val = v.attribute("val"), mn = v.attribute("min"), mx = v.attribute("max");
            if (val) uniform.ParseValue(jet::String(val.value()), true);
            if (mn) { uMin = uniform; uMin.ParseValue(jet::String(mn.value()), false); }
            if (mx) { uMax = uniform; uMax.ParseValue(jet::String(mx.value()), false); }
            if (pugi::xml_attribute a = v.attribute("default")) {
                if (uniform.GetCount() > 1) uniform.ParseArrayValues(jet::String(a.value()));
                else uniform.ParseValue(jet::String(a.value()), false);
            }
            if (uniform.HasServerData())
                m_autoShaderData.AddUniform(jet::String(nameAttr.value()), uniform);
            else
                m_shaderData.AddUniform(jet::String(nameAttr.value()), uniform, uMin, uMax);
        }
    }
    // debug uniforms every pass gets (the fragment shader declares them, see GLES20ShaderProgram::LoadV100)
    {
        ShaderUniform c; c.ParseType(jet::String("vec4")); c.ParseValue(jet::String("mesh.lodColor"), true);
        m_autoShaderData.AddUniform(jet::String("_jet_debugColor"), c);
        ShaderUniform l; l.ParseType(jet::String("int")); l.ParseValue(jet::String("mesh.lod"), true);
        m_autoShaderData.AddUniform(jet::String("_jet_debugLod"), l);
    }
    if (pugi::xml_node at = node.child("attributes")) {
        for (pugi::xml_node v = at.child("var"); v; v = v.next_sibling("var")) {
            pugi::xml_attribute nameAttr = v.attribute("name"), val = v.attribute("val");
            if (!nameAttr || !val) continue;
            ShaderAttribute attr;
            attr.ParseValue(jet::String(val.value()));
            m_shaderData.AddAttribute(jet::String(nameAttr.value()), attr);
        }
    }
    if (pugi::xml_node sm = node.child("samplers")) {
        for (pugi::xml_node v = sm.child("var"); v; v = v.next_sibling("var")) {
            pugi::xml_attribute nameAttr = v.attribute("name");
            if (!nameAttr) continue;
            ShaderSampler sampler;
            pugi::xml_attribute val = v.attribute("val");
            if (val) sampler.ParseBinding(jet::String(val.value()));
            pugi::xml_attribute ws = v.attribute("wrapS"), wt = v.attribute("wrapT");
            if (ws || wt) sampler.ParseWrap(jet::String(ws ? ws.value() : "repeat"), jet::String(wt ? wt.value() : "repeat"));
            if (pugi::xml_attribute a = v.attribute("filter")) sampler.ParseFilter(jet::String(a.value()));
            if (pugi::xml_attribute a = v.attribute("semantic")) sampler.ParseSemantic(jet::String(a.value()));
            if (pugi::xml_attribute a = v.attribute("uvSet")) sampler.ParseUVSet(jet::String(a.value()));
            if (pugi::xml_attribute a = v.attribute("anisotropy")) sampler.ParseAnisotropy(jet::String(a.value()));
            if (val) m_autoShaderData.AddSampler(jet::String(nameAttr.value()), sampler);
            else m_shaderData.AddSampler(jet::String(nameAttr.value()), sampler);
        }
    }
    if (pugi::xml_node rs = node.child("renderstate")) m_renderState.Load(rs);
    if (pugi::xml_node sh = node.child("shader")) {
        m_shaderProgram.reset(ShaderProgram::New(m_technique));
        m_shaderProgram->SetShaderData(&m_shaderData, &m_autoShaderData);
        m_shaderProgram->Load(sh);
    }
    if (pugi::xml_node lods = node.child("lods"))
        for (pugi::xml_node lod = lods.child("lod"); lod; lod = lod.next_sibling("lod")) {
            pugi::xml_attribute d = lod.attribute("distance");
            if (d) m_lodDistances.push_back((float)d.as_double());
        }
}

// Load @0x6f02c8
void RenderPass::Load(const pugi::xml_node &node) { LoadV100(node); }

}
