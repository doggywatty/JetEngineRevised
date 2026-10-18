// source/game/interfaces/interfaceobjects/InterfaceList.cpp
#include "InterfaceList.h"
#include "InterfaceButton.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "sprites/sprite.h"
#include <cmath>

const jet::String InterfaceList::k_uiTmplName("List");

// @0x44e4d0
InterfaceList::InterfaceList()
    : m_orientation(Vertical), m_enableTouch(false), m_hAlign(0), m_vAlign(0), m_spacing(0), m_scrollOffset(0, 0), m_scrollTarget(0, 0), m_velocity(0),
      m_dragButton(0), m_dragButtonWasPressed(false), m_dragListOpen(false), m_inputRect(0, 0, 0, 0), m_flick(false), m_isDragging(false), m_bgObject(0) {}

InterfaceList::InterfaceList(AuroraLevelData *level, int id) : InterfaceList() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceList::~InterfaceList() {}

// @0x44e5a0: orientation 11, alignments 12 13, clip 14, touch 15, spacing 16
void InterfaceList::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    auto P = [&](int i) { return l->GetParamValue(e, i); };
    Init(math::vec2<float>((float)P(2), (float)P(3)), math::vec2<float>((float)P(9), (float)P(10)), P(11), P(12), P(13), P(16), P(14) == 1, P(15) == 1);
}

// @0x44e6ec
void InterfaceList::Init(math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int spacing, bool enableClip, bool enableTouch) {
    SetPos(pos);
    SetSize(size);
    m_orientation = orientation;
    m_vAlign = vAlign;
    m_hAlign = hAlign;
    m_enableTouch = enableTouch;
    m_spacing = (float)spacing * Sprite::GetDefaultPositionScale();
    if (enableClip) {
        math::vec2<float> p = GetPos(), s = GetSize();
        m_clip = math::rect<float>(p.x, p.y, p.x + s.x, p.y + s.y);
    }
}

// @0x44e80c: the scroll offset eases towards its target
void InterfaceList::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) {
        UpdateTouches();
        float f = m_flick ? 5.0f : 3.5f;
        m_scrollOffset.x += (1.0f / f) * (m_scrollTarget.x - m_scrollOffset.x);
        m_scrollOffset.y += (1.0f / f) * (m_scrollTarget.y - m_scrollOffset.y);
        UpdateChildOffsets();
        if (m_bgObject) {
            if (m_dragButton) m_bgObject->SetOffset(m_dragButton->GetOffset());
            m_bgObject->Update(dt);
        }
    }
    UpdateChildren(dt);
}

// @0x44e918
void InterfaceList::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    if (!IsVisible()) return;
    InterfaceObject::Render(cam, painter);
    math::vec2<float> off = GetOffset();
    float ox = off.x - m_scrollOffset.x, oy = off.y - m_scrollOffset.y;
    math::vec2<float> pos = GetPos(), size = GetSize();
    m_inputRect = GetTransformedRect(cam, painter.GetPostClipTransform(), math::rect<float>(pos.x + ox, pos.y + oy, pos.x + size.x + ox, pos.y + size.y + oy));
    if (m_bgObject) m_bgObject->Render(cam, painter);
    RenderChildren(cam, painter);
}

// @0x44ea6c: the children but the drag button
void InterfaceList::Reset() {
    for (size_t i = 0; i < m_childObjects.size(); i++)
        if (m_childObjects[i] != m_dragButton) m_childObjects[i]->Reset();
}

// @0x44eab4
void InterfaceList::UpdateTouches() {
    if (!m_enableTouch) return;
    TouchMgr *tm = TouchMgr::GetInstance();
    const Touch *touch = tm->FindTouch(m_inputRect, -1);
    bool dragging;  // a touch moved the list this frame
    if (!m_dragButton) {
        dragging = false;
        if (MenuMgr::GetInstance()->ACTION_GetNumPressedButtonsLastFrame() >= 1 && touch) {
            float d = 0;
            if (m_orientation == Vertical) d = touch->pos.y - touch->startPos.y;
            else if (m_orientation == Horizontal) d = touch->pos.x - touch->startPos.x;
            dragging = std::fabs(d) > 32.0f;
        } else if (MenuMgr::GetInstance()->ACTION_GetNumPressedButtonsLastFrame() < 1) {
            dragging = touch != 0;
        }
    } else {
        if (m_dragButton->GetState() == InterfaceButton::Pressed) m_dragButtonWasPressed = true;
        else if (!touch) m_dragButtonWasPressed = false;
        if (touch) {
            if (!m_dragButtonWasPressed || m_dragListOpen) touch = 0;
        }
        if (m_dragButton->GetState() == InterfaceButton::Released) m_dragListOpen = !m_dragListOpen;
        dragging = touch != 0;
    }
    unsigned state = 0;
    if (dragging) {
        if (m_orientation == Vertical) {
            float dy = touch->pos.y - touch->prevPos.y;
            if (!m_flick || touch->state != Touch::Released) m_velocity = dy;
            else dy = (dy + m_velocity) * 8.0f;
            m_scrollTarget.y += dy;
            if (std::fabs(touch->pos.y - touch->startPos.y) > 16.0f || m_isDragging) {
                m_isDragging = true;
                Reset();
            }
        } else if (m_orientation == Horizontal) {
            m_scrollTarget.x = (touch->pos.x + m_scrollTarget.x) - touch->prevPos.x;
            if (std::fabs(touch->pos.x - touch->startPos.x) > 16.0f || m_isDragging) {
                m_isDragging = true;
                Reset();
            }
        }
        state = touch->state;
    } else {
        if (!m_isDragging) return;
        const Touch *any = tm->FindTouch(-1);
        if (any) state = any->state;
        else { m_isDragging = false; return; }
    }
    if ((state & 3) != 0) return;
    m_isDragging = false;
}

