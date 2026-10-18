// source/game/interfaces/interfaceobjects/InterfaceGraph.cpp
#include "InterfaceGraph.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "utils/painterclipscope.h"
#include <cmath>

const jet::String InterfaceGraph::k_uiTmplName("Graph");

// @0x44c740 / @0x44c794
InterfaceGraph::InterfaceGraph() : m_spriteIdx(0), m_frameIdx(0), m_animIdx(0), m_anim(0), m_blending(Sprite::SpriteBlendingDef::Alpha), m_color(0xffffffffu) {}

InterfaceGraph::InterfaceGraph(AuroraLevelData *level, int id) : InterfaceGraph() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceGraph::~InterfaceGraph() { delete m_anim; }

// @0x44c858: blending from parameter 14 (0 opaque, 1 alpha, 2 additive, 3 additive2, 4 multiply)
void InterfaceGraph::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    int x = l->GetParamValue(e, 2), y = l->GetParamValue(e, 3);
    int sprite = l->GetParamValue(e, 9), frame = l->GetParamValue(e, 10), anim = l->GetParamValue(e, 11);
    switch (l->GetParamValue(e, 14)) {
    case 0: m_blending = Sprite::SpriteBlendingDef::Opaque; break;
    case 1: m_blending = Sprite::SpriteBlendingDef::Alpha; break;
    case 2: m_blending = Sprite::SpriteBlendingDef::Additive; break;
    case 3: m_blending = Sprite::SpriteBlendingDef::Additive2; break;
    case 4: m_blending = Sprite::SpriteBlendingDef::Multiply; break;
    default: break;
    }
    Init(math::vec2<float>((float)x, (float)y), sprite, frame, anim);
}

// @0x44c964: size and art offset are the frame's logical rect
void InterfaceGraph::Init(math::vec2<float> pos, int sprite, int frame, int anim) {
    SetPos(pos);
    m_spriteIdx = sprite;
    m_frameIdx = frame;
    m_animIdx = anim;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)sprite);
    if (m_frameIdx < 0) {
        m_anim = new SpriteAnim(s);
        frame = s->GetAFrameFrame(m_animIdx, 0);
        m_anim->SetAnim(m_animIdx, true);
    }
    math::vec2<float> offset(0, 0), size(0, 0);
    s->GetFrameLogicalRect(frame, 0, offset, size);
    SetSpriteArtOffset(offset);
    SetSize(size);
}

// @0x44ca94
void InterfaceGraph::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive() && m_anim) m_anim->UpdateAnim(dt);
    UpdateChildren(dt);
}

// @0x44caf8: the frame (or the animation) with the object's rotation and scale, inside its clip
void InterfaceGraph::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    Sprite::SpriteBlending oldBlending = Sprite::GetBlending();
    Sprite::SetBlending(m_blending);
    jet::video::Color oldColor = s->GetCurrentColor();
    s->SetCurrentColor(jet::video::Color(((m_color >> 16) & 0xff) / 255.0f, ((m_color >> 8) & 0xff) / 255.0f, (m_color & 0xff) / 255.0f, (m_color >> 24) / 255.0f));
    PainterClipScope clip(&painter, GetClip(), true);
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
    if (!m_anim) s->PaintFrame(painter, (unsigned)m_frameIdx, m);
    else m_anim->RenderAnim(painter, m);
    s->SetCurrentColor(oldColor);
    Sprite::SetBlending(oldBlending);
    RenderChildren(cam, painter);
}

bool InterfaceGraph::IsAnimEnded() { return !m_anim || m_anim->IsAnimEnded(); }

void InterfaceGraph::SetAnim(int anim, bool loop) {
    m_animIdx = anim;
    m_anim->SetAnim(anim, loop);
}

// @0x44cd28: a still frame again
void InterfaceGraph::SetFrame(int frame) {
    delete m_anim;
    m_anim = 0;
    Init(GetPos(), m_spriteIdx, frame, -1);
}

int InterfaceGraph::GetCurrentAFrame() { return m_anim->GetAframe(); }
void InterfaceGraph::ResetAnim() { m_anim->ResetAnim(); }
