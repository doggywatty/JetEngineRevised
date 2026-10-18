// source/game/interfaces/interfaceobjects/InterfaceDeviceSpecificAnim.cpp
#include "InterfaceDeviceSpecificAnim.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "utils/painterclipscope.h"
#include <cmath>

const jet::String InterfaceDeviceSpecificAnim::k_uiTmplName("DeviceSpecificAnim");

InterfaceDeviceSpecificAnim::InterfaceDeviceSpecificAnim() : m_spriteIdx(0), m_blending(Sprite::SpriteBlendingDef::Alpha), m_color(0xffffffff), m_anim(-1), m_time(0), m_scale(1) {}

// @0x44bb7c
InterfaceDeviceSpecificAnim::InterfaceDeviceSpecificAnim(AuroraLevelData *level, int id) : InterfaceDeviceSpecificAnim() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceDeviceSpecificAnim::~InterfaceDeviceSpecificAnim() {}

// @0x44bc5c: param 9 sprite, 10 blending (0 opaque, 1 alpha, 2 additive, 3 additive2, 4 multiply)
void InterfaceDeviceSpecificAnim::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    static const Sprite::SpriteBlendingDef::type k_blend[] = {Sprite::SpriteBlendingDef::Opaque, Sprite::SpriteBlendingDef::Alpha, Sprite::SpriteBlendingDef::Additive,
                                                              Sprite::SpriteBlendingDef::Additive2, Sprite::SpriteBlendingDef::Multiply};
    int b = l->GetParamValue(e, 10);
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), l->GetParamValue(e, 9),
         Sprite::SpriteBlending(k_blend[(unsigned)b < 5 ? b : 0]));
}

// @0x44bd34
void InterfaceDeviceSpecificAnim::Init(const math::vec2<float> &pos, int sprite, Sprite::SpriteBlending blending) {
    SetPos(pos);
    m_spriteIdx = sprite;
    m_blending = blending;
    m_anim = -1;
    m_time = 0;
    m_scale = 1;
    _ComputeAnim();
}

// @0x44bd94
void InterfaceDeviceSpecificAnim::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) m_time += dt;
    UpdateChildren(dt);
}

// @0x44bdec
void InterfaceDeviceSpecificAnim::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible() || m_anim < 0) return;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    if (!s) return;
    Sprite::SpriteBlending oldBlending = Sprite::GetBlending();
    Sprite::SetBlending(m_blending);
    jet::video::Color oldColor = s->GetCurrentColor();
    s->SetCurrentColor(jet::video::Color(((m_color >> 16) & 0xff) / 255.0f, ((m_color >> 8) & 0xff) / 255.0f, (m_color & 0xff) / 255.0f, (m_color >> 24) / 255.0f));
    math::mat3<float> m;
    if (std::fabs(m_rotation) > 1.1920929e-07f) {
        float c = cosf(m_rotation), sn = sinf(m_rotation);
        m.m[0] = c; m.m[1] = sn; m.m[3] = -sn; m.m[4] = c;
    }
    math::vec2<float> scale = GetCombinedScale();
    if (!math::isOne(scale, 1.1920929e-07f)) m.postScale(scale);
    math::vec2<float> pos = GetPos(), off = GetOffset();
    m.m[6] = off.x + pos.x;
    m.m[7] = off.y + pos.y;
    s->SetPositionScale(m_scale);
    s->PaintAnim(painter, (unsigned)m_anim, m_time, m);
    s->SetCurrentColor(oldColor);
    Sprite::SetBlending(oldBlending);
    RenderChildren(cam, painter);
}

math::rect<float> InterfaceDeviceSpecificAnim::AnimRect(Sprite *s, int anim) const {
    math::rect<float> r(0, 0, 0, 0);
    if (s->GetAFrameLogicalRectsCount(anim, 0) < 1) s->GetAnimRect(r, (unsigned)anim, math::mat3<float>());
    else s->GetAFrameLogicalRect(anim, 0, 0, r);
    return r;
}

// @0x44bfd0: the animation whose first frame is exactly the screen size, else the one with the
// screen's aspect ratio (scaled to fit), else the closest aspect within 0.1
void InterfaceDeviceSpecificAnim::_ComputeAnim() {
    m_anim = -1;
    m_scale = 1;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    if (!s) return;
    s->SetPositionScale(1);
    math::vec2<float> screen = m_guiLevel->GetDisplaySize();
    const float eps = 1.1920929e-07f;
    int n = s->GetNumberOfAnims();
    for (int i = 0; i < n && m_anim < 0; i++) {
        math::rect<float> r = AnimRect(s, i);
        if (std::fabs(r.getWidth() - screen.x) < eps && std::fabs(r.getHeight() - screen.y) < eps) m_anim = i;
    }
    for (int i = 0; i < n && m_anim < 0; i++) {
        math::rect<float> r = AnimRect(s, i);
        float sc = screen.x / r.getWidth();
        if (std::fabs(sc - screen.y / r.getHeight()) < eps) {
            m_anim = i;
            m_scale = sc;
        }
    }
    for (int i = 0; i < n && m_anim < 0; i++) {
        math::rect<float> r = AnimRect(s, i);
        float sc = screen.x / r.getWidth();
        if (std::fabs(sc - screen.y / r.getHeight()) < 0.1f) {
            m_anim = i;
            m_scale = sc;
        }
    }
}
