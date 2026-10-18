// core/jet/include/text/texter.h
#ifndef CORE_JET_INCLUDE_TEXT_TEXTER_H
#define CORE_JET_INCLUDE_TEXT_TEXTER_H

namespace jet::text2 {
class Texter {  // sizeof 0x5C
public:
    Texter(const jet::text2::Texter::Style&);
    Texter();
    void SetWrapOnAnyChar(bool);
    math::vec2<float> DrawString(jet::video::Painter&, const char*, const math::vec2<float>&, float, safe_enum<jet::text2::Texter::AlignmentDef,enum jet::text2::Texter::AlignmentDef::type>);
    math::vec2<float> DrawString(jet::video::Painter&, const char*, const math::vec2<float>&, safe_enum<jet::text2::Texter::AlignmentDef,enum jet::text2::Texter::AlignmentDef::type>);
protected:
    math::vec2<float> ComputeSize(const std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&);
    math::vec2<float> ComputeSize(std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&, float);
public:
    math::vec2<float> ComputeSize(const char*, float);
    math::vec2<float> ComputeSize(const char*);
    const jet::text2::Texter::Style &GetStyle();
    void SetStyle(const jet::text2::Texter::Style&);
protected:
    float GetLineWidth(unsigned, const std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&);
    math::vec2<float> Execute(jet::text2::Texter::Params&, const math::vec2<float>&, unsigned, const std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&);
    unsigned WordWrap(std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&, float);
    unsigned ComputeLineCount(std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&);
    jet::text2::Texter::Style m_style;  // 0x0
    bool m_wrapOnAnyChar;  // 0x30
    std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> > m_glyphs;  // 0x34
    std::vector<boost::shared_ptr<jet::video::Texture>,std::allocator<boost::shared_ptr<jet::video::Texture> > > m_pages;  // 0x48
};
}

namespace jet::text {
class Texter {  // sizeof 0x30
public:
    Texter(const boost::shared_ptr<jet::text::Font>&, jet::video::Painter*);
    Texter();
    void SetFont(const boost::shared_ptr<jet::text::Font>&);
    const boost::shared_ptr<jet::text::Font> &GetFont();
    void SetPainter(jet::video::Painter*);
    jet::video::Painter *GetPainter();
    void SetWrapOnAnyChar(bool);
    math::vec2<int> DrawString(const char*, const math::vec2<int>&, unsigned, safe_enum<jet::text::Texter::AlignmentDef,enum jet::text::Texter::AlignmentDef::type>);
    math::vec2<int> DrawString(const char*, const math::vec2<int>&, safe_enum<jet::text::Texter::AlignmentDef,enum jet::text::Texter::AlignmentDef::type>);
protected:
    math::vec2<unsigned int> ComputeSize(std::vector<jet::text::Font::GlyphData const *,stack_alloc<jet::text::Font::GlyphData const *,2048> >&, unsigned);
public:
    math::vec2<unsigned int> ComputeSize(const jet::text::Font::GlyphData* const*, unsigned);
    math::vec2<unsigned int> ComputeSize(const char*, unsigned);
    math::vec2<unsigned int> ComputeSize(const char*);
    math::vec2<int> ComputePosition(const std::vector<jet::text::Font::GlyphData const *,stack_alloc<jet::text::Font::GlyphData const *,2048> >&, const math::vec2<int>&, safe_enum<jet::text::Texter::AlignmentDef,enum jet::text::Texter::AlignmentDef::type>);
    bool ComputeGlyphVector(std::vector<jet::text::Font::GlyphData const *,stack_alloc<jet::text::Font::GlyphData const *,2048> >&, const char*);
    void SetDecoration(safe_enum<jet::text::Texter::DecorationDef,enum jet::text::Texter::DecorationDef::type>, unsigned, unsigned);
    void SetNormalStyle();
    void SetShadowStyle(unsigned, unsigned);
    void SetOutlineStyle(unsigned, unsigned);
    void SetStrikeoutThickness(unsigned);
    void SetUnderlineThickness(unsigned);
    void SetColorMode(safe_enum<jet::text::Texter::ColorModeDef,enum jet::text::Texter::ColorModeDef::type>);
    void SetGradientColor(unsigned);
    void SetPrimaryColor(unsigned);
    const jet::text::Texter::Style &GetStyle();
    void SetStyle(const jet::text::Texter::Style&);
protected:
    unsigned GetMaxLineHeight(const jet::text::Font::GlyphData* const*, unsigned);
    unsigned GetLineWidth(const jet::text::Font::GlyphData* const*, unsigned);
    unsigned ComputeHeight(const jet::text::Font::GlyphData* const*, unsigned);
    math::vec2<int> DrawStyled(const jet::text::Texter::Style&, int, int, int, int, unsigned, int, const math::vec2<int>&, const jet::text::Font::GlyphData* const*, unsigned);
    void SimpleWordWrapping(std::vector<jet::text::Font::GlyphData const *,stack_alloc<jet::text::Font::GlyphData const *,2048> >&, unsigned);
    jet::video::Painter *m_painter;  // 0x0
    jet::text::Texter::Style m_style;  // 0x4
    bool m_wrapOnAnyChar;  // 0x2C
};
}

#endif
