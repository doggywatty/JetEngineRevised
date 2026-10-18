// core/jet/include/text2/texter2.h
#ifndef CORE_JET_INCLUDE_TEXT2_TEXTER2_H
#define CORE_JET_INCLUDE_TEXT2_TEXTER2_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "jenum.h"
#include <deque>
#include "text2/font2.h"

namespace jet::video { class Painter; }

namespace jet::text2 {
// converts logical glyph order into display order / contextual forms (right to left scripts)
class Shaper {
public:
    virtual ~Shaper() {}
    virtual void Convert(const std::vector<Font::Glyph>&, std::vector<Font::Glyph>&) = 0;
};

// bidirectional reordering for Arabic: Latin runs are reversed so that a right to left pen draws them
// left to right; neutral characters stay with the run they trail
class ArabicShaper : public Shaper {
public:
    ArabicShaper() {}
    virtual ~ArabicShaper() {}
    virtual void Convert(const std::vector<Font::Glyph>&, std::vector<Font::Glyph>&);
};

class Texter {  // sizeof 0x5C
public:
    struct AlignmentDef {
        enum type { BASELINE = 1, TOP = 2, LEFT = 4, HCENTER = 8, VCENTER = 16, RIGHT = 32, BOTTOM = 64,
                    TOP_LEFT = 6, TOP_RIGHT = 34, TOP_CENTER = 10, BOTTOM_LEFT = 68, BOTTOM_RIGHT = 96,
                    BOTTOM_CENTER = 72, BASELINE_LEFT = 5, BASELINE_RIGHT = 33, BASELINE_CENTER = 9,
                    CENTER_LEFT = 20, CENTER_RIGHT = 48, CENTER = 24 };
    };
    struct DecorationDef { enum type { NONE, SHADOW, OUTLINE }; };
    struct ColorModeDef { enum type { NONE, GRADIENT_LR, GRADIENT_RL, GRADIENT_TB, GRADIENT_BT }; };
    struct Style {  // 0x30
        boost::shared_ptr<Font> font;           // 0x0
        unsigned height;                        // 0x8
        float scale;                            // 0xc
        unsigned decoration;                    // 0x10
        unsigned primaryColor;                  // 0x14 0xAARRGGBB
        unsigned decorationColor;               // 0x18
        unsigned decorationThickness;           // 0x1c
        unsigned strikeoutThickness;            // 0x20
        unsigned underlineThickness;            // 0x24
        unsigned colorMode;                     // 0x28
        unsigned gradientColor;                 // 0x2c
        Style() : height(10), scale(1.0f), decoration(0), primaryColor(0xffffffff), decorationColor(0), decorationThickness(1),
                  strikeoutThickness(0), underlineThickness(0), colorMode(0), gradientColor(0xffffffff) {}
    };

    Texter(const Style&);
    Texter();
    void SetWrapOnAnyChar(bool);
    void SetRightToLeft(bool);
    void SetShaper(boost::shared_ptr<Shaper>);
    math::vec2<float> DrawString(jet::video::Painter&, const char*, const math::vec2<float>&, float maxWidth, safe_enum<AlignmentDef,AlignmentDef::type>) const;
    math::vec2<float> DrawString(jet::video::Painter&, const char*, const math::vec2<float>&, safe_enum<AlignmentDef,AlignmentDef::type>) const;
    math::vec2<float> ComputeSize(const char*, float maxWidth) const;
    math::vec2<float> ComputeSize(const char*) const;
    math::vec2<float> ComputeSize(std::vector<Font::Glyph>&, float maxWidth) const;
    const Style &GetStyle() const;
    void SetStyle(const Style&);

protected:
    struct LineMarker { float x, y; unsigned color; };
    struct Params {
        jet::video::Painter *painter;           // 0x0
        bool onlyOneLine;                       // 0x4
        unsigned color;                         // 0x8
        Style style;                            // 0xc
        unsigned alignment;                     // 0x3c
        unsigned lineCount;                     // 0x40
        float minX, minY, maxX, maxY;           // 0x44 bounds of the pen positions
        Params() : painter(0), onlyOneLine(false), color(0), alignment(AlignmentDef::TOP_LEFT), lineCount(1),
                   minX(-1), minY(-1), maxX(1), maxY(1) {}
    };
    math::vec2<float> ComputeSize(const std::vector<Font::Glyph>&) const;
    float GetLineWidth(unsigned, const std::vector<Font::Glyph>&) const;
    math::vec2<float> Execute(Params&, const math::vec2<float>&, unsigned, const std::vector<Font::Glyph>&) const;
    bool CanWrapHere(unsigned, unsigned, unsigned) const;
    unsigned WordWrap(std::vector<Font::Glyph>&, float) const;
    unsigned ComputeLineCount(const std::vector<Font::Glyph>&) const;

    Style m_style;                                                   // 0x0
    bool m_wrapOnAnyChar;                                            // 0x30
    bool m_rightToLeft;                                              // 0x31
    boost::shared_ptr<Shaper> m_shaper;                              // 0x34
    mutable std::vector<Font::Glyph> m_glyphs;                       // 0x3c
    mutable std::vector<boost::shared_ptr<jet::video::Texture> > m_pages;  // 0x48
};
}

#endif
