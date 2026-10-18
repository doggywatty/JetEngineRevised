// source/game/interfaces/interfaceobjects/InterfaceButton.cpp
#include "InterfaceButton.h"
#include "game/gamemanagers/FontInfoMgr.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "sprites/fontmgr.h"
#include "utils/painterclipscope.h"
#include <cmath>

const jet::String InterfaceButton::k_uiTmplName("Button");
jet::text2::Texter::Style InterfaceButton::s_fontStyle;

// @0x44a5b8
InterfaceButton::InterfaceButton()
    : m_fromAurora(false), m_shadowThickness(0), m_outlineThickness(0), m_strikeoutThickness(0), m_underlineThickness(0), m_state(Untouched),
      m_currTouchID(0), m_spriteIdx(0), m_idleAnimId(0), m_pressedAnimId(0), m_selectedAnimId(0), m_fontId(0), m_palIdle(0), m_palPressed(0),
      m_palSelected(0), m_iconPos(0), m_iconIdleAnim(0), m_iconPressedAnim(0), m_iconSelectedAnim(0), m_vAlign(0), m_hAlign(0), m_textOffset(0),
      m_enableTextClip(false), m_frameTextOff(0, 0), m_frameTextSize(0, 0), m_idleAnim(0), m_pressedAnim(0), m_selectedAnim(0), m_selected(false),
      m_iconSize(0, 0), m_enableTextWrap(false), m_enableTextScaleToWidth(false), m_autoScrollText_animScale(0), m_color(0xffffffffu),
      m_inputRect(0, 0, 0, 0), m_textScale(1, 1) {}

// @0x44a50c
InterfaceButton::InterfaceButton(AuroraLevelData *level, int id) : InterfaceButton() {
    m_guiLevel = level;
    m_guiId = id;
    m_fromAurora = true;
}

InterfaceButton::~InterfaceButton() {
    delete m_idleAnim;
    delete m_pressedAnim;
    delete m_selectedAnim;
}

// @0x3d13d8
void InterfaceButton::SetText(const char *s) { SetText(jet::String(s)); }

void InterfaceButton::SetText(const jet::String &s) {
    if (m_text.Equals(s)) return;
    m_text = s;
    m_auroraText = jet::String();
}

// @0x44a728
void InterfaceButton::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    auto P = [&](int i) { return l->GetParamValue(e, i); };
    m_textScale = math::vec2<float>(1, 1);
    jet::String text;
    int key = P(14);
    if (key >= 0) {
        m_auroraText = AuroraLevelData::GetAuroraStringFromIndex(key);
        text = babel::Babel::GetInstance()->GetStringMgr()->Get(m_auroraText);
    }
    Init(math::vec2<float>((float)P(2), (float)P(3)), P(9), P(10), P(11), P(12), text, P(15), P(16), P(17), P(18), P(19), P(20), P(21), P(22), P(23),
         P(24), P(25), P(27) == 1, P(28) == 1, P(29), P(30), P(31), P(32), P(33) == 1);
}

