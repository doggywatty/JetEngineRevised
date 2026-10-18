// core/jet/src/video/material.cpp
#include "video/material.h"
#include "video/rendertechniqueloader.h"
#include <cstring>

namespace jet::video {

Material::Material() : m_color(0xffffffff), m_drawLayer(0), m_usedAttributes(0), m_allTagMasks(0), m_hash(0) {}

Material::Material(const Material &o) { *this = o; }

Material::~Material() {}

void Material::ResetHash() { m_hash = 0; }

Material &Material::operator=(const Material &o) {
    m_color = o.m_color;
    m_drawLayer = o.m_drawLayer;
    m_technique = o.m_technique;
    m_uniformData = o.m_uniformData;
    m_textureLayers = o.m_textureLayers;
    m_passes = o.m_passes;
    m_usedAttributes = o.m_usedAttributes;
    m_allTagMasks = o.m_allTagMasks;
    ResetHash();
    return *this;
}

bool Material::operator==(const Material &o) const {
    if (m_color != o.m_color || m_drawLayer != o.m_drawLayer || m_technique != o.m_technique) return false;
    for (unsigned i = 0; i < m_passes.size(); i++)
        if (!EqualsPass(o, i)) return false;
    return true;
}
bool Material::operator!=(const Material &o) const { return !(*this == o); }

bool Material::EqualsPass(const Material &o, unsigned pass) const {
    if (pass >= m_passes.size() || pass >= o.m_passes.size()) return false;
    const PassData &a = m_passes[pass];
    const PassData &b = o.m_passes[pass];
    if (a.uniformDataSize != b.uniformDataSize) return false;
    if (memcmp(&m_uniformData[a.uniformDataOffset], &o.m_uniformData[b.uniformDataOffset], a.uniformDataSize) != 0) return false;
    if (a.textureLayerCount != b.textureLayerCount) return false;
    for (unsigned i = 0; i < a.textureLayerCount; i++)
        if (m_textureLayers[a.textureLayerStart + i] != o.m_textureLayers[b.textureLayerStart + i]) return false;
    return a.renderState == b.renderState;
}

unsigned Material::GetPassHash(unsigned pass) const {
    const PassData &p = m_passes[pass];
    unsigned h = 2166136261u;
    for (unsigned i = 0; i < p.uniformDataSize; i++) h = (h ^ m_uniformData[p.uniformDataOffset + i]) * 16777619u;
    return h;
}
unsigned Material::GetHash() const {
    if (m_hash) return m_hash;
    unsigned h = m_color * 16777619u ^ (unsigned)m_drawLayer;
    for (unsigned i = 0; i < m_passes.size(); i++) h = h * 16777619u ^ GetPassHash(i);
    return h;
}

bool Material::IsValid() const { return m_technique.get() != 0; }

void Material::Reset() {
    m_color = 0xffffffff;
    m_drawLayer = 0;
    for (unsigned i = 0; i < m_textureLayers.size(); i++) {
        m_textureLayers[i].SetTexture(boost::shared_ptr<Texture>());
        m_textureLayers[i].SetAlphaTexture(boost::shared_ptr<Texture>());
    }
    m_allTagMasks = 0;
    ResetHash();
}

unsigned Material::GetColor() const { return m_color; }
void Material::SetColor(unsigned c) { m_color = c; ResetHash(); }
void Material::SetDrawLayer(int l) { m_drawLayer = l; }
int Material::GetDrawLayer() const { return m_drawLayer; }

void Material::SetRenderTechnique(const boost::shared_ptr<RenderTechnique> &tech) {
    ResetHash();
    m_technique = tech;
    m_passes.clear();
    m_uniformData.clear();
    m_textureLayers.clear();
    m_usedAttributes = 0;
    m_allTagMasks = 0;
    if (!tech) return;

    unsigned passCount = tech->GetRenderPassCount();
    m_passes.resize(passCount);
    for (unsigned i = 0; i < passCount; i++) {
        RenderPass &pass = tech->GetRenderPass(i);
        const ShaderData &sd = pass.GetShaderData();
        const ShaderData &autoSd = pass.GetAutoShaderData();
        PassData &pd = m_passes[i];
        pd.tagMask = 1u << (pass.GetTag() & 0xff);
        m_allTagMasks |= pd.tagMask;
        pd.renderState = pass.GetRenderState();

        pd.uniformDataOffset = (unsigned)m_uniformData.size();
        pd.uniformDataSize = sd.GetTotalUniformDataSize();
        m_uniformData.resize(m_uniformData.size() + pd.uniformDataSize);
        for (unsigned u = 0; u < sd.GetUniformCount(); u++) {
            const ShaderUniform &def = sd.GetUniform(u);
            memcpy(&m_uniformData[pd.uniformDataOffset + sd.GetUniformDataOffset(u)], def.GetData(), def.GetDataSize());
        }

        pd.textureLayerStart = (unsigned)m_textureLayers.size();
        pd.textureLayerCount = sd.GetSamplerCount();
        for (unsigned s = 0; s < sd.GetSamplerCount(); s++) {
            TextureLayer layer;
            const ShaderSampler &samp = sd.GetSampler(s);
            layer.SetWrapS(samp.GetWrapS());
            layer.SetWrapT(samp.GetWrapT());
            layer.SetFilter(samp.GetFilter());
            layer.SetAnisotropy(samp.GetAnisotropy());
            m_textureLayers.push_back(layer);
        }

        m_usedAttributes |= sd.GetUsedAttributes() | autoSd.GetUsedAttributes();
    }
}

// SetRenderTechnique(name) @0x6eaf68: through the technique loader (one instance per file)
void Material::SetRenderTechnique(const jet::String &name) {
    SetRenderTechnique(RenderTechniqueLoader::GetInstance()->Load(name));
}

const boost::shared_ptr<RenderTechnique> &Material::GetRenderTechnique() const { return m_technique; }

unsigned Material::GetRenderPassCount() const { return (unsigned)m_passes.size(); }
const jet::String &Material::GetRenderPassName(unsigned i) const { return m_technique->GetRenderPass(i).GetName(); }
RenderState &Material::GetRenderState(unsigned i) { return m_passes[i].renderState; }
const RenderState &Material::GetRenderState(unsigned i) const { return m_passes[i].renderState; }

unsigned Material::GetUniformCount(unsigned pass) const { return m_technique->GetRenderPass(pass).GetShaderData().GetUniformCount(); }
const jet::String &Material::GetUniformName(unsigned pass, unsigned i) const { return m_technique->GetRenderPass(pass).GetShaderData().GetUniformName(i); }
int Material::FindUniformIdxByName(unsigned pass, const jet::String &name) const { return m_technique->GetRenderPass(pass).GetShaderData().FindUniformIdxByName(name); }
const ShaderUniform &Material::GetUniformDef(unsigned pass, unsigned i) const { return m_technique->GetRenderPass(pass).GetShaderData().GetUniform(i); }

const unsigned char *Material::GetUniformData(unsigned pass, unsigned i) const {
    const PassData &pd = m_passes[pass];
    const ShaderData &sd = m_technique->GetRenderPass(pass).GetShaderData();
    return &m_uniformData[pd.uniformDataOffset + sd.GetUniformDataOffset(i)];
}

ShaderUniform Material::GetUniform(unsigned pass, unsigned i) const {
    const ShaderData &sd = m_technique->GetRenderPass(pass).GetShaderData();
    const ShaderUniform &def = sd.GetUniform(i);
    return ShaderUniform(def.GetType(), (unsigned short)def.GetCount(), GetUniformData(pass, i));
}

void Material::SetUniform(unsigned pass, unsigned i, const ShaderUniform &value) {
    const PassData &pd = m_passes[pass];
    const ShaderData &sd = m_technique->GetRenderPass(pass).GetShaderData();
    unsigned size = sd.GetUniformDataSize(i);
    memcpy(&m_uniformData[pd.uniformDataOffset + sd.GetUniformDataOffset(i)], value.GetData(), std::min(size, value.GetDataSize()));
    ResetHash();
}

bool Material::SetUniform(unsigned pass, const jet::String &name, const ShaderUniform &value) {
    int idx = FindUniformIdxByName(pass, name);
    if (idx < 0) return false;
    SetUniform(pass, (unsigned)idx, value);
    return true;
}

unsigned Material::GetTextureLayerCount(unsigned pass) const { return m_passes[pass].textureLayerCount; }
TextureLayer &Material::GetTextureLayer(unsigned pass, unsigned i) { return m_textureLayers[m_passes[pass].textureLayerStart + i]; }
const TextureLayer &Material::GetTextureLayer(unsigned pass, unsigned i) const { return m_textureLayers[m_passes[pass].textureLayerStart + i]; }
const jet::String &Material::GetTextureLayerName(unsigned pass, unsigned i) const {
    return m_technique->GetRenderPass(pass).GetShaderData().GetSamplerName(i);
}
TextureLayer *Material::FindTextureLayerByName(unsigned pass, const jet::String &name) {
    int idx = m_technique->GetRenderPass(pass).GetShaderData().FindSamplerIdxByName(name);
    return idx < 0 ? 0 : &GetTextureLayer(pass, (unsigned)idx);
}
const TextureLayer *Material::FindTextureLayerByName(unsigned pass, const jet::String &name) const {
    int idx = m_technique->GetRenderPass(pass).GetShaderData().FindSamplerIdxByName(name);
    return idx < 0 ? 0 : &GetTextureLayer(pass, (unsigned)idx);
}

void Material::SetTextureFiltering(safe_enum<Texture::FilterDef,unsigned char> f) {
    for (unsigned i = 0; i < m_textureLayers.size(); i++) m_textureLayers[i].SetFilter(f);
}
void Material::SetTextureMipmapping(bool v) {
    for (unsigned i = 0; i < m_textureLayers.size(); i++) m_textureLayers[i].SetMipmap(v);
}
void Material::SetTextureWrapping(safe_enum<Texture::WrapDef,unsigned char> w) {
    for (unsigned i = 0; i < m_textureLayers.size(); i++) { m_textureLayers[i].SetWrapS(w); m_textureLayers[i].SetWrapT(w); }
}

unsigned Material::GetUsedAttributes() const { return m_usedAttributes; }
unsigned Material::GetAllTagMasks() const { return m_allTagMasks; }
unsigned Material::GetTagMask(unsigned pass) const { return m_passes[pass].tagMask; }
void Material::SetTagActive(unsigned pass, unsigned, bool) { (void)pass; }
bool Material::IsTagActive(unsigned pass, unsigned tag) const { return (m_passes[pass].tagMask & (1u << (tag & 0xff))) != 0; }

// XML material format not yet reverse engineered (only the code path through a .rendertechnique
// reference is implemented); Load()/LoadV100() are the remaining piece here.
void Material::Load(jet::stream::IStream&) {}
void Material::LoadV100(const pugi::xml_node&) {}

}
