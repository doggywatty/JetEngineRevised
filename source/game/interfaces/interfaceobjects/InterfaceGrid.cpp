// source/game/interfaces/interfaceobjects/InterfaceGrid.cpp
#include "InterfaceGrid.h"
#include "InterfaceScrollBar.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "sprites/sprite.h"
#include <cmath>

const jet::String InterfaceGrid::k_uiTmplName("Grid");

// a division by zero gives 0 on the ARM the .so runs on (a grid of columns not set, as the costume grids of the main menu)
static int Div(int a, int b) { return b ? a / b : 0; }

// @0x44ce30
InterfaceGrid::InterfaceGrid()
    : m_orientation(0), m_hAlign(0), m_vAlign(0), m_rows(0), m_columns(0), m_scrollOffset(0, 0), m_scrollTarget(0, 0), m_spacing(0), m_scrollPos(0),
      m_scrollRange(0), m_dragging(false), m_isDragging(false), m_page(0), m_focusIndex(-1), m_inputRect(0, 0, 0, 0), m_mainChild(0), m_scrollBarLayerIdx(-1),
      m_scrollBar(0), m_enableTouch(true) {}

InterfaceGrid::InterfaceGrid(AuroraLevelData *level, int id) : InterfaceGrid() {
    m_guiLevel = level;
    m_guiId = id;
}

InterfaceGrid::~InterfaceGrid() {}

// @0x4c46b0: orientation 11, alignments 12 13, rows 14, columns 15, clip 16, spacing 17, scroll bar 19
void InterfaceGrid::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    auto P = [&](int i) { return l->GetParamValue(e, i); };
    m_scrollBarLayerIdx = P(19);
    Init(math::vec2<float>((float)P(2), (float)P(3)), math::vec2<float>((float)P(9), (float)P(10)), P(11), P(12), P(13), P(14), P(15), P(17), P(16) == 1);
}

// @0x44d078
void InterfaceGrid::Init(math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int rows, int columns, int spacing, bool enableClip) {
    SetPos(pos);
    SetSize(size);
    m_orientation = orientation;
    m_vAlign = vAlign;
    m_hAlign = hAlign;
    m_rows = rows;
    m_columns = columns;
    m_spacing = (float)spacing * Sprite::GetDefaultPositionScale();
    if (enableClip) {
        math::vec2<float> p = GetPos(), s = GetSize();
        m_clip = math::rect<float>(p.x, p.y, p.x + s.x, p.y + s.y);
    }
    m_page = 0;
    m_mainChild = 0;
    m_focusIndex = -1;
    m_enableTouch = true;
}

// @0x44d1bc: snaps the scroll target to the page, eases the offset to it and tells the scroll bar
void InterfaceGrid::Update(int dt) {
    if (!IsVisible() || m_childObjects.empty()) return;
    if (IsActive()) {
        UpdateTouches();
        if (m_orientation == 1) {
            math::vec2<float> cs = m_childObjects[0]->GetSize();
            float rowH = m_spacing + cs.y;
            float top = (float)(-Div(m_page, m_columns)) * rowH;
            if (m_scrollTarget.y > top) m_scrollTarget.y = top;
            if (m_focusIndex >= 0) {
                float bottom = ((float)(-Div(m_focusIndex, m_columns)) + (float)m_rows) * rowH;
                if (m_scrollTarget.y < bottom) m_scrollTarget.y = bottom;
            }
            int count = (int)m_childObjects.size();
            if (m_rows > count) count = m_rows;
            m_scrollRange = (m_spacing + GetSize().y) - (float)count * rowH;
            m_enableTouch = m_scrollRange < 0.0f;
            if (m_scrollRange < 0.0f) {
                if (m_scrollTarget.y < m_scrollRange) m_scrollTarget.y = m_scrollRange;
            } else {
                m_scrollTarget.y = 0.0f;
            }
        } else if (m_orientation == 0) {
            m_enableTouch = true;
            m_scrollRange = 1.0f;
        }
        m_scrollOffset.x += (m_scrollTarget.x - m_scrollOffset.x) * 0.2857143f;
        m_scrollOffset.y += (m_scrollTarget.y - m_scrollOffset.y) * 0.2857143f;
        m_scrollPos = m_orientation == 1 ? m_scrollOffset.y : m_scrollOffset.x;
        UpdateChildOffsets();
        if (m_scrollBar) static_cast<InterfaceScrollBar*>(m_scrollBar)->SetScrollPos(m_scrollPos / m_scrollRange);
    }
    UpdateChildren(dt);
}