// @0x44aa9c
void InterfaceButton::Init(math::vec2<float> pos, int sprite, int idleAnim, int pressedAnim, int selectedAnim, const jet::String &text, int font, int palIdle,
                           int palPressed, int palSelected, int iconPos, int iconIdleAnim, int iconPressedAnim, int iconSelectedAnim, int vAlign,
                           int hAlign, int textOffset, bool wrap, bool scaleToWidth, int shadow, int outline, int strikeout, int underline, bool clipText) {
    SetPos(pos);
    m_spriteIdx = sprite;
    m_idleAnimId = idleAnim;
    m_pressedAnimId = pressedAnim;
    m_selectedAnimId = selectedAnim;
    m_text = text;
    m_fontId = font;
    m_palIdle = palIdle;
    m_palPressed = palPressed;
    m_palSelected = palSelected;
    m_iconPos = iconPos;
    m_iconIdleAnim = iconIdleAnim;
    m_iconPressedAnim = iconPressedAnim;
    m_iconSelectedAnim = iconSelectedAnim;
    m_vAlign = vAlign;
    m_hAlign = hAlign;
    m_textOffset = textOffset;
    m_enableTextWrap = wrap;
    m_enableTextScaleToWidth = scaleToWidth;
    m_shadowThickness = shadow;
    m_outlineThickness = outline;
    m_strikeoutThickness = strikeout;
    m_underlineThickness = underline;
    m_enableTextClip = clipText;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    int frame = s->GetAFrameFrame(m_idleAnimId, 0);
    math::vec2<float> offset(0, 0), size(0, 0);
    s->GetFrameLogicalRect(frame, 0, offset, size);
    SetSpriteArtOffset(offset);
    s->GetFrameLogicalRect(frame, 1, m_frameTextOff, m_frameTextSize);
    if (m_iconPos < 0) {
        m_iconSize = math::vec2<float>(0, 0);
    } else {
        int iconFrame = s->GetAFrameFrame(m_iconIdleAnim, 0);
        s->GetFrameLogicalRect(iconFrame, 0, offset, m_iconSize);
        size.x += m_iconSize.x;
    }
    SetSize(size);
    if (m_idleAnimId >= 0) { m_idleAnim = new SpriteAnim(s); m_idleAnim->SetAnim(m_idleAnimId, true); }
    if (m_pressedAnimId >= 0) { m_pressedAnim = new SpriteAnim(s); m_pressedAnim->SetAnim(m_pressedAnimId, true); }
    if (m_selectedAnimId >= 0) { m_selectedAnim = new SpriteAnim(s); m_selectedAnim->SetAnim(m_selectedAnimId, true); }
}

// @0x44ad64: the button takes the place of another element
void InterfaceButton::InitFrom(InterfaceObject *like) { InitFrom(like, like->GetPos(), like->GetSize()); }

void InterfaceButton::InitFrom(InterfaceObject *parent, const math::vec2<float> &pos, const math::vec2<float> &size) {
    SetPos(pos);
    SetSize(size);
    SetParent(parent);
}

// @0x44ae10
void InterfaceButton::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) {
        UpdateTouches();
        if (m_idleAnim) {
            m_autoScrollText_animScale = s_autoScrollText_animScale;
            m_idleAnim->UpdateAnim(dt);
            m_pressedAnim->UpdateAnim(dt);
            m_selectedAnim->UpdateAnim(dt);
        }
    }
    UpdateChildren(dt);
}

// @0x44b8cc: untouched -> pressed (on a touch over the button) -> released, back to idle once the finger is gone
void InterfaceButton::UpdateTouches() {
    math::rect<float> rect = m_inputRect;
    if (HasClip()) rect.clipAgainst(GetClip());
    const Touch *t = TouchMgr::GetInstance()->FindTouch(rect, -1);
    if (m_state == Untouched && !t) m_state = Idle;
    if (m_state == Pressed) {
        if (t && m_currTouchID == t->id && !(t->state & Touch::Cancelled)) {
            if (t->state & Touch::Released) {
                m_state = Released;
                MenuMgr::GetInstance()->ACTION_AddReleasedButton(this);
            } else {
                MenuMgr::GetInstance()->ACTION_AddPressedButton(this);
            }
            return;
        }
    } else if (m_state != Released) {
        if (m_state != Idle || !t) return;
        MenuMgr::GetInstance()->ACTION_AddPressedButton(this);
        m_state = Pressed;
        m_currTouchID = t->id;
        return;
    }
    m_state = Idle;
}

// @0x44b878
void InterfaceButton::Reset() {
    if (!m_auroraText.empty()) m_text = babel::Babel::GetInstance()->GetStringMgr()->Get(m_auroraText);
    m_state = Untouched;
}

