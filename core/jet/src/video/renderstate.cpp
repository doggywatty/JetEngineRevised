// core/jet/src/video/renderstate.cpp
#include "video/renderstate.h"
#include "pugixml.hpp"

namespace jet::video {

RenderState::RenderState()
    : m_flags(FlagDepthTest | FlagDepthWrite | FlagCulling), m_cullFace(FaceDef::Back),
      m_winding(WindingDef::CCW), m_depthFunc(CompareFuncDef::Less), m_stencilFunc(CompareFuncDef::LEqual),
      m_colorMask(0xf), m_frontStencilPassOp(StencilOpDef::Keep), m_frontStencilsFailOp(StencilOpDef::Keep),
      m_frontStencilzFailOp(StencilOpDef::Keep), m_backStencilPassOp(StencilOpDef::Keep),
      m_backStencilsFailOp(StencilOpDef::Keep), m_backStencilzFailOp(StencilOpDef::Keep),
      m_stencilRef(1), m_stencilMask(0xffffffff), m_polygonOffsetFactor(0), m_polygonOffsetUnits(0),
      m_lineWidth(1.0f), m_blendSubtract(false) {
}

RenderState::RenderState(const RenderState &o) { *this = o; }

RenderState &RenderState::operator=(const RenderState &o) {
    m_flags = o.m_flags; m_cullFace = o.m_cullFace; m_winding = o.m_winding;
    m_depthFunc = o.m_depthFunc; m_stencilFunc = o.m_stencilFunc; m_colorMask = o.m_colorMask;
    m_frontStencilPassOp = o.m_frontStencilPassOp; m_frontStencilsFailOp = o.m_frontStencilsFailOp;
    m_frontStencilzFailOp = o.m_frontStencilzFailOp; m_backStencilPassOp = o.m_backStencilPassOp;
    m_backStencilsFailOp = o.m_backStencilsFailOp; m_backStencilzFailOp = o.m_backStencilzFailOp;
    m_blendFormula = o.m_blendFormula; m_stencilRef = o.m_stencilRef; m_stencilMask = o.m_stencilMask;
    m_blendSubtract = o.m_blendSubtract;
    m_polygonOffsetFactor = o.m_polygonOffsetFactor; m_polygonOffsetUnits = o.m_polygonOffsetUnits;
    m_lineWidth = o.m_lineWidth;
    return *this;
}

bool RenderState::operator==(const RenderState &o) const {
    return m_flags == o.m_flags && m_cullFace == o.m_cullFace && m_winding == o.m_winding &&
           m_depthFunc == o.m_depthFunc && m_stencilFunc == o.m_stencilFunc && m_colorMask == o.m_colorMask &&
           m_frontStencilPassOp == o.m_frontStencilPassOp && m_frontStencilsFailOp == o.m_frontStencilsFailOp &&
           m_frontStencilzFailOp == o.m_frontStencilzFailOp && m_backStencilPassOp == o.m_backStencilPassOp &&
           m_backStencilsFailOp == o.m_backStencilsFailOp && m_backStencilzFailOp == o.m_backStencilzFailOp &&
           m_blendFormula == o.m_blendFormula && m_stencilRef == o.m_stencilRef && m_stencilMask == o.m_stencilMask &&
           m_polygonOffsetFactor == o.m_polygonOffsetFactor && m_polygonOffsetUnits == o.m_polygonOffsetUnits &&
           m_lineWidth == o.m_lineWidth;
}
bool RenderState::operator!=(const RenderState &o) const { return !(*this == o); }

bool RenderState::GetFlag(int f) const { return (m_flags & f) != 0; }
void RenderState::SetFlag(int f) { m_flags |= f; }
void RenderState::UnsetFlag(int f) { m_flags &= ~f; }
unsigned RenderState::GetFlags() const { return m_flags; }

void RenderState::SetBlending(bool v) { v ? SetFlag(FlagBlending) : UnsetFlag(FlagBlending); }
bool RenderState::GetBlending() const { return GetFlag(FlagBlending); }
void RenderState::SetBlendFormula(const BlendFormula &f) { m_blendFormula = f; }
const BlendFormula &RenderState::GetBlendFormula() const { return m_blendFormula; }

unsigned char RenderState::GetColorMask() const { return m_colorMask; }
void RenderState::SetColorMask(unsigned char m) { m_colorMask = m; }
void RenderState::SetColorMask(bool r, bool g, bool b, bool a) {
    m_colorMask = (r ? 1 : 0) | (g ? 2 : 0) | (b ? 4 : 0) | (a ? 8 : 0);
}

void RenderState::SetCulling(bool v) { v ? SetFlag(FlagCulling) : UnsetFlag(FlagCulling); }
bool RenderState::GetCulling() const { return GetFlag(FlagCulling); }
void RenderState::SetCullFace(safe_enum<FaceDef,unsigned char> v) { m_cullFace = v; }
safe_enum<RenderState::FaceDef,unsigned char> RenderState::GetCullFace() const { return m_cullFace; }
void RenderState::SetWinding(safe_enum<WindingDef,unsigned char> v) { m_winding = v; }
safe_enum<RenderState::WindingDef,unsigned char> RenderState::GetWinding() const { return m_winding; }

void RenderState::SetDepthTest(bool v) { v ? SetFlag(FlagDepthTest) : UnsetFlag(FlagDepthTest); }
bool RenderState::GetDepthTest() const { return GetFlag(FlagDepthTest); }
void RenderState::SetDepthFunc(safe_enum<CompareFuncDef,unsigned char> v) { m_depthFunc = v; }
safe_enum<RenderState::CompareFuncDef,unsigned char> RenderState::GetDepthFunc() const { return m_depthFunc; }
void RenderState::SetDepthWrite(bool v) { v ? SetFlag(FlagDepthWrite) : UnsetFlag(FlagDepthWrite); }
bool RenderState::GetDepthWrite() const { return GetFlag(FlagDepthWrite); }

void RenderState::SetStencilTest(bool v) { v ? SetFlag(FlagStencilTest) : UnsetFlag(FlagStencilTest); }
bool RenderState::GetStencilTest() const { return GetFlag(FlagStencilTest); }
void RenderState::SetStencilFunc(safe_enum<CompareFuncDef,unsigned char> f, int ref, unsigned mask) {
    m_stencilFunc = f; m_stencilRef = ref; m_stencilMask = mask;
}
safe_enum<RenderState::CompareFuncDef,unsigned char> RenderState::GetStencilFunc() const { return m_stencilFunc; }
void RenderState::SetStencilRef(int v) { m_stencilRef = v; }
int RenderState::GetStencilRef() const { return m_stencilRef; }
void RenderState::SetStencilMask(unsigned v) { m_stencilMask = v; }
unsigned RenderState::GetStencilMask() const { return m_stencilMask; }

void RenderState::SetStencilOp(safe_enum<FaceDef,unsigned char> face, safe_enum<StencilOpDef,unsigned char> sfail,
                                safe_enum<StencilOpDef,unsigned char> zfail, safe_enum<StencilOpDef,unsigned char> pass) {
    if (face == FaceDef::Front || face == FaceDef::FrontAndBack) {
        m_frontStencilsFailOp = sfail; m_frontStencilzFailOp = zfail; m_frontStencilPassOp = pass;
    }
    if (face == FaceDef::Back || face == FaceDef::FrontAndBack) {
        m_backStencilsFailOp = sfail; m_backStencilzFailOp = zfail; m_backStencilPassOp = pass;
    }
}
void RenderState::SetStencilOp(safe_enum<StencilOpDef,unsigned char> sfail, safe_enum<StencilOpDef,unsigned char> zfail,
                                safe_enum<StencilOpDef,unsigned char> pass) {
    SetStencilOp(safe_enum<FaceDef,unsigned char>(FaceDef::FrontAndBack), sfail, zfail, pass);
}

void RenderState::SetStencilFrontPassOp(safe_enum<StencilOpDef,unsigned char> v) { m_frontStencilPassOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilFrontPassOp() const { return m_frontStencilPassOp; }
void RenderState::SetStencilFrontSFailOp(safe_enum<StencilOpDef,unsigned char> v) { m_frontStencilsFailOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilFrontSFailOp() const { return m_frontStencilsFailOp; }
void RenderState::SetStencilFrontZFailOp(safe_enum<StencilOpDef,unsigned char> v) { m_frontStencilzFailOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilFrontZFailOp() const { return m_frontStencilzFailOp; }
void RenderState::SetStencilBackPassOp(safe_enum<StencilOpDef,unsigned char> v) { m_backStencilPassOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilBackPassOp() const { return m_backStencilPassOp; }
void RenderState::SetStencilBackSFailOp(safe_enum<StencilOpDef,unsigned char> v) { m_backStencilsFailOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilBackSFailOp() const { return m_backStencilsFailOp; }
void RenderState::SetStencilBackZFailOp(safe_enum<StencilOpDef,unsigned char> v) { m_backStencilzFailOp = v; }
safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilBackZFailOp() const { return m_backStencilzFailOp; }

void RenderState::SetScissoring(bool v) { v ? SetFlag(FlagScissoring) : UnsetFlag(FlagScissoring); }
bool RenderState::GetScissoring() const { return GetFlag(FlagScissoring); }

void RenderState::SetPolygonOffset(bool v) { v ? SetFlag(FlagPolygonOffset) : UnsetFlag(FlagPolygonOffset); }
bool RenderState::GetPolygonOffset() const { return GetFlag(FlagPolygonOffset); }
void RenderState::SetPolygonOffset(float factor, float units) {
    SetPolygonOffset(factor != 0 || units != 0);
    m_polygonOffsetFactor = factor;
    m_polygonOffsetUnits = units;
}
void RenderState::SetPolygonOffsetFactor(float v) { m_polygonOffsetFactor = v; }
float RenderState::GetPolygonOffsetFactor() const { return m_polygonOffsetFactor; }
void RenderState::SetPolygonOffsetUnits(float v) { m_polygonOffsetUnits = v; }
float RenderState::GetPolygonOffsetUnits() const { return m_polygonOffsetUnits; }
void RenderState::SetLineWidth(float v) { m_lineWidth = v; }
float RenderState::GetLineWidth() const { return m_lineWidth; }

safe_enum<BlendFormula::FactorDef,unsigned char> RenderState::GetBlendFactor(const jet::String &s, safe_enum<BlendFormula::FactorDef,unsigned char> def) {
    using F = BlendFormula::FactorDef;
    if (s.Equals("one") || s.Equals("1")) return safe_enum<F,unsigned char>(F::One);
    if (s.Equals("zero") || s.Equals("0")) return safe_enum<F,unsigned char>(F::Zero);
    if (s.Equals("srcColor")) return safe_enum<F,unsigned char>(F::SrcColor);
    if (s.Equals("invSrcColor")) return safe_enum<F,unsigned char>(F::InvSrcColor);
    if (s.Equals("dstColor")) return safe_enum<F,unsigned char>(F::DstColor);
    if (s.Equals("invDstColor")) return safe_enum<F,unsigned char>(F::InvDstColor);
    if (s.Equals("srcAlpha")) return safe_enum<F,unsigned char>(F::SrcAlpha);
    if (s.Equals("invSrcAlpha")) return safe_enum<F,unsigned char>(F::InvSrcAlpha);
    if (s.Equals("dstAlpha")) return safe_enum<F,unsigned char>(F::DstAlpha);
    if (s.Equals("invDstAlpha")) return safe_enum<F,unsigned char>(F::InvDstAlpha);
    return def;
}

safe_enum<RenderState::CompareFuncDef,unsigned char> RenderState::GetCompareFunc(const jet::String &s, safe_enum<CompareFuncDef,unsigned char> def) const {
    using C = CompareFuncDef;
    if (s.Equals("less")) return safe_enum<C,unsigned char>(C::Less);
    if (s.Equals("lequal")) return safe_enum<C,unsigned char>(C::LEqual);
    if (s.Equals("equal")) return safe_enum<C,unsigned char>(C::Equal);
    if (s.Equals("gequal")) return safe_enum<C,unsigned char>(C::GEqual);
    if (s.Equals("greater")) return safe_enum<C,unsigned char>(C::Greater);
    if (s.Equals("notequal") || s.Equals("different")) return safe_enum<C,unsigned char>(C::NotEqual);
    if (s.Equals("always")) return safe_enum<C,unsigned char>(C::Always);
    if (s.Equals("never")) return safe_enum<C,unsigned char>(C::Never);
    return def;
}

safe_enum<RenderState::StencilOpDef,unsigned char> RenderState::GetStencilOp(const jet::String &s, safe_enum<StencilOpDef,unsigned char> def) const {
    using S = StencilOpDef;
    if (s.Equals("keep")) return safe_enum<S,unsigned char>(S::Keep);
    if (s.Equals("zero") || s.Equals("clear")) return safe_enum<S,unsigned char>(S::Zero);
    if (s.Equals("replace")) return safe_enum<S,unsigned char>(S::Replace);
    if (s.Equals("invert")) return safe_enum<S,unsigned char>(S::Invert);
    if (s.Equals("increment")) return safe_enum<S,unsigned char>(S::Increment);
    if (s.Equals("decrement")) return safe_enum<S,unsigned char>(S::Decrement);
    if (s.Equals("incrementWrap")) return safe_enum<S,unsigned char>(S::IncrementWrap);
    if (s.Equals("decrementWrap")) return safe_enum<S,unsigned char>(S::DecrementWrap);
    return def;
}

// XML attribute names below follow the engine's other .rendertechnique loaders (rendertechnique.cpp)
// and aren't independently confirmed against RenderState::LoadV100's own string table yet.
// LoadV100 @0x6f1730: one child element per state, each with a "val" and extra attributes.
void RenderState::LoadV100(const pugi::xml_node &node) {
    typedef BlendFormula::FactorDef F;
    if (pugi::xml_node n = node.child("blending")) {
        if (pugi::xml_attribute a = n.attribute("val")) SetBlending(jet::String(a.value()).Equals("true"));
        if (pugi::xml_attribute a = n.attribute("src")) {
            safe_enum<F,unsigned char> f = GetBlendFactor(jet::String(a.value()), safe_enum<F,unsigned char>(F::One));
            m_blendFormula = BlendFormula(f, m_blendFormula.GetDstRGB(), f, m_blendFormula.GetDstAlpha());
        }
        if (pugi::xml_attribute a = n.attribute("dst")) {
            safe_enum<F,unsigned char> f = GetBlendFactor(jet::String(a.value()), safe_enum<F,unsigned char>(F::Zero));
            m_blendFormula = BlendFormula(m_blendFormula.GetSrcRGB(), f, m_blendFormula.GetSrcAlpha(), f);
        }
        if (pugi::xml_attribute a = n.attribute("eq")) {
            jet::String e(a.value());
            if (e.Equals("add")) m_blendSubtract = false;
            else if (e.Equals("substract")) m_blendSubtract = true;
        }
    }
    if (pugi::xml_node n = node.child("winding"))
        if (pugi::xml_attribute a = n.attribute("val"))
            SetWinding(safe_enum<WindingDef,unsigned char>(jet::String(a.value()).Equals("ccw") ? WindingDef::CCW : WindingDef::CW));
    if (pugi::xml_node n = node.child("culling")) {
        if (pugi::xml_attribute a = n.attribute("val")) SetCulling(jet::String(a.value()).Equals("true"));
        if (pugi::xml_attribute a = n.attribute("face"))
            SetCullFace(safe_enum<FaceDef,unsigned char>(jet::String(a.value()).Equals("front") ? FaceDef::Front : FaceDef::Back));
    }
    if (pugi::xml_node n = node.child("depthTest"))
        if (pugi::xml_attribute a = n.attribute("val")) SetDepthTest(jet::String(a.value()).Equals("true"));
    if (pugi::xml_node n = node.child("depthWrite"))
        if (pugi::xml_attribute a = n.attribute("val")) SetDepthWrite(jet::String(a.value()).Equals("true"));
    if (pugi::xml_node n = node.child("stencil")) {
        if (pugi::xml_attribute a = n.attribute("val")) SetStencilTest(jet::String(a.value()).Equals("true"));
        if (pugi::xml_attribute a = n.attribute("func")) m_stencilFunc = GetCompareFunc(jet::String(a.value()), m_stencilFunc);
        if (pugi::xml_attribute a = n.attribute("ref")) m_stencilRef = a.as_int();
        if (pugi::xml_attribute a = n.attribute("mask")) m_stencilMask = a.as_uint();
        // "SFail"/"ZFail"/"pass" set both faces; "front*"/"back*" one face
        safe_enum<StencilOpDef,unsigned char> fs = m_frontStencilsFailOp, bs = m_backStencilsFailOp;
        safe_enum<StencilOpDef,unsigned char> fz = m_frontStencilzFailOp, bz = m_backStencilzFailOp;
        safe_enum<StencilOpDef,unsigned char> fp = m_frontStencilPassOp, bp = m_backStencilPassOp;
        if (pugi::xml_attribute a = n.attribute("SFail")) fs = bs = GetStencilOp(jet::String(a.value()), fs);
        if (pugi::xml_attribute a = n.attribute("frontSFail")) fs = GetStencilOp(jet::String(a.value()), fs);
        if (pugi::xml_attribute a = n.attribute("backSFail")) bs = GetStencilOp(jet::String(a.value()), bs);
        if (pugi::xml_attribute a = n.attribute("ZFail")) fz = bz = GetStencilOp(jet::String(a.value()), fz);
        if (pugi::xml_attribute a = n.attribute("frontZFail")) fz = GetStencilOp(jet::String(a.value()), fz);
        if (pugi::xml_attribute a = n.attribute("backZFail")) bz = GetStencilOp(jet::String(a.value()), bz);
        if (pugi::xml_attribute a = n.attribute("pass")) fp = bp = GetStencilOp(jet::String(a.value()), fp);
        if (pugi::xml_attribute a = n.attribute("frontPass")) fp = GetStencilOp(jet::String(a.value()), fp);
        if (pugi::xml_attribute a = n.attribute("backPass")) bp = GetStencilOp(jet::String(a.value()), bp);
        SetStencilOp(safe_enum<FaceDef,unsigned char>(FaceDef::Front), fs, fz, fp);
        SetStencilOp(safe_enum<FaceDef,unsigned char>(FaceDef::Back), bs, bz, bp);
    }
    if (pugi::xml_node n = node.child("polygonOffset")) {
        if (pugi::xml_attribute a = n.attribute("val")) SetPolygonOffset(jet::String(a.value()).Equals("true"));
        float factor = m_polygonOffsetFactor, units = m_polygonOffsetUnits;
        if (pugi::xml_attribute a = n.attribute("factor")) factor = a.as_float();
        if (pugi::xml_attribute a = n.attribute("units")) units = a.as_float();
        SetPolygonOffset(factor, units);
    }
}

void RenderState::Load(const pugi::xml_node &node) {
    pugi::xml_attribute versionAttr = node.attribute("version");
    int version = versionAttr ? versionAttr.as_int() : 100;
    if (version == 100) LoadV100(node);
}

}
