// source/game/interfaces/interfaceobjects/InterfacePBar.cpp
#include "InterfacePBar.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "sprites/sprite.h"
#include "utils/painterclipscope.h"
#include "video/driver.h"
#include <algorithm>
#include <cmath>

const jet::String InterfacePBar::k_uiTmplName("PBar");

// @0x45171c
InterfacePBar::InterfacePBar()
    : m_firstValue(true), m_smoothValue(false), m_value(0), m_valueStep(0), m_valueCurrent(0), m_extra1Value(1), m_extra2Value(1), m_valueToDraw(1),
      m_extra1ValueToDraw(1), m_extra2ValueToDraw(1), m_logicRectPos(0, 0), m_inertia(false), m_inertiaMultiPerMillisecond(k_defaultInertiaMultiPerMillisecond),
      m_blinkTimer(0), m_usePgtToBlinkColor(false), m_sinusBlink(false), m_blinkColor0(0xffffff), m_blinkColor1(0x3232ff), m_alpha(0xff), m_barType(0),
      m_spriteIdx(0), m_bgAnimId(0), m_barAnimId(0), m_extraBar1AnimId(0), m_extraBar2AnimId(0), m_limitAnimId(-1), m_bgAnim(0), m_barAnim(0),
      m_extraBar1Anim(0), m_extraBar2Anim(0), m_limitAnim(0) {}

InterfacePBar::InterfacePBar(AuroraLevelData *level, int id) : InterfacePBar() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfacePBar::~InterfacePBar() {
    delete m_bgAnim;
    delete m_barAnim;
    delete m_extraBar1Anim;
    delete m_extraBar2Anim;
    delete m_limitAnim;
}

// @0x414b1c
void InterfacePBar::SetValue(float v) {
    if (m_smoothValue && !m_firstValue) {
        m_value = v;
        m_valueStep = v - m_valueCurrent;
        return;
    }
    m_value = v;
    m_firstValue = false;
    m_valueCurrent = v;
}

// @0x4518f4: type 14, sprite 9, animations 10 11 12 13 17
void InterfacePBar::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    auto P = [&](int i) { return l->GetParamValue(e, i); };
    Init(P(14), math::vec2<float>((float)P(2), (float)P(3)), P(9), P(10), P(11), P(12), P(13), P(17), 0);
}

// @0x451a10
void InterfacePBar::Init(int type, math::vec2<float> pos, int sprite, int bgAnim, int barAnim, int extra1Anim, int extra2Anim, int limitAnim, int logicRectIdx) {
    SetPos(pos);
    m_barType = type;
    m_barAnimId = barAnim;
    m_spriteIdx = sprite;
    m_bgAnimId = bgAnim;
    m_extraBar1AnimId = extra1Anim;
    m_extraBar2AnimId = extra2Anim;
    m_limitAnimId = limitAnim;
    m_blinkTimer = 0;
    m_sinusBlink = false;
    m_blinkColor0 = 0xffffff;
    m_blinkColor1 = 0x3232ff;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)sprite);
    math::vec2<float> size(0, 0);
    if (bgAnim >= 0) {
        int frame = s->GetAFrameFrame(bgAnim, 0);
        s->GetFrameLogicalRect(frame, logicRectIdx, m_logicRectPos, size);
        SetSize(size);
    }
    if (m_bgAnimId >= 0) { m_bgAnim = new SpriteAnim(s); m_bgAnim->SetAnim(m_bgAnimId, true); }
    if (m_barAnimId >= 0) { m_barAnim = new SpriteAnim(s); m_barAnim->SetAnim(m_barAnimId, true); }
    if (m_extraBar1AnimId >= 0) { m_extraBar1Anim = new SpriteAnim(s); m_extraBar1Anim->SetAnim(m_extraBar1AnimId, true); }
    if (m_extraBar2AnimId >= 0) { m_extraBar2Anim = new SpriteAnim(s); m_extraBar2Anim->SetAnim(m_extraBar2AnimId, true); }
    if (m_limitAnimId >= 0) { m_limitAnim = new SpriteAnim(s); m_limitAnim->SetAnim(m_limitAnimId, true); }
}

