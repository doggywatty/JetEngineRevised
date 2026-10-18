// core/jet/include/video/blendformula.h
#ifndef CORE_JET_INCLUDE_VIDEO_BLENDFORMULA_H
#define CORE_JET_INCLUDE_VIDEO_BLENDFORMULA_H

#include "jenum.h"

namespace jet::video {
class BlendFormula {
public:
    // Verified against libdespicablemefree.so 0.0.3, RenderState::GetBlendFactor's string table
    // (0x61bf28: zero/0, one/1, srcColor, invSrcColor, dstColor, invDstColor, srcAlpha,
    // invSrcAlpha, dstAlpha, invDstAlpha, in that order).
    struct FactorDef {
        enum type { Zero, One, SrcColor, InvSrcColor, DstColor, InvDstColor,
                    SrcAlpha, InvSrcAlpha, DstAlpha, InvDstAlpha };
    };
    // ComboDef constructor @0x681b70: the factors of each combination (source factor for the colour and the alpha, then the
    // destination factor), in the order of the .so's switch.
    struct ComboDef {
        enum type { Opaque, Alpha, Additive, Modulate, DestColorAdd, Screen, ZeroInvSrcColor };
    };

    BlendFormula() : m_srcRGB(FactorDef::One), m_dstRGB(FactorDef::Zero),
                      m_srcAlpha(FactorDef::One), m_dstAlpha(FactorDef::Zero) {}
    BlendFormula(safe_enum<FactorDef,unsigned char> srcRGB, safe_enum<FactorDef,unsigned char> dstRGB,
                 safe_enum<FactorDef,unsigned char> srcA, safe_enum<FactorDef,unsigned char> dstA)
        : m_srcRGB(srcRGB), m_dstRGB(dstRGB), m_srcAlpha(srcA), m_dstAlpha(dstA) {}
    explicit BlendFormula(ComboDef::type combo) { SetCombo(combo); }

    void SetCombo(ComboDef::type combo) {
        safe_enum<FactorDef,unsigned char> src = FactorDef::Zero, dst = FactorDef::Zero;
        switch (combo) {
        case ComboDef::Opaque: src = FactorDef::One; dst = FactorDef::Zero; break;
        case ComboDef::Alpha: src = FactorDef::SrcAlpha; dst = FactorDef::InvSrcAlpha; break;
        case ComboDef::Additive: src = FactorDef::One; dst = FactorDef::One; break;
        case ComboDef::Modulate: src = FactorDef::Zero; dst = FactorDef::SrcColor; break;
        case ComboDef::DestColorAdd: src = FactorDef::DstColor; dst = FactorDef::One; break;
        case ComboDef::Screen: src = FactorDef::One; dst = FactorDef::InvSrcColor; break;
        case ComboDef::ZeroInvSrcColor: src = FactorDef::Zero; dst = FactorDef::InvSrcColor; break;
        }
        m_srcRGB = m_srcAlpha = src;
        m_dstRGB = m_dstAlpha = dst;
    }

    bool operator==(const BlendFormula &o) const {
        return m_srcRGB == o.m_srcRGB && m_dstRGB == o.m_dstRGB && m_srcAlpha == o.m_srcAlpha && m_dstAlpha == o.m_dstAlpha;
    }
    bool operator!=(const BlendFormula &o) const { return !(*this == o); }

    bool IsSolid() const { return m_srcRGB == FactorDef::One && m_dstRGB == FactorDef::Zero; }

    void SetSrcRGB(safe_enum<FactorDef,unsigned char> v) { m_srcRGB = v; }
    safe_enum<FactorDef,unsigned char> GetSrcRGB() const { return m_srcRGB; }
    void SetDstRGB(safe_enum<FactorDef,unsigned char> v) { m_dstRGB = v; }
    safe_enum<FactorDef,unsigned char> GetDstRGB() const { return m_dstRGB; }
    void SetSrcAlpha(safe_enum<FactorDef,unsigned char> v) { m_srcAlpha = v; }
    safe_enum<FactorDef,unsigned char> GetSrcAlpha() const { return m_srcAlpha; }
    void SetDstAlpha(safe_enum<FactorDef,unsigned char> v) { m_dstAlpha = v; }
    safe_enum<FactorDef,unsigned char> GetDstAlpha() const { return m_dstAlpha; }

private:
    safe_enum<FactorDef,unsigned char> m_srcRGB;
    safe_enum<FactorDef,unsigned char> m_dstRGB;
    safe_enum<FactorDef,unsigned char> m_srcAlpha;
    safe_enum<FactorDef,unsigned char> m_dstAlpha;
};
}

#endif