// @0x44ed00: keeps the scroll inside the list; with a drag button the open list stays anchored to it
void InterfaceList::UpdateScrollOffsetTarget(float width, float height) {
    bool vertical = m_orientation == Vertical;
    if (!vertical && m_orientation != Horizontal) return;
    float &target = vertical ? m_scrollTarget.y : m_scrollTarget.x;
    float &offset = vertical ? m_scrollOffset.y : m_scrollOffset.x;
    float extent = vertical ? height : width;
    int align = vertical ? m_vAlign : m_hAlign;
    InterfaceButton *btn = m_dragButton;
    if (!btn || m_dragListOpen) {
        if (!btn) {
            math::vec2<float> size = GetSize();
            float limit = (vertical ? size.y : size.x) - extent;
            if (target < limit) {
                target = limit;
                if (vertical) offset = limit;
            }
            if (target <= 0.0f) return;
            target = 0.0f;
            if (vertical) offset = 0.0f;
            return;
        }
        if (align == 0) { if (target <= 0.0f) return; }
        else if (align == 2) { if (0.0f <= target) return; }
        else return;
        target = 0.0f;
        m_dragListOpen = btn->GetState() != InterfaceButton::Pressed;
        return;
    }
    math::vec2<float> bs = btn->GetSize();
    float bsz = vertical ? bs.y : bs.x;
    if (align == 0) target = (m_spacing + bsz) - extent;
    else if (align == 2) target = (m_spacing - bsz) + extent;
}

// @0x44efa4: stacks the visible children along the axis
void InterfaceList::UpdateChildOffsets() {
    if (m_orientation != Vertical && m_orientation != Horizontal) return;
    bool vertical = m_orientation == Vertical;
    math::vec2<float> pos = GetPos(), size = GetSize();
    // along the axis: start of the stack; across: where each child goes
    float total = vertical ? GetTotalChildrenHeight() : GetTotalChildrenWidth();
    int alongAlign = vertical ? m_vAlign : m_hAlign;
    int acrossAlign = vertical ? m_hAlign : m_vAlign;
    float posAlong = vertical ? pos.y : pos.x, sizeAlong = vertical ? size.y : size.x;
    float posAcross = vertical ? pos.x : pos.y, sizeAcross = vertical ? size.x : size.y;
    float cursor = posAlong;
    if (alongAlign == 1) { if (total < sizeAlong) cursor = posAlong + (sizeAlong - total) * 0.5f; }
    else if (alongAlign == 2) cursor = (posAlong + sizeAlong) - total;
    else if (alongAlign == 0) cursor = m_spacing + posAlong;
    for (size_t i = 0; i < m_childObjects.size(); i++) {
        InterfaceObject *c = m_childObjects[i];
        if (!c->IsVisible()) continue;
        math::vec2<float> cs = c->GetSize();
        float across = posAcross;
        float csAcross = vertical ? cs.x : cs.y, csAlong = vertical ? cs.y : cs.x;
        if (acrossAlign == 1) across = (posAcross + sizeAcross * 0.5f) - csAcross * 0.5f;
        else if (acrossAlign == 2) across = (posAcross + sizeAcross) - csAcross;
        math::vec2<float> cp = c->GetPos();
        if (vertical) c->SetOffset(math::vec2<float>(across - cp.x, cursor - cp.y));
        else c->SetOffset(math::vec2<float>(cursor - cp.x, across - cp.y));
        cursor += csAlong + m_spacing;
    }
    if (!vertical) {
        // the horizontal total starts at the spacing, the vertical one is measured from the children alone
    }
    UpdateScrollOffsetTarget(vertical ? 0.0f : total, vertical ? total : 0.0f);
}

math::vec2<float> InterfaceList::GetOffset() {
    math::vec2<float> o = InterfaceObject::GetOffset();
    return math::vec2<float>(m_scrollOffset.x + o.x, m_scrollOffset.y + o.y);
}

math::vec2<float> InterfaceList::GetOffsetForClip() { return InterfaceObject::GetOffset(); }

// @0x43f594: drawn by the list itself, not as a child
void InterfaceList::SetBGObject(InterfaceObject *o) {
    m_bgObject = o;
    if (o) o->SetParent(0);
}

// @0x43f5ac
float InterfaceList::GetTotalChildrenWidth() const {
    float w = m_spacing;
    for (size_t i = 0; i < m_childObjects.size(); i++)
        if (m_childObjects[i]->IsVisible()) w += m_childObjects[i]->GetSize().x + m_spacing;
    return w;
}

float InterfaceList::GetTotalChildrenHeight() const {
    float h = m_spacing;
    for (size_t i = 0; i < m_childObjects.size(); i++)
        if (m_childObjects[i]->IsVisible()) h += m_childObjects[i]->GetSize().y + m_spacing;
    return h;
}

// @0x43f6b4
void InterfaceList::ResetScrollOffset() {
    m_scrollOffset = math::vec2<float>(0, 0);
    m_scrollTarget = math::vec2<float>(0, 0);
}

// @0x43f6cc
void InterfaceList::CloseIfDragListOpen() {
    if (!m_dragListOpen) return;
    m_dragListOpen = false;
    ResetScrollOffset();
}