// @0x451c70
void InterfacePBar::Update(int dt) {
    if (IsVisible()) {
        if (IsActive()) {
            if (m_bgAnim) m_bgAnim->UpdateAnim(dt);
            if (m_barAnim) m_barAnim->UpdateAnim(dt);
            if (m_extraBar1Anim) m_extraBar1Anim->UpdateAnim(dt);
            if (m_extraBar2Anim) m_extraBar2Anim->UpdateAnim(dt);
            if (m_limitAnim) m_limitAnim->UpdateAnim(dt);
            if (m_blinkTimer > 0) m_blinkTimer -= dt;
            if (m_smoothValue && m_valueCurrent != m_value) {
                float step = std::min(std::max(m_valueStep * 0.002f * (float)dt, 0.0f), m_valueStep);
                float v = step + m_valueCurrent;
                if (m_value <= v) v = m_value;
                m_valueCurrent = v;
            }
            if (!m_inertia) {
                m_valueToDraw = m_valueCurrent;
                m_extra1ValueToDraw = m_extra1Value;
                m_extra2ValueToDraw = m_extra2Value;
            } else {
                float f = (float)dt * m_inertiaMultiPerMillisecond;
                m_extra2ValueToDraw += f * (m_extra2Value - m_extra2ValueToDraw);
                m_extra1ValueToDraw += f * (m_extra1Value - m_extra1ValueToDraw);
                m_valueToDraw += f * (m_valueCurrent - m_valueToDraw);
            }
        }
        UpdateChildren(dt);
    }
}

// @0x452648: the rectangle of the bar between two fill levels
math::rect<float> InterfacePBar::GetPBarRect(float from, float to) {
    math::vec2<float> pos = GetPos(), off = GetOffset(), size = GetSize();
    math::vec2<float> scale = GetScale();
    float x0 = off.x + pos.x + m_logicRectPos.x, y0 = off.y + pos.y + m_logicRectPos.y;
    float x1 = off.x + pos.x + m_logicRectPos.x + scale.x * size.x, y1 = off.y + pos.y + m_logicRectPos.y + scale.y * size.y;
    math::rect<float> r(x0, y0, x1, y1);
    switch (m_barType) {
    case LeftToRight: r.upperLeftCorner.x = x0 + (x1 - x0) * to; r.lowerRightCorner.x = x0 + (x1 - x0) * from; break;
    case RightToLeft: r.lowerRightCorner.x = x1 - (x1 - x0) * to; r.upperLeftCorner.x = x1 - (x1 - x0) * from; break;
    case TopToBottom: r.upperLeftCorner.y = y0 + (y1 - y0) * to; r.lowerRightCorner.y = y0 + (y1 - y0) * from; break;
    case BottomToTop: r.lowerRightCorner.y = y1 - (y1 - y0) * to; r.upperLeftCorner.y = y1 - (y1 - y0) * from; break;
    default: break;
    }
    return r;
}

// @0x4527f4: where the first fill ends
math::vec2<float> InterfacePBar::GetLimitPos(float from, float to) {
    math::rect<float> r = GetPBarRect(from, to);
    math::vec2<float> p(0, 0);
    switch (m_barType) {
    case LeftToRight: p = math::vec2<float>(r.lowerRightCorner.x, r.upperLeftCorner.y); break;
    case RightToLeft: p = math::vec2<float>(r.upperLeftCorner.x, r.upperLeftCorner.y); break;
    case TopToBottom: p = math::vec2<float>(r.upperLeftCorner.x, r.lowerRightCorner.y); break;
    case BottomToTop: p = math::vec2<float>(r.upperLeftCorner.x, r.upperLeftCorner.y); break;
    default: break;
    }
    return p;
}

