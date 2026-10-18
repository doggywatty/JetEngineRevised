// core/jet/include/video/renderstate.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERSTATE_H
#define CORE_JET_INCLUDE_VIDEO_RENDERSTATE_H

#include "jenum.h"
#include "core/pstring.h"
#include "video/blendformula.h"

namespace pugi { class xml_node; }

namespace jet::video {
class RenderState {  // sizeof 0x28
    friend class RenderPass;
    friend class Material;
public:
    // CompareFuncDef/StencilOpDef verified against libdespicablemefree.so 0.0.3:
    // RenderState::GetCompareFunc's string table (0x61c0d8: less/lequal/equal/gequal/greater/
    // notequal(alias "different")/always/never) and GetStencilOp's (0x61c220: keep/zero(alias
    // "clear")/replace/invert/increment/decrement/incrementWrap/decrementWrap).
    // FaceDef/WindingDef are not individually confirmed the same way (no string parser found for
    // them); the natural two/three-way GL ordering is used.
    // UploadRenderState's glCullFace table (0xc95070): GL_BACK, GL_FRONT
    struct FaceDef { enum type { Back, Front, FrontAndBack }; };
    struct WindingDef { enum type { CW, CCW }; };
    struct CompareFuncDef {
        enum type { Less, LEqual, Equal, GEqual, Greater, NotEqual, Always, Never };
    };
    struct StencilOpDef {
        enum type { Keep, Zero, Replace, Invert, Increment, Decrement, IncrementWrap, DecrementWrap };
    };

    RenderState(const RenderState&);
    RenderState();
    RenderState &operator=(const RenderState&);
    bool operator==(const RenderState&) const;
    bool operator!=(const RenderState&) const;
    void SetBlending(bool);
    bool GetBlending() const;
    void SetBlendFormula(const BlendFormula&);
    const BlendFormula &GetBlendFormula() const;
    unsigned char GetColorMask() const;
    void SetColorMask(unsigned char);
    void SetColorMask(bool, bool, bool, bool);
    void SetCulling(bool);
    bool GetCulling() const;
    void SetCullFace(safe_enum<FaceDef,unsigned char>);
    safe_enum<FaceDef,unsigned char> GetCullFace() const;
    void SetWinding(safe_enum<WindingDef,unsigned char>);
    safe_enum<WindingDef,unsigned char> GetWinding() const;
    void SetDepthTest(bool);
    bool GetDepthTest() const;
    void SetDepthFunc(safe_enum<CompareFuncDef,unsigned char>);
    safe_enum<CompareFuncDef,unsigned char> GetDepthFunc() const;
    void SetDepthWrite(bool);
    bool GetDepthWrite() const;
    void SetStencilTest(bool);
    bool GetStencilTest() const;
    void SetStencilFunc(safe_enum<CompareFuncDef,unsigned char>, int, unsigned);
    safe_enum<CompareFuncDef,unsigned char> GetStencilFunc() const;
    void SetStencilRef(int);
    int GetStencilRef() const;
    void SetStencilMask(unsigned);
    unsigned GetStencilMask() const;
    void SetStencilOp(safe_enum<FaceDef,unsigned char>, safe_enum<StencilOpDef,unsigned char>, safe_enum<StencilOpDef,unsigned char>, safe_enum<StencilOpDef,unsigned char>);
    void SetStencilOp(safe_enum<StencilOpDef,unsigned char>, safe_enum<StencilOpDef,unsigned char>, safe_enum<StencilOpDef,unsigned char>);
    void SetStencilFrontPassOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilFrontPassOp() const;
    void SetStencilFrontSFailOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilFrontSFailOp() const;
    void SetStencilFrontZFailOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilFrontZFailOp() const;
    void SetStencilBackPassOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilBackPassOp() const;
    void SetStencilBackSFailOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilBackSFailOp() const;
    void SetStencilBackZFailOp(safe_enum<StencilOpDef,unsigned char>);
    safe_enum<StencilOpDef,unsigned char> GetStencilBackZFailOp() const;
    void SetScissoring(bool);
    bool GetScissoring() const;
    void SetPolygonOffset(float, float);
    void SetPolygonOffset(bool);
    bool GetPolygonOffset() const;
    void SetPolygonOffsetFactor(float);
    float GetPolygonOffsetFactor() const;
    void SetPolygonOffsetUnits(float);
    float GetPolygonOffsetUnits() const;
    void SetLineWidth(float);
    float GetLineWidth() const;
protected:
    void Load(const pugi::xml_node&);
    unsigned GetFlags() const;
    void LoadV100(const pugi::xml_node&);
    safe_enum<BlendFormula::FactorDef,unsigned char> GetBlendFactor(const jet::String&, safe_enum<BlendFormula::FactorDef,unsigned char>);
    safe_enum<CompareFuncDef,unsigned char> GetCompareFunc(const jet::String&, safe_enum<CompareFuncDef,unsigned char>) const;
    safe_enum<StencilOpDef,unsigned char> GetStencilOp(const jet::String&, safe_enum<StencilOpDef,unsigned char>) const;
    bool GetFlag(int) const;
    void SetFlag(int);
    void UnsetFlag(int);
private:
    enum Flag {
        FlagBlending = 1, FlagCulling = 2, FlagDepthTest = 4, FlagDepthWrite = 8,
        FlagStencilTest = 0x10, FlagScissoring = 0x20, FlagPolygonOffset = 0x40
    };
    unsigned char m_flags;
    safe_enum<FaceDef,unsigned char> m_cullFace;
    safe_enum<WindingDef,unsigned char> m_winding;
    safe_enum<CompareFuncDef,unsigned char> m_depthFunc;
    safe_enum<CompareFuncDef,unsigned char> m_stencilFunc;
    unsigned char m_colorMask;
    safe_enum<StencilOpDef,unsigned char> m_frontStencilPassOp;
    safe_enum<StencilOpDef,unsigned char> m_frontStencilsFailOp;
    safe_enum<StencilOpDef,unsigned char> m_frontStencilzFailOp;
    safe_enum<StencilOpDef,unsigned char> m_backStencilPassOp;
    safe_enum<StencilOpDef,unsigned char> m_backStencilsFailOp;
    safe_enum<StencilOpDef,unsigned char> m_backStencilzFailOp;
    BlendFormula m_blendFormula;
    int m_stencilRef;
    unsigned m_stencilMask;
    float m_polygonOffsetFactor;
    float m_polygonOffsetUnits;
    float m_lineWidth;
    bool m_blendSubtract;  // <blending eq="substract">
public:
    bool IsBlendSubtract() const { return m_blendSubtract; }
};
}

#endif