// @0x44d3f8
void InterfaceGrid::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    if (!IsVisible()) return;
    InterfaceObject::Render(cam, painter);
    math::vec2<float> off = GetOffset();
    float ox = off.x - m_scrollOffset.x, oy = off.y - m_scrollOffset.y;
    math::vec2<float> pos = GetPos(), size = GetSize();
    m_inputRect = GetTransformedRect(cam, painter.GetPostClipTransform(), math::rect<float>(pos.x + ox, pos.y + oy, pos.x + size.x + ox, pos.y + size.y + oy));
    RenderChildren(cam, painter);
}

void InterfaceGrid::Reset() {
    for (size_t i = 0; i < m_childObjects.size(); i++) m_childObjects[i]->Reset();
}

// @0x44d568
void InterfaceGrid::UpdateTouches() {
    if (!m_enableTouch) return;
    TouchMgr *tm = TouchMgr::GetInstance();
    const Touch *touch = tm->FindTouch(m_inputRect, -1);
    math::vec2<float> cs = m_childObjects[0]->GetSize();
    float stepX = m_spacing + cs.x, stepY = m_spacing + cs.y;
    bool drag = false;
    if (MenuMgr::GetInstance()->ACTION_GetNumPressedButtonsLastFrame() < 1) {
        drag = touch != 0;
    } else if (touch) {
        float d = 0;
        bool known = true;
        if (m_orientation == 1) d = touch->pos.y - touch->startPos.y;
        else if (m_orientation == 0) d = touch->pos.x - touch->startPos.x;
        else known = false;
        drag = !known || std::fabs(d) > 32.0f;
    }
    if (drag) {
        m_dragging = true;
        float travel = 0;
        if (m_orientation == 1) {
            m_scrollTarget.y = (touch->pos.y + m_scrollTarget.y) - touch->prevPos.y;
            travel = std::fabs(touch->pos.y - touch->startPos.y);
        } else if (m_orientation == 0) {
            m_scrollTarget.x = (touch->pos.x + m_scrollTarget.x) - touch->prevPos.x;
            travel = std::fabs(touch->pos.x - touch->startPos.x);
        }
        if (m_orientation == 1 || m_orientation == 0) {
            if (travel > 16.0f || m_isDragging) {
                m_isDragging = true;
                Reset();
            }
        }
        if (m_orientation == 1) m_page = -(int)((int)(m_scrollTarget.y - stepY * 0.5f) / (int)stepY);
        else if (m_orientation == 0) m_page = -(int)((int)(m_scrollTarget.x + stepX * 0.5f) / (int)stepX);
        if (touch->state & 3) return;
        m_isDragging = false;
        return;
    }
    if (m_isDragging) {
        const Touch *any = tm->FindTouch(-1);
        if (!any || !(any->state & 3)) m_isDragging = false;
    }
    m_dragging = false;
    if (!m_childObjects.empty()) {
        if (m_orientation == 0) {
            if (m_scrollTarget.x != m_scrollRange) m_scrollTarget.x = (float)(-m_page) * stepX;
        } else if (m_orientation == 1 && m_scrollTarget.y != m_scrollRange) {
            m_scrollTarget.y = (float)(-m_page) * stepY;
        }
    }
}

// @0x44d864
void InterfaceGrid::UpdateChildOffsets() {
    if (m_childObjects.empty()) return;
    math::vec2<float> cs = m_childObjects[0]->GetSize();
    float cw = cs.x, ch = cs.y;
    int n = (int)m_childObjects.size();
    float totalH = (float)m_rows * (m_spacing + ch) - m_spacing;
    float totalW = (float)m_columns * (m_spacing + cw) - m_spacing;
    int rowsLast, colsLast;
    if (m_orientation == 1) { rowsLast = Div(n, m_columns); colsLast = m_columns; }
    else { colsLast = Div(n, m_rows); rowsLast = m_rows; }
    math::vec2<float> pos = GetPos(), size = GetSize();
    float y0 = pos.y;
    if (totalH < size.y) {
        if (m_vAlign == 1) y0 += (size.y - totalH) * 0.5f;
        else if (m_vAlign == 2) y0 = (y0 + size.y) - totalH;
    }
    float x0 = pos.x;
    if (m_hAlign == 1) x0 = (pos.x + size.x * 0.5f) - totalW * 0.5f;
    else if (m_hAlign == 2) x0 = (pos.x + size.x) - totalW;
    if (m_orientation == 1) {
        if (m_scrollBar) m_scrollBar->SetVisible(size.y < totalH);
        float y = y0;
        for (int r = 0; r <= rowsLast; r++) {
            float x = x0;
            for (int c = 0; c <= colsLast; c++) {
                size_t idx = (size_t)(m_columns * r + c);
                if (idx >= m_childObjects.size()) break;
                InterfaceObject *child = m_childObjects[idx];
                math::vec2<float> cp = child->GetPos();
                child->SetOffset(math::vec2<float>(x - cp.x, y - cp.y));
                if (child == m_mainChild) {
                    math::rect<float> clip = GetClip();
                    math::vec2<float> p = child->GetPos(), o = child->GetOffset(), s = child->GetSize();
                    float newY;
                    if (clip.upperLeftCorner.y <= p.y + o.y) {
                        if (p.y + o.y + s.y <= clip.lowerRightCorner.y) { x += cw + m_spacing; continue; }
                        newY = clip.lowerRightCorner.y - cp.y - child->GetSize().y;
                    } else {
                        newY = clip.upperLeftCorner.y - cp.y;
                    }
                    child->SetOffset(math::vec2<float>(x - cp.x, newY - GetOffset().y));
                }
                x += cw + m_spacing;
            }
            y += ch + m_spacing;
        }
    } else if (m_orientation == 0) {
        float x = x0;
        for (int c = 0; c <= colsLast; c++) {
            float y = y0;
            for (int r = 0; r <= rowsLast; r++) {
                size_t idx = (size_t)(m_rows * c + r);
                if (idx >= m_childObjects.size()) break;
                InterfaceObject *child = m_childObjects[idx];
                math::vec2<float> cp = child->GetPos();
                child->SetOffset(math::vec2<float>(x - cp.x, y - cp.y));
                y += ch + m_spacing;
            }
            x += cw + m_spacing;
        }
    }
}