// @0x44ba30
void InterfaceButton::SetupFontStyle(int palette, int alpha) {
    if (m_shadowThickness > 0) {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::SHADOW;
        s_fontStyle.decorationThickness = m_shadowThickness;
    } else if (m_outlineThickness > 0) {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::OUTLINE;
        s_fontStyle.decorationThickness = m_outlineThickness;
    } else {
        s_fontStyle.decoration = jet::text2::Texter::DecorationDef::NONE;
    }
    s_fontStyle.primaryColor = GetPaletteColor(palette) | (unsigned)alpha << 24;
    s_fontStyle.strikeoutThickness = m_strikeoutThickness;
    s_fontStyle.underlineThickness = m_underlineThickness;
    s_fontStyle.decorationColor = GetPaletteOutlineColor(palette) | (unsigned)alpha << 24;
}

// @0x44bac4: the Aurora text again, or an empty one
void InterfaceButton::ResetTextToDefault() {
    if (!m_auroraText.empty()) return;
    if (m_guiLevel && m_guiLevel->GetParamValue(m_guiId, 14) >= 0) {
        m_auroraText = AuroraLevelData::GetAuroraStringFromIndex(m_guiLevel->GetParamValue(m_guiId, 14));
        Reset();
        return;
    }
    SetText("");
}

void InterfaceButton::SetAnimLoop(bool loop) {
    m_idleAnim->SetLoop(loop);
    m_pressedAnim->SetLoop(loop);
    m_selectedAnim->SetLoop(loop);
}

void InterfaceButton::SetIdleAnim(int a) { m_idleAnimId = a; if (m_idleAnim) m_idleAnim->SetAnim(a, true); }
void InterfaceButton::SetPressedAnim(int a) { m_pressedAnimId = a; if (m_pressedAnim) m_pressedAnim->SetAnim(a, true); }
void InterfaceButton::SetSelectedAnim(int a) { m_selectedAnimId = a; if (m_selectedAnim) m_selectedAnim->SetAnim(a, true); }

static unsigned AlignFlags(int h, int v, float &x, float &y, float w, float hgt, float off) {
    unsigned f = 0;
    if (h == 1) { f |= jet::text2::Texter::AlignmentDef::HCENTER; x += w * 0.5f; }
    else if (h == 2) { f |= jet::text2::Texter::AlignmentDef::RIGHT; x = (w + x) - off; }
    else if (h == 0) { f |= jet::text2::Texter::AlignmentDef::LEFT; x += off; }
    if (v == 1) { f |= jet::text2::Texter::AlignmentDef::VCENTER; y += hgt * 0.5f; }
    else if (v == 2) { f |= jet::text2::Texter::AlignmentDef::BOTTOM; y = (hgt + y) - off; }
    else if (v == 0) { f |= jet::text2::Texter::AlignmentDef::TOP; y += off; }
    return f;
}

