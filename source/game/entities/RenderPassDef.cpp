// source/game/entities/RenderPassDef.cpp
#include "RenderPassDef.h"
#include "video/material.h"
#include "video/textureloader.h"
#include "video/driver.h"
#include <algorithm>

jet::String RenderPassDef::k_tmplName("RenderPass");

RenderPassDef::RenderPassDef(const clara::Template *t) : GameEntity(t) {}
RenderPassDef::~RenderPassDef() {}

// Components named "" or "unused" are padding for shader parameters that do not exist
static bool IsUniformName(const jet::String &n) { return !n.empty() && !n.Equals("unused"); }

static void SetByName(jet::video::Material &m, unsigned pass, const jet::String &name, const jet::video::ShaderUniform &u) {
    int idx = m.FindUniformIdxByName(pass, name);
    if (idx >= 0) m.SetUniform(pass, (unsigned)idx, u);
}

// LoadMaterialValues @0x51c5bc
void RenderPassDef::LoadMaterialValues(jet::video::Material &m, unsigned pass) const {
    using jet::video::ShaderUniform;
    if (const clara::Param *p = FindParamByName(jet::String("Values")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            const jet::String &n = p->GetComponentName(i);
            if (!IsUniformName(n)) continue;
            float v = p->GetAsFloat(i);
            if (n.Equals("Shininess")) v *= 100.0f;
            SetByName(m, pass, n, ShaderUniform(v));
        }
    if (const clara::Param *p = FindParamByName(jet::String("Vec2Values")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++)
            if (IsUniformName(p->GetComponentName(i))) SetByName(m, pass, p->GetComponentName(i), ShaderUniform(p->GetAsVector2D(i)));
    if (const clara::Param *p = FindParamByName(jet::String("Vec3Values")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++)
            if (IsUniformName(p->GetComponentName(i))) SetByName(m, pass, p->GetComponentName(i), ShaderUniform(p->GetAsVector3D(i)));
    if (const clara::Param *p = FindParamByName(jet::String("Vec4Values")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++)
            if (IsUniformName(p->GetComponentName(i))) SetByName(m, pass, p->GetComponentName(i), ShaderUniform(p->GetAsVector4D(i)));

    // Blending: every blended mode also stops depth writes
    typedef jet::video::BlendFormula::FactorDef F;
    jet::String blending;
    GetParam(jet::String("Blending"), blending);
    jet::video::RenderState &rs = m.GetRenderState(pass);
    F::type src = F::One, dst = F::Zero;
    bool blend = true;
    if (blending.Equals("Additive")) { src = F::One; dst = F::One; }
    else if (blending.Equals("Transparent")) { src = F::SrcAlpha; dst = F::InvSrcAlpha; }
    else if (blending.Equals("Multiply")) { src = F::Zero; dst = F::SrcColor; }
    else if (blending.Equals("ColorBurn")) { src = F::DstColor; dst = F::One; }
    else if (blending.Equals("Screen")) { src = F::One; dst = F::InvSrcColor; }
    else if (blending.Equals("Substractive")) { src = F::Zero; dst = F::InvSrcColor; }
    else blend = false;
    rs.SetBlending(blend);
    rs.SetBlendFormula(jet::video::BlendFormula(src, dst, src, dst));
    if (blend) rs.SetDepthWrite(false);

    bool doubleFace = false;
    GetParam(jet::String("DoubleFace"), doubleFace);
    rs.SetCulling(!doubleFace);
}

// LoadMaterialTextures @0x51ce74: placeholders are kept by name for ApplyMaterials; files load now. Only as many components
// as the pass has texture layers are read (the .so takes the smaller of the two counts).
void RenderPassDef::LoadMaterialTextures(jet::video::Material &m, std::vector<jet::String> &names, unsigned pass) const {
    const clara::Param *p = FindParamByName(jet::String("Textures"));
    if (!p) { names.clear(); return; }
    const unsigned count = std::min(p->GetComponentCount(), m.GetTextureLayerCount(pass));
    names.clear();
    names.reserve(count);
    static const char *keep[] = { "- None -", "unused", "<Ambient>", "<Diffuse>", "<Bump>", "<SpecularCol>", "<SpecularLvl>",
                                  "<Glossiness>", "<SelfIllum>", "<Opacity>", "<FilterCol>", "<Reflection>", "<Refraction>", "<Displacement>" };
    jet::video::TextureLoader *tl = jet::video::TextureLoader::GetInstance();
    const bool separateAlpha = jet::video::Driver::GetCurrent() && jet::video::Driver::GetCurrent()->GetSeparateAlphaTextures();
    for (unsigned i = 0; i < count; i++) {
        jet::String t = p->GetAsEnumName(i);
        names.push_back(t);
        bool placeholder = t.empty();
        for (unsigned k = 0; k < sizeof(keep) / sizeof(keep[0]) && !placeholder; k++) placeholder = t.Equals(keep[k]);
        if (placeholder) continue;
        jet::video::TextureLayer &layer = m.GetTextureLayer(pass, i);
        layer.SetTexture(tl->Load(t));
        if (separateAlpha) {
            jet::String alpha = t;
            alpha.append("__alpha");
            if (tl->CanLoad(alpha)) layer.SetAlphaTexture(tl->Load(alpha));
        }
    }
}
