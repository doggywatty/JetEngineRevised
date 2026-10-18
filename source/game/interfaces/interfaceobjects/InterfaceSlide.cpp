// source/game/interfaces/interfaceobjects/InterfaceSlide.cpp
#include "InterfaceSlide.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "sprites/sprite.h"
#include "utils/painterclipscope.h"
#include <algorithm>

const jet::String InterfaceSlide::k_uiTmplName("Slide");

// @0x453148
InterfaceSlide::InterfaceSlide()
    : m_state(-1), m_touchId(0), m_spriteIdx(0), m_trackFrame(0), m_thumbFrame(0), m_thumbPressedFrame(0), m_value(0), m_thumbSize(0, 0), m_inputRect(0, 0, 0, 0) {}

InterfaceSlide::InterfaceSlide(AuroraLevelData *level, int id) : InterfaceSlide() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceSlide::~InterfaceSlide() {}

// @0x4531f8: sprite 9, track 10, thumb 11, pressed thumb 12
void InterfaceSlide::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), l->GetParamValue(e, 9), l->GetParamValue(e, 10), l->GetParamValue(e, 11),
         l->GetParamValue(e, 12));
}

// @0x4532c0: the size is the track's, the thumb's from its own frame
void InterfaceSlide::Init(math::vec2<float> pos, int sprite, int trackFrame, int thumbFrame, int thumbPressedFrame) {
    SetPos(pos);
    m_spriteIdx = sprite;
    m_trackFrame = trackFrame;
    m_thumbPressedFrame = thumbPressedFrame;
    m_thumbFrame = thumbFrame;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)sprite);
    math::vec2<float> offset(0, 0), size(0, 0);
    s->GetFrameLogicalRect(trackFrame, 0, offset, size);
    SetSize(size);
    s->GetFrameLogicalRect(thumbFrame, 0, offset, m_thumbSize);
}

// @0x4533ac
void InterfaceSlide::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) UpdateTouches();
    UpdateChildren(dt);
}

// @0x453404
void InterfaceSlide::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    PainterClipScope clip(&painter, GetClip(), true);
    math::vec2<float> pos = GetPos(), off = GetOffset();
    s->PaintFrame(painter, (unsigned)m_trackFrame, math::vec2<float>(off.x + pos.x, off.y + pos.y));
    math::vec2<float> size = GetSize();
    math::vec2<float> thumb((off.x + pos.x + m_value * size.x) - m_thumbSize.x * 0.5f, (off.y + pos.y) - m_thumbSize.y * 0.5f);
    thumb.y += size.y * 0.5f;
    s->PaintFrame(painter, (unsigned)(m_state + 1U < 2 ? m_thumbFrame : m_thumbPressedFrame), thumb);
    off = GetOffset();
    math::vec2<float> p(off.x + thumb.x, off.y + thumb.y);
    m_inputRect = GetTransformedRect(cam, painter.GetPostClipTransform(), math::rect<float>(p.x, p.y, p.x + m_thumbSize.x, p.y + m_thumbSize.y));
    RenderChildren(cam, painter);
}

void InterfaceSlide::Reset() { m_state = -1; }

// @0x45368c: dragging the thumb sets the value from the touch position along the track
void InterfaceSlide::UpdateTouches() {
    const Touch *t = TouchMgr::GetInstance()->FindTouch(m_inputRect, -1);
    if (m_state == -1 && !t) m_state = 0;
    if (m_state == 0) {
        if (t) {
            m_state = 1;
            m_touchId = t->id;
        }
    } else if (m_state == 1) {
        if (!t || m_touchId != t->id || (t->state & Touch::Cancelled)) {
            m_state = 0;
            if (!t) return;
        } else if (t->state & Touch::Released) {
            m_state = 0;
        }
        math::vec2<float> off = GetOffset(), pos = GetPos(), size = GetSize();
        m_value = std::min(std::max(((t->pos.x - off.x) - pos.x) / size.x, 0.0f), 1.0f);
    }
}
