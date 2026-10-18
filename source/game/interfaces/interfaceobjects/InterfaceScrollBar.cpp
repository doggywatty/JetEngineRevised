// source/game/interfaces/interfaceobjects/InterfaceScrollBar.cpp
#include "InterfaceScrollBar.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "sprites/sprite.h"
#include "utils/painterclipscope.h"

const jet::String InterfaceScrollBar::k_uiTmplName("ScrollBar");

// @0x452af4
InterfaceScrollBar::InterfaceScrollBar()
    : m_spriteIdx(0), m_trackFrame(0), m_thumbFrame(0), m_scrollPos(1.0f), m_opacity(1.0f), m_thumbSize(0, 0), m_trackTileSize(0, 0), m_horizontal(false) {}

InterfaceScrollBar::InterfaceScrollBar(AuroraLevelData *level, int id) : InterfaceScrollBar() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceScrollBar::~InterfaceScrollBar() {}

// @0x452ba0: sprite 9, track frame 10, thumb frame 11, size 14 15
void InterfaceScrollBar::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), l->GetParamValue(e, 9), l->GetParamValue(e, 10), l->GetParamValue(e, 11),
         math::vec2<float>((float)l->GetParamValue(e, 14), (float)l->GetParamValue(e, 15)));
}

// @0x452c9c
void InterfaceScrollBar::Init(math::vec2<float> pos, int sprite, int trackFrame, int thumbFrame, const math::vec2<float> &size) {
    SetPos(pos);
    m_spriteIdx = sprite;
    m_trackFrame = trackFrame;
    m_thumbFrame = thumbFrame;
    SetSize(size);
    math::vec2<float> offset(0, 0);
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)sprite);
    s->GetFrameLogicalRect(trackFrame, 0, offset, m_trackTileSize);
    s->GetFrameLogicalRect(thumbFrame, 0, offset, m_thumbSize);
}

// @0x452d78
void InterfaceScrollBar::Update(int dt) {
    if (!IsVisible()) return;
    IsActive();
    UpdateChildren(dt);
}

// @0x452dc0: the track tiles fill the bar, the thumb sits at the scroll position
void InterfaceScrollBar::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible() || !(m_opacity > 0.0f)) return;
    Sprite *s = AuroraLevelMgr::GetInstance()->GetGraphItemSprite((unsigned)m_spriteIdx);
    PainterClipScope clip(&painter, GetClip(), true);
    math::vec2<float> cp = GetCombinedPos(), size = GetSize();
    PainterClipScope barClip(&painter, math::rect<float>(cp.x, cp.y, cp.x + size.x, cp.y + size.y), true);
    jet::video::Color oldColor = s->GetCurrentColor();
    int alpha = (int)(m_opacity * 255.0f);
    s->SetCurrentColor(jet::video::Color(1.0f, 1.0f, 1.0f, (float)(alpha & 0xff) / 255.0f));
    math::vec2<float> pos = GetPos(), off = GetOffset();
    math::vec2<float> p(off.x + pos.x, off.y + pos.y);
    size = GetSize();
    if (!m_horizontal) {
        math::vec2<float> tile = p;
        for (float rest = size.y; rest > 0.0f; rest -= m_trackTileSize.y) {
            s->PaintFrame(painter, (unsigned)m_trackFrame, tile);
            tile.y += m_trackTileSize.y;
        }
        p.y += (size.y - m_thumbSize.y) * m_scrollPos;
    } else {
        math::vec2<float> tile = p;
        for (float rest = size.x; rest > 0.0f; rest -= m_trackTileSize.x) {
            s->PaintFrame(painter, (unsigned)m_trackFrame, tile);
            tile.x += m_trackTileSize.x;
        }
        p.x += (size.x - m_thumbSize.x) * m_scrollPos;
    }
    s->PaintFrame(painter, (unsigned)m_thumbFrame, p);
    s->SetCurrentColor(oldColor);
    RenderChildren(cam, painter);
}
