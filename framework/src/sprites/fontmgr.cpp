// framework/src/sprites/fontmgr.cpp
#include "sprites/fontmgr.h"
#include "sprites/sprite.h"
#include "text/fontloader.h"
#include "stream/streammgr.h"
#include "util/clipstack.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using jet::text2::Texter;

FontMgr::FontInfo FontMgr::s_defaultFontInfo;
bool FontMgr::s_is2DRenderingWorldCoords = false;
jet::video::Material FontMgr::s_ttfMaterial;
jet::video::Material FontMgr::s_auroraSpriteMaterial;
unsigned FontMgr::g_minFontSize = 14;
unsigned FontMgr::g_minRasterFontSize = 24;
Texter::Style FontMgr::s_debugFontStyle;
bool FontMgr::s_debugFontStyleInitialized = false;

FontMgr::FontMgr() : m_wrapOnAnyChar(false), m_anyRightToLeft(false) {}
FontMgr::~FontMgr() {}

unsigned FontMgr::GetLoadedFontsCount() { return (unsigned)m_fontsText.size(); }
void FontMgr::ClearFonts() { m_fontsText.clear(); }

FontMgr::FontInfo &FontMgr::GetFont(const jet::String &name) {
    auto it = m_fontsText.find(name.c_str());
    return it == m_fontsText.end() ? s_defaultFontInfo : it->second;
}

bool FontMgr::GetIsRightToLeft(const jet::String &name) { return GetFont(name).rightToLeft; }

void FontMgr::SetWrapOnAnyChar(bool v) {
    m_wrapOnAnyChar = v;
    for (auto &f : m_fontsText) f.second.texter->SetWrapOnAnyChar(v);
}

