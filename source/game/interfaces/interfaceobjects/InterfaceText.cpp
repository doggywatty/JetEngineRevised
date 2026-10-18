// source/game/interfaces/interfaceobjects/InterfaceText.cpp
#include "InterfaceText.h"
#include "game/gamemanagers/FontInfoMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "sprites/fontmgr.h"
#include "utils/painterclipscope.h"
#include <cmath>

const jet::String InterfaceText::k_uiTmplName("Text");
jet::text2::Texter::Style InterfaceText::s_fontStyle;

// @0x4537d0 / @0x453854
InterfaceText::InterfaceText()
    : m_wrapAtDefaultWidth(false), m_fontIdx(0), m_vAlign(0), m_hAlign(0), m_palIdx(0), m_shadowThickness(0), m_outlineThickness(0),
      m_strikeoutThickness(0), m_underlineThickness(0), m_enableTextClip(false), m_alpha(0xff), m_enableWrap(false), m_enableScaleToWidth(false),
      m_autoScrollText_animScale(0), m_fixedScale(0), m_enableAutoScroll(true) {}

InterfaceText::InterfaceText(AuroraLevelData *level, int id) : InterfaceText() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceText::~InterfaceText() {}

// @0x4538c0 SetText @0x3d07b4: a new text replaces the key
void InterfaceText::SetText(const char *s) { SetText(jet::String(s)); }

void InterfaceText::SetText(const jet::String &s) {
    if (m_text.Equals(s)) return;
    m_text = s;
    m_auroraText = jet::String();
}

// @0x45394c: the text is the Aurora string of parameter 14 looked up in the localization
void InterfaceText::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    m_fixedScale = 0;
    int x = l->GetParamValue(e, 2), y = l->GetParamValue(e, 3), w = l->GetParamValue(e, 9), h = l->GetParamValue(e, 10), font = l->GetParamValue(e, 11);
    int key = l->GetParamValue(e, 14);
    int vAlign = l->GetParamValue(e, 12), hAlign = l->GetParamValue(e, 13), palette = l->GetParamValue(e, 15), shadow = l->GetParamValue(e, 16);
    int rotation = l->GetParamValue(e, 19), wrap = l->GetParamValue(e, 20), scaleToWidth = l->GetParamValue(e, 21);
    int outline = l->GetParamValue(e, 22), strikeout = l->GetParamValue(e, 23), underline = l->GetParamValue(e, 24), clip = l->GetParamValue(e, 25);
    jet::String text;
    if (key >= 0) {
        m_auroraText = AuroraLevelData::GetAuroraStringFromIndex(key);
        text = babel::Babel::GetInstance()->GetStringMgr()->Get(m_auroraText);
    }
    Init(math::vec2<float>((float)x, (float)y), w, h, font, text, vAlign, hAlign, palette, (float)rotation * 0.017453292f, shadow,
         outline, strikeout, underline, wrap == 1, scaleToWidth == 1, clip == 1);
}

// @0x453bec
void InterfaceText::Init(math::vec2<float> pos, int w, int h, int font, const jet::String &text, int vAlign, int hAlign, int palette, float rotationRad,
                         int shadow, int outline, int strikeout, int underline, bool wrap, bool scaleToWidth, bool clipToBox) {
    SetPos(pos);
    SetSize(math::vec2<float>((float)w, (float)h));
    m_fontIdx = font;
    m_text = text;
    m_vAlign = vAlign;
    m_hAlign = hAlign;
    m_palIdx = palette;
    m_shadowThickness = shadow;
    m_outlineThickness = outline;
    m_strikeoutThickness = strikeout;
    m_underlineThickness = underline;
    m_rotation = rotationRad;
    m_enableWrap = wrap;
    m_enableScaleToWidth = scaleToWidth;
    m_enableTextClip = clipToBox;
}

// @0x453ccc
void InterfaceText::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) m_autoScrollText_animScale = s_autoScrollText_animScale;
    UpdateChildren(dt);
}

// @0x4544ec
void InterfaceText::Reset() {
    if (m_auroraText.empty()) return;
    m_text = babel::Babel::GetInstance()->GetStringMgr()->Get(m_auroraText);
}

// @0x454538
void InterfaceText::SetupFontStyle() {
    if (m_shadowThickness > 0) {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::SHADOW;
        s_fontStyle.decorationThickness = m_shadowThickness;
    } else if (m_outlineThickness > 0) {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::OUTLINE;
        s_fontStyle.decorationThickness = m_outlineThickness;
    } else {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::NONE;
    }
    s_fontStyle.primaryColor = GetPaletteColor(m_palIdx) | (unsigned)m_alpha << 24;
    s_fontStyle.decorationColor = GetPaletteOutlineColor(m_palIdx) | (unsigned)m_alpha << 24;
    s_fontStyle.strikeoutThickness = m_strikeoutThickness;
    s_fontStyle.underlineThickness = m_underlineThickness;
}

// the scale that fits the text on one line and the width the text may wrap at
static void TextScale(FontMgr *fm, const jet::String &font, const char *text, bool wrap, bool scaleToWidth, float boxWidth, jet::text2::Texter::Style &style,
                      float &scale, float &maxWidth) {
    if (wrap) {
        scale = 1.0f;
        maxWidth = boxWidth;
    } else {
        scale = scaleToWidth ? fm->ScaleToWidth(font, text, boxWidth, style) : 1.0f;
        maxWidth = -1.0f;
    }
}

