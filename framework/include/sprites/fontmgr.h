// framework/include/sprites/fontmgr.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_FONTMGR_H
#define FRAMEWORK_INCLUDE_SPRITES_FONTMGR_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "jsingleton.h"
#include "core/pstring.h"
#include "smath.h"
#include "jenum.h"
#include "video/material.h"
#include "video/painter.h"
#include "text2/texter2.h"
#include "localization/lang.h"

// TrueType text through jet::text2: named fonts (face + pixel height + painter) and the
// screen/world coordinate material they draw with (FontMgr @0x576a34)
class FontMgr : public Singleton<FontMgr> {  // sizeof 0x24
public:
    struct FontInfo {  // 0x1c
        jet::video::Painter *painter;                   // 0x0
        boost::shared_ptr<jet::text2::Texter> texter;   // 0x4
        boost::shared_ptr<jet::text2::Font> font;       // 0xc
        unsigned height;                                // 0x14
        bool rightToLeft;                               // 0x18
        FontInfo() : painter(0), height(0), rightToLeft(false) {}
    };

    FontMgr();
    virtual ~FontMgr();
    unsigned GetLoadedFontsCount();
    void ClearFonts();
    void AddFreeTypeFont(const jet::String &name, const jet::String &file, unsigned height, jet::video::Painter&, bool rightToLeft = false, bool arabicShaper = false);
    void DrawString(const char*, float x, float y, safe_enum<jet::text2::Texter::AlignmentDef,jet::text2::Texter::AlignmentDef::type>, const jet::String &font, const jet::text2::Texter::Style&, float scale, float maxWidth, bool useWorldScale, const math::rect<float> &clip, const math::mat4<float> &transform);
    static void Set2DRenderingWorldCoords(bool);
    static bool Is2DRenderingWorldCoords() { return s_is2DRenderingWorldCoords; }
    static unsigned GetMinFontSize() { return g_minFontSize; }
    math::vec2<float> GetTextSize(const char*, const jet::String &font, float maxWidth, jet::text2::Texter::Style);
    void LoadFontDevicePolicy(const jet::String &cfg, babel::Lang);
    void SetWrapOnAnyChar(bool);
    void SetRightToLeft(bool v) { m_anyRightToLeft = v; }
    bool IsAnyRightToLeft() const { return m_anyRightToLeft; }
    bool GetIsRightToLeft(const jet::String &font);
    float ScaleToWidth(const jet::String &font, const char*, float width, jet::text2::Texter::Style);
    static const jet::text2::Texter::Style &GetDebugFontStyle();
private:
    void _DrawString(const char*, float x, float y, safe_enum<jet::text2::Texter::AlignmentDef,jet::text2::Texter::AlignmentDef::type>, const jet::String &font, const jet::text2::Texter::Style&, float scale, float maxWidth, const math::rect<float> &clip, const math::mat4<float> &transform);
    FontInfo &GetFont(const jet::String&);
    void SetupPainterMaterial(const jet::String &font);
    static FontInfo s_defaultFontInfo;
    std::map<std::string, FontInfo> m_fontsText;  // 0x4
    static bool s_is2DRenderingWorldCoords;
    static jet::video::Material s_ttfMaterial;
    static jet::video::Material s_auroraSpriteMaterial;
    static unsigned g_minFontSize;
    static unsigned g_minRasterFontSize;
    bool m_wrapOnAnyChar;  // 0x1c
    bool m_anyRightToLeft;  // 0x1D: a loaded font reads right to left
    static jet::text2::Texter::Style s_debugFontStyle;
    static bool s_debugFontStyleInitialized;
};

#endif