// @0x451eac
void InterfacePBar::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    PainterClipScope scope(&painter, GetClip(), true);
    jet::video::Color oldColor = s->GetCurrentColor();
    s->SetCurrentColor(jet::video::Color(1, 1, 1, (float)m_alpha / 255.0f));
    math::mat3<float> m;
    math::vec2<float> scale = GetCombinedScale();
    if (!math::isOne(scale, 1.1920929e-07f)) m.postScale(scale);
    math::vec2<float> pos = GetPos(), off = GetOffset();
    m.m[6] = off.x + pos.x;
    m.m[7] = off.y + pos.y;
    if (m_bgAnim) m_bgAnim->RenderAnim(painter, m);
    if (m_blinkTimer > 0 || m_usePgtToBlinkColor || m_sinusBlink) {
        float t = m_valueToDraw;
        if (m_blinkTimer > 0) t = (float)m_blinkTimer * 0.002f;
        if (m_sinusBlink) t = std::fabs(sinf(jet::video::Driver::GetCurrent()->GetTime() * 6.2831855f));
        unsigned c0 = (unsigned)m_blinkColor0, c1 = (unsigned)m_blinkColor1;
        unsigned r0 = (c0 & 0xffffff) >> 16, g0 = (c0 & 0xffff) >> 8, b0 = c0 & 0xff;
        float dr = (float)((int)((c1 & 0xffffff) >> 16) - (int)r0), dg = (float)((int)((c1 & 0xffff) >> 8) - (int)g0), db = (float)((int)(c1 & 0xff) - (int)(c0 & 0xff));
        unsigned r = r0 + (int)(t * dr), g = g0 + (int)(t * dg), b = b0 + (int)(t * db);
        s->SetCurrentColor(jet::video::Color((float)(r & 0xff) / 255.0f, (float)(g & 0xff) / 255.0f, (float)(b & 0xff) / 255.0f, (float)m_alpha / 255.0f));
        Sprite::SetOverridenRenderTechnique(jet::String("sprite2d_color"));
    }
    if (m_barType == FrameStepped) {
        float frames = (float)(s->GetNumberOfAnimationFrames(m_barAnimId) - 1);
        s->PaintFrame(painter, (unsigned)s->GetAFrameFrame(m_barAnimId, (int)(frames * m_valueToDraw)), m);
    }
    if (m_barType == RadialFill || m_barType == RadialEmpty) {
        Sprite::SetOverridenRenderTechnique(jet::String("_progressBar2d"));
        float minA, maxA;
        if (m_barType == RadialEmpty) { minA = (1.0f - m_valueToDraw) * 6.2831855f; maxA = 6.2831855f; }
        else { maxA = m_valueToDraw * 6.2831855f; minA = 0.0f; }
        s->UpdateProgressBarUniforms((unsigned)s->GetAFrameFrameFirstModule(m_barAnimId, 0), minA, maxA);
        s->PaintFrame(painter, (unsigned)s->GetAFrameFrame(m_barAnimId, 0), m);
    } else if (m_barType == NeedleUp || m_barType == NeedleDown) {
        float f = m_valueToDraw;
        if (m_barType == NeedleDown) f = 1.0f - f;
        float angle = 1.5707964f + f * -3.1415927f;
        if (m_barAnim) {
            math::mat4<float> old = painter.GetTransform();
            math::mat4<float> t;
            math::quat<float> q;
            q.setFromEulerXYZ(math::vec3<float>(0, 0, angle));
            t.setRotationPart(q.getAsMat3());
            math::vec2<float> p = GetPos(), o = GetOffset();
            t.m[12] = o.x + p.x;
            t.m[13] = o.y + p.y;
            t.m[14] = 0.0f;
            painter.SetTransform(t);
            m_barAnim->RenderAnim(painter, math::vec2<float>(0, 0));
            painter.SetTransform(old);
        }
    } else {
        math::rect<float> bar = GetPBarRect(m_valueToDraw, 0);
        math::vec2<float> limit = GetLimitPos(m_valueToDraw, 0);
        PainterClipScope barScope(&painter, bar, true);
        if (m_barAnim) m_barAnim->RenderAnim(painter, m);
        barScope.Pop();
        if (m_extraBar1AnimId >= 0) {
            barScope.Push(GetPBarRect(m_extra1ValueToDraw, m_valueToDraw), true);
            if (m_extraBar1Anim) m_extraBar1Anim->RenderAnim(painter, m);
            barScope.Pop();
            if (m_extraBar2AnimId >= 0) {
                barScope.Push(GetPBarRect(m_extra2ValueToDraw, m_extra1ValueToDraw), true);
                if (m_extraBar2Anim) m_extraBar2Anim->RenderAnim(painter, m);
                barScope.Pop();
            }
        }
        if (m_limitAnimId >= 0 && m_limitAnim) {
            m.m[6] = limit.x;
            m.m[7] = limit.y;
            m_limitAnim->RenderAnim(painter, m);
        }
    }
    Sprite::SetOverridenRenderTechnique(jet::String());
    s->SetCurrentColor(oldColor);
    RenderChildren(cam, painter);
}