// @0x44af24: the sprite of the current state, the icon frame and the text; the input rect is where it ended up on screen
void InterfaceButton::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    if (m_idleAnim) {
        Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
        math::vec2<float> iconOffset(0, 0);
        jet::video::Color oldColor = s->GetCurrentColor();
        s->SetCurrentColor(jet::video::Color(((m_color >> 16) & 0xff) / 255.0f, ((m_color >> 8) & 0xff) / 255.0f, (m_color & 0xff) / 255.0f, (m_color >> 24) / 255.0f));
        SpriteAnim *anim = m_idleAnim;
        int state = m_state;
        if (m_iconPos == 0) iconOffset = math::vec2<float>(m_iconSize.x, 0);
        int palette = m_palIdle, iconAnim = m_iconIdleAnim;
        if (state > -2) {
            if (state < 1) {
                if (m_selected) {
                    anim = m_selectedAnim;
                    iconAnim = m_iconSelectedAnim;
                    palette = m_palSelected;
                }
            } else if (state == 1) {
                anim = m_pressedAnim;
                iconAnim = m_iconPressedAnim;
                palette = m_palPressed;
            }
        }
        PainterClipScope clip(&painter, GetClip(), true);
        math::mat3<float> m;
        math::vec2<float> scale = GetCombinedScale();
        if (!math::isOne(scale, 1.1920929e-07f)) m.postScale(scale);
        math::vec2<float> pos = GetPos(), off = GetOffset();
        m.m[6] = off.x + pos.x;
        m.m[7] = off.y + pos.y;
        m.postTranslate(iconOffset);
        anim->RenderAnim(painter, m);
        if (m_iconPos != -1) {
            m.m[6] = off.x + pos.x;
            m.m[7] = off.y + pos.y;
            m.postTranslate(iconOffset);
            s->_PaintPrepareRender(painter);
            s->PaintAFrame(painter, (unsigned)iconAnim, 0, m);
        }
        RenderDebug(painter);
        if (!m_text.empty()) {
            FontMgr *fm = FontMgr::GetInstance();
            jet::String font = FontInfoMgr::GetFontName(m_fontId);
            SetupFontStyle(palette, 0xff);
            float tx = m_frameTextOff.x + iconOffset.x, ty = m_frameTextOff.y + iconOffset.y;
            int hAlign = m_hAlign, vAlign = m_vAlign;
            math::rect<float> textClip(0, 0, 0, 0);
            if (m_enableTextClip) {
                float x0 = off.x + pos.x + tx * scale.x, y0 = off.y + pos.y + ty * scale.y;
                textClip = math::rect<float>(x0, y0, x0 + scale.x * m_frameTextSize.x, y0 + scale.y * m_frameTextSize.y);
            }
            PainterClipScope textScope(&painter, textClip, true);
            float textScale, maxWidth;
            if (!m_enableTextWrap) {
                textScale = m_enableTextScaleToWidth ? fm->ScaleToWidth(font, m_text.c_str(), m_frameTextSize.x, s_fontStyle) : 1.0f;
                maxWidth = -1.0f;
            } else {
                textScale = 1.0f;
                maxWidth = m_frameTextSize.x;
            }
            math::vec2<float> ts = fm->GetTextSize(m_text.c_str(), font, maxWidth, s_fontStyle);
            float textW = textScale * ts.x, textH = textScale * ts.y;
            math::vec2<float> scroll(0, 0);
            if (!m_enableTextWrap) {
                if (textW > m_frameTextSize.x) {
                    scroll.x = (-15.0f - textW) * m_autoScrollText_animScale;
                    hAlign = 0;
                }
            } else if (textH > m_frameTextSize.y) {
                vAlign = 0;
                hAlign = 0;
                scroll.y = (-15.0f - textH) * m_autoScrollText_animScale;
            }
            scroll.x *= scale.x;
            scroll.y *= scale.y;
            unsigned flags = AlignFlags(hAlign, vAlign, tx, ty, m_frameTextSize.x, m_frameTextSize.y, (float)m_textOffset);
            tx *= scale.x;
            ty *= scale.y;
            tx += scroll.x + pos.x + off.x;
            ty += scroll.y + pos.y + off.y;
            math::rect<float> noClip(0, 0, 0, 0);
            float drawScale = m_textScale.x * scale.x * textScale;
            math::mat4<float> identity;
            safe_enum<jet::text2::Texter::AlignmentDef, jet::text2::Texter::AlignmentDef::type> align((jet::text2::Texter::AlignmentDef::type)flags);
            fm->DrawString(m_text.c_str(), tx, ty, align, font, s_fontStyle, drawScale, maxWidth, false, noClip, identity);
            if (std::fabs(scroll.x) > 1.1920929e-07f)
                fm->DrawString(m_text.c_str(), tx + (textW + 15.0f) * scale.x, ty, align, font, s_fontStyle, drawScale, maxWidth, false, noClip, identity);
            if (m_enableTextWrap && std::fabs(scroll.y) > 1.1920929e-07f)
                fm->DrawString(m_text.c_str(), tx, ty + (textH + 15.0f) * scale.y, align, font, s_fontStyle, drawScale, maxWidth, false, noClip, identity);
        }
        s->SetCurrentColor(oldColor);
    }
    math::vec2<float> cp = GetCombinedPos(), sz = GetSize();
    m_inputRect = GetTransformedRect(cam, painter.GetPostClipTransform(), math::rect<float>(cp.x, cp.y, cp.x + sz.x, cp.y + sz.y));
    RenderChildren(cam, painter);
}
