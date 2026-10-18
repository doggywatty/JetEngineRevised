// source/game/interfaces/interfaceobjects/InterfaceListButtonScroll.cpp
#include "InterfaceListButtonScroll.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include <cmath>

const jet::String InterfaceListButtonScroll::k_uiTmplName("ListButtonScroll");

// @0x44f6f4
InterfaceListButtonScroll::InterfaceListButtonScroll() : m_count(0), m_stepSize(0), m_scrollRange(0), m_wrap(false), m_measured(false) {}

InterfaceListButtonScroll::InterfaceListButtonScroll(AuroraLevelData *level, int id) : InterfaceList(level, id), m_count(0), m_stepSize(0), m_scrollRange(0), m_wrap(false), m_measured(false) {}

InterfaceListButtonScroll::~InterfaceListButtonScroll() {}

// @0x44f7c4: param 20 = wrap around
void InterfaceListButtonScroll::Init() {
    InterfaceList::Init();
    m_wrap = m_guiLevel->GetParamValue(m_guiId, 20) == 1;
    m_measured = false;
}

// @0x44f7fc: children are measured once, after the first update laid them out
void InterfaceListButtonScroll::Update(int dt) {
    InterfaceList::Update(dt);
    if (m_measured) return;
    CalculateScrollByClick();
    m_measured = true;
}

// @0x44f828
void InterfaceListButtonScroll::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    if (!IsVisible()) return;
    InterfaceObject::Render(cam, painter);
    math::vec2<float> off = GetOffset();
    float ox = off.x - m_scrollOffset.x, oy = off.y - m_scrollOffset.y;
    math::vec2<float> pos = GetPos(), size = GetSize();
    m_inputRect = GetTransformedRect(cam, painter.GetPostClipTransform(), math::rect<float>(pos.x + ox, pos.y + oy, pos.x + size.x + ox, pos.y + size.y + oy));
    if (m_bgObject) m_bgObject->Render(cam, painter);
    RenderChildren(cam, painter);
}

// @0x44f82c
unsigned InterfaceListButtonScroll::CurrentElement() const {
    float v;
    if (m_orientation == Vertical) v = m_scrollTarget.y;
    else if (m_orientation == Horizontal) v = m_scrollTarget.x;
    else return 0;
    float e = std::floor((-v / m_scrollRange) * (float)m_count + 0.5f);
    return e > 0 ? (unsigned)e : 0;
}

// @0x44f888
void InterfaceListButtonScroll::ScrollUp() {
    unsigned i = CurrentElement();
    if (i == 0) {
        if (!m_wrap) return;
        i = m_count;
    }
    m_scrollTarget = PositionOffsetAt(i - 1);
}

// @0x44f8f0
void InterfaceListButtonScroll::ScrollDown() {
    unsigned i = CurrentElement();
    if (i == m_count - 1) {
        if (!m_wrap) return;
        i = 0;
    } else {
        i++;
    }
    m_scrollTarget = PositionOffsetAt(i);
}

// @0x44f95c: the scroll range is the content past the margins, split evenly per child
void InterfaceListButtonScroll::CalculateScrollByClick() {
    m_count = (unsigned)(m_childObjects.size());
    float total;
    if (m_orientation == Vertical) total = GetTotalChildrenHeight();
    else if (m_orientation == Horizontal) total = GetTotalChildrenWidth();
    else return;
    m_scrollRange = total - (m_spacing + m_spacing);
    m_stepSize = m_scrollRange / (float)m_count;
}

// @0x44f9d0
math::vec2<float> InterfaceListButtonScroll::PositionOffsetAt(unsigned idx) const {
    if (m_orientation == Vertical) return math::vec2<float>(0, -((float)idx * m_stepSize));
    if (m_orientation == Horizontal) return math::vec2<float>(-((float)idx * m_stepSize), 0);
    return math::vec2<float>(0, 0);
}

// @0x44fa1c
void InterfaceListButtonScroll::ScrollAt(unsigned) { m_scrollTarget = PositionOffsetAt(CurrentElement()); }

// @0x44fa50: a touch on the list that is pressed or moving
bool InterfaceListButtonScroll::IsScrolling() const {
    const Touch *t = TouchMgr::GetInstance()->FindTouch(m_inputRect, -1);
    return t && (unsigned)(t->state - 1) < 2;
}