// @0x4545c8
math::vec2<float> InterfaceText::GetTextSize() {
    if (m_text.empty()) return math::vec2<float>(0, 0);
    FontMgr *fm = FontMgr::GetInstance();
    jet::String font = FontInfoMgr::GetFontName(m_fontIdx);
    SetupFontStyle();
    float scale, maxWidth;
    TextScale(fm, font, m_text.c_str(), m_enableWrap, m_enableScaleToWidth, GetUnscaledSize().x, s_fontStyle, scale, maxWidth);
    math::vec2<float> sz = fm->GetTextSize(m_text.c_str(), font, maxWidth, s_fontStyle);
    math::vec2<float> cs = GetCombinedScale();
    return math::vec2<float>(scale * cs.x * sz.x, scale * cs.x * sz.y);
}

// @0x453dcc
void InterfaceText::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    if (!m_text.empty()) {
        FontMgr *fm = FontMgr::GetInstance();
        jet::String font = FontInfoMgr::GetFontName(m_fontIdx);
        SetupFontStyle();
        math::vec2<float> size = GetSize();
        math::vec2<float> pos = GetPos();
        int hAlign = m_hAlign, vAlign = m_vAlign;
        math::rect<float> clip(0, 0, 0, 0);
        if (!m_enableTextClip) {
            if (HasClip()) clip = GetClip();
        } else {
            math::vec2<float> off = GetOffset();
            clip = math::rect<float>(off.x + pos.x, off.y + pos.y, off.x + pos.x + size.x, off.y + pos.y + size.y);
            if (HasClip()) clip.clipAgainst(GetClip());
        }
        float scale, maxWidth;
        TextScale(fm, font, m_text.c_str(), m_enableWrap, m_enableScaleToWidth, GetUnscaledSize().x, s_fontStyle, scale, maxWidth);
        math::vec2<float> ts = fm->GetTextSize(m_text.c_str(), font, maxWidth, s_fontStyle);
        float textW = scale * ts.x, textH = scale * ts.y;
        float scrollX = 0, scrollY = 0;
        if (m_enableAutoScroll) {
            if (!m_enableWrap) {
                if (GetUnscaledSize().x < textW) {
                    scrollX = (-15.0f - textW) * m_autoScrollText_animScale;
                    hAlign = 0;
                }
            } else if (GetUnscaledSize().y < textH) {
                vAlign = 0;
                hAlign = 0;
                scrollY = (-15.0f - textH) * m_autoScrollText_animScale;
            }
        }
        if (fm->GetIsRightToLeft(font)) {
            scrollX = -scrollX;
            if (hAlign == 0) hAlign = 2;
            else if (hAlign == 2) hAlign = 0;
        }
        math::vec2<float> cs = GetCombinedScale();
        scrollX *= cs.x;
        scrollY *= cs.y;
        unsigned flags = 0;
        if (hAlign == 1) { flags |= jet::text2::Texter::AlignmentDef::HCENTER; pos.x += size.x * 0.5f; }
        else if (hAlign == 2) { flags |= jet::text2::Texter::AlignmentDef::RIGHT; pos.x += size.x; }
        else if (hAlign == 0) flags |= jet::text2::Texter::AlignmentDef::LEFT;
        if (vAlign == 1) { flags |= jet::text2::Texter::AlignmentDef::VCENTER; pos.y += size.y * 0.5f; }
        else if (vAlign == 2) { flags |= jet::text2::Texter::AlignmentDef::BOTTOM; pos.y += size.y; }
        else if (vAlign == 0) flags |= jet::text2::Texter::AlignmentDef::TOP;
        math::vec2<float> off = GetOffset();
        pos.x += off.x + scrollX;
        pos.y += off.y + scrollY;
        math::mat4<float> transform;
        if (std::fabs(m_rotation) > 1.1920929e-07f) {
            math::quat<float> q;
            q.setFromEulerXYZ(math::vec3<float>(0, 0, m_rotation));
            transform.setRotationPart(q.getAsMat3());
        }
        math::vec2<float> cs2 = GetCombinedScale();
        float drawScale = m_fixedScale > 0 ? m_fixedScale : scale * cs2.x;
        float drawWidth = m_wrapAtDefaultWidth ? 300.0f : maxWidth;
        safe_enum<jet::text2::Texter::AlignmentDef, jet::text2::Texter::AlignmentDef::type> align(
            (jet::text2::Texter::AlignmentDef::type)flags);
        fm->DrawString(m_text.c_str(), pos.x, pos.y, align, font, s_fontStyle, drawScale, drawWidth, false, clip, transform);
        if (std::fabs(scrollX) > 1.1920929e-07f) {
            fm->DrawString(m_text.c_str(), pos.x + (textW + 15.0f) * cs2.x, pos.y, align, font, s_fontStyle, drawScale, maxWidth, false, clip, transform);
        }
        if (m_enableWrap && std::fabs(scrollY) > 1.1920929e-07f) {
            fm->DrawString(m_text.c_str(), pos.x, pos.y + (textH + 15.0f) * cs2.y, align, font, s_fontStyle, drawScale, maxWidth, false, clip, transform);
        }
    }
    RenderChildren(cam, painter);
}