math::vec2<float> InterfaceGrid::GetOffset() {
    math::vec2<float> o = InterfaceObject::GetOffset();
    return math::vec2<float>(m_scrollOffset.x + o.x, m_scrollOffset.y + o.y);
}

math::vec2<float> InterfaceGrid::GetOffsetForClip() { return InterfaceObject::GetOffset(); }

// the child's rectangle as the grid draws it
math::rect<float> InterfaceGrid::ChildRect(InterfaceObject *c) {
    math::vec2<float> p = c->GetPos(), o = c->GetOffset(), s = c->GetSize();
    return math::rect<float>(o.x + p.x, o.y + p.y, p.x + o.x + s.x, p.y + o.y + s.y);
}

// @0x44ddf0: only the children on screen
void InterfaceGrid::UpdateChildren(int dt) {
    math::rect<float> clip = GetClip();
    for (size_t i = 0; i < m_childObjects.size(); i++)
        if (ChildRect(m_childObjects[i]).isRectCollided(clip)) m_childObjects[i]->Update(dt);
}

// @0x44df78: the main child last, on top
void InterfaceGrid::RenderChildren(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    math::rect<float> clip = GetClip();
    for (size_t i = 0; i < m_childObjects.size(); i++) {
        InterfaceObject *c = m_childObjects[i];
        if (c != m_mainChild && ChildRect(c).isRectCollided(clip)) c->Render(cam, painter);
    }
    if (m_mainChild) m_mainChild->Render(cam, painter);
}

// @0x44e14c
bool InterfaceGrid::IsChidlVisible(int i) {
    InterfaceObject *c = m_childObjects[i];
    if (c == m_mainChild) return true;
    return ChildRect(c).isRectCollided(GetClip());
}

// @0x44e270
void InterfaceGrid::Focus(int index, bool animate) {
    if (m_orientation == 1) {
        math::vec2<float> cs = m_childObjects[0]->GetSize();
        float rowH = m_spacing + cs.y;
        int row = Div(index, m_columns);
        float t = (float)(-row) * rowH;
        m_scrollTarget.y = t;
        m_page = -(int)((int)t / (int)rowH);
        int count = (int)m_childObjects.size();
        if (count < m_rows) count = m_rows;
        float limit = (m_spacing + GetSize().y) - (float)count * rowH;
        if (0.0f <= limit) m_scrollTarget.y = 0.0f;
        else if (m_scrollTarget.y < limit) m_scrollTarget.y = limit;
    } else {
        math::vec2<float> cs = m_childObjects[0]->GetSize();
        float step = m_spacing + cs.x;
        int col = Div(index, m_rows);
        float t = (float)(-col) * step;
        m_scrollTarget.x = t;
        m_page = -(int)((int)t / (int)step) - m_columns / 2;
    }
    if (!animate) m_scrollOffset = m_scrollTarget;
}

// @0x44e3e4
int InterfaceGrid::GetNumPages() {
    int per = m_orientation == 0 ? m_columns : m_rows;
    return (int)m_childObjects.size() / per;
}

void InterfaceGrid::RemoveChild(InterfaceObject *c) {
    InterfaceObject::RemoveChild(c);
    if (m_mainChild == c) m_mainChild = 0;
}