// @0x576bc8: the glyphs of the usual characters are cached up front
void FontMgr::AddFreeTypeFont(const jet::String &name, const jet::String &file, unsigned height, jet::video::Painter &painter, bool rightToLeft, bool arabicShaper) {
    boost::shared_ptr<jet::text2::Font> font = jet::text::FontLoader::GetInstance()->LoadFreetypeFont(file);
    if (!font) return;
    if (height < g_minRasterFontSize) height = g_minRasterFontSize;
    std::vector<jet::text2::Font::Glyph> glyphs;
    std::vector<boost::shared_ptr<jet::video::Texture> > pages;
    font->GetGlyphs(glyphs, pages, height, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:;/-+,.'!@#$%%^&*(){}[]|\\");
    FontInfo info;
    info.painter = &painter;
    info.font = font;
    info.texter.reset(new Texter());
    info.height = height;
    info.rightToLeft = rightToLeft;
    info.texter->SetWrapOnAnyChar(m_wrapOnAnyChar);
    info.texter->SetRightToLeft(rightToLeft);
    if (arabicShaper) info.texter->SetShaper(boost::shared_ptr<jet::text2::Shaper>(new jet::text2::ArabicShaper()));
    m_fontsText[name.c_str()] = info;
}

// @0x577400: the two techniques the ttf and aurora materials draw with
void FontMgr::Set2DRenderingWorldCoords(bool world) {
    s_is2DRenderingWorldCoords = world;
    s_ttfMaterial.SetRenderTechnique(jet::String(world ? "_font_ttf_world_coords" : "_font_ttf_screen_coords"));
    s_auroraSpriteMaterial.SetRenderTechnique(jet::String(world ? "_default2dusingcamera" : "_default2d"));
    jet::video::Material *mats[2] = { &s_ttfMaterial, &s_auroraSpriteMaterial };
    for (jet::video::Material *m : mats) {
        m->GetRenderState(0).SetCulling(false);
        m->GetRenderState(0).SetBlending(true);
        m->GetRenderState(0).SetBlendFormula(jet::video::BlendFormula(jet::video::BlendFormula::ComboDef::Alpha));
    }
}

void FontMgr::SetupPainterMaterial(const jet::String &name) { GetFont(name).painter->SetMaterial(s_ttfMaterial); }

// @0x576eb4: x, y and scale follow the sprite position scale when asked to
void FontMgr::DrawString(const char *text, float x, float y, safe_enum<Texter::AlignmentDef,Texter::AlignmentDef::type> align, const jet::String &font, const Texter::Style &style, float scale, float maxWidth, bool useWorldScale, const math::rect<float> &clip, const math::mat4<float> &transform) {
    float f = useWorldScale ? Sprite::GetDefaultPositionScale() : 1.0f;
    _DrawString(text, x * f, y * f, align, font, style, scale * f, maxWidth, clip, transform);
}

// @0x576f3c
void FontMgr::_DrawString(const char *text, float x, float y, safe_enum<Texter::AlignmentDef,Texter::AlignmentDef::type> align, const jet::String &fontName, const Texter::Style &style, float scale, float maxWidth, const math::rect<float> &clip, const math::mat4<float> &transform) {
    if (!text || !*text) return;
    FontInfo &info = GetFont(fontName);
    Texter::Style s = style;
    s.strikeoutThickness = (unsigned)(scale * (float)style.strikeoutThickness);
    s.underlineThickness = (unsigned)(scale * (float)style.underlineThickness);
    s.font = info.font;
    s.height = info.height;
    info.texter->SetStyle(s);
    SetupPainterMaterial(fontName);
    jet::video::Painter *painter = info.painter;
    unsigned clipPushed = 0;
    if (!(fabsf(clip.upperLeftCorner.x - clip.lowerRightCorner.x) < 1.1920929e-7f && fabsf(clip.upperLeftCorner.y - clip.lowerRightCorner.y) < 1.1920929e-7f)) {
        painter->GetClipStack().Push(clip, true);
        clipPushed = 1;
    }
    painter->SetClippingEnabled(painter->GetClipStack().GetSize() != 0);
    const math::mat4<float> old = painter->GetTransform();
    math::mat4<float> m;
    if (fabsf(scale - 1.0f) > 1.1920929e-7f) { m.m[0] = scale; m.m[5] = scale; m.m[10] = scale; }
    m.m[12] = x;
    m.m[13] = y;
    painter->SetTransform(m * transform);
    const float wrap = maxWidth > 0 ? maxWidth / scale : 100000000.0f;
    info.texter->DrawString(*painter, text, math::vec2<float>(0, 0), wrap, align);
    painter->SetTransform(old);
    while (clipPushed--) painter->GetClipStack().Pop();
}

// @0x577744
math::vec2<float> FontMgr::GetTextSize(const char *text, const jet::String &fontName, float maxWidth, Texter::Style style) {
    FontInfo &info = GetFont(fontName);
    style.font = info.font;
    style.height = info.height;
    info.texter->SetStyle(style);
    return info.texter->ComputeSize(text, maxWidth);
}

// @0x577bd0: the scale that makes the text fit the width, not below the minimum font size
float FontMgr::ScaleToWidth(const jet::String &fontName, const char *text, float width, Texter::Style style) {
    FontInfo &info = GetFont(fontName);
    style.font = info.font;
    style.height = info.height;
    info.texter->SetStyle(style);
    float w = info.texter->ComputeSize(text).x;
    if (!(w > width)) return 1.0f;
    float s = width / w;
    float h = (float)info.height;
    if ((unsigned)(s * h) < g_minFontSize) s = (float)g_minFontSize / h;
    return s;
}

// @0x577c90
const Texter::Style &FontMgr::GetDebugFontStyle() {
    if (!s_debugFontStyleInitialized) {
        s_debugFontStyleInitialized = true;
        s_debugFontStyle.decoration = Texter::DecorationDef::SHADOW;
        s_debugFontStyle.decorationThickness = 1;
        s_debugFontStyle.primaryColor = 0xffffffff;
        s_debugFontStyle.decorationColor = 0xff000000;
    }
    return s_debugFontStyle;
}

// @0x5777f4: "<lang> <raster size> <min size>" lines, the language's entry wins over the defaults
void FontMgr::LoadFontDevicePolicy(const jet::String &cfg, babel::Lang lang) {
    g_minFontSize = 14;
    g_minRasterFontSize = 24;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(cfg);
    if (!s) return;
    s->Start();
    std::string text;
    unsigned size = s->Size();
    if (size) {
        text.resize(size);
        s->SeekBeg(0);
        text.resize(s->Read(&text[0], size));
    }
    std::map<std::string, std::pair<unsigned, unsigned> > policy;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t e = text.find('\n', pos);
        if (e == std::string::npos) e = text.size();
        std::string line = text.substr(pos, e - pos);
        pos = e + 1;
        char code[64];
        int a = 0, b = 0;
        sscanf(line.c_str(), "%63s %d %d", code, &a, &b);
        policy[code] = std::make_pair((unsigned)a, (unsigned)b);
    }
    jet::String code;
    lang.ToCode(code);
    if (!code.empty()) {
        auto it = policy.find(code.c_str());
        if (it != policy.end()) {
            g_minRasterFontSize = it->second.first;
            g_minFontSize = it->second.second;
        }
    }
    fprintf(stderr, "Font device/language policy: raster %d min %d\n", g_minRasterFontSize, g_minFontSize);
    s->Finish();
}
