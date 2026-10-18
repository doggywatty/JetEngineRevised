// source/game/input/TouchMgr.cpp
#include "TouchMgr.h"

int TouchMgr::s_updateCount = 0;

// @0x493328
Touch::Touch() : id(-1), state(0), startPos(0, 0), prevPos(0, 0), pos(0, 0), changedThisFrame(false), pressFrames(0), unused0(0), unused1(0) {}

// @0x496264
TouchMgr::TouchMgr() : m_getNextTouchId(0), m_getNextTouchStateMask(0) {}
TouchMgr::~TouchMgr() {}

// @0x4963c4
void TouchMgr::Invalidate() {
    for (Touch &t : m_touches) {
        t.id = -1;
        t.state = 0;
        t.changedThisFrame = false;
        t.pressFrames = 0;
    }
    m_queue.clear();
}

static Touch Event(int id, unsigned state, float x, float y) {
    Touch t;
    t.id = id;
    t.state = state;
    t.pos = math::vec2<float>(x, y);
    if (state == Touch::Pressed) t.startPos = t.pos;
    return t;
}

// @0x49641c .. @0x4964fc: the events wait in a queue until Update
void TouchMgr::OnTouchPress(int id, float x, float y) { m_queue.push_back(Event(id, Touch::Pressed, x, y)); }
void TouchMgr::OnTouchDrag(int id, float x, float y) { m_queue.push_back(Event(id, Touch::Dragged, x, y)); }
void TouchMgr::OnTouchRelease(int id, float x, float y) { m_queue.push_back(Event(id, Touch::Released, x, y)); }
void TouchMgr::OnTouchCanceled(int id, float x, float y) { m_queue.push_back(Event(id, Touch::Cancelled, x, y)); }

// @0x496544
std::vector<const Touch*> TouchMgr::GetAllActiveTouches() const {
    std::vector<const Touch*> r;
    for (const Touch &t : m_touches)
        if (t.id != -1 && t.state != 0) r.push_back(&t);
    return r;
}

// @0x4865bc
std::vector<const Touch*> TouchMgr::GetAllValidTouches() const {
    std::vector<const Touch*> r;
    for (const Touch &t : m_touches)
        if (t.id != -1) r.push_back(&t);
    return r;
}

// @0x496628 / @0x496638
Touch *TouchMgr::GetFirstTouch(int mask) {
    m_getNextTouchStateMask = mask;
    m_getNextTouchId = 0;
    return GetNextTouch();
}

Touch *TouchMgr::GetNextTouch() {
    while (m_getNextTouchId < k_maxTouches) {
        Touch &t = m_touches[m_getNextTouchId++];
        if (t.id != -1 && (t.state & (unsigned)m_getNextTouchStateMask)) return &t;
    }
    return 0;
}

std::vector<const Touch*> TouchMgr::FindTouches(int mask) const {
    std::vector<const Touch*> r;
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask)) r.push_back(&t);
    return r;
}

const Touch *TouchMgr::FindTouch(int mask) const {
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask)) return &t;
    return 0;
}

// a pressed touch is where it started, a held one is the segment it moved along, others where they are
bool TouchMgr::Hits(const Touch &t, const math::rect<float> &r) const {
    if (t.state == Touch::Pressed) return r.isPointInside(t.startPos);
    if (t.state != Touch::Dragged) return r.isPointInside(t.pos);
    if (r.isPointInside(t.pos) || r.isPointInside(t.prevPos)) return true;
    math::line2<float> seg(t.prevPos, t.pos);
    return seg.getIntersectionWithRect(r, 0, 0);
}

bool TouchMgr::Hits(const Touch &t, const math::vec2<float> &c, float radius) const {
    float r2 = radius * radius;
    if (t.state == Touch::Pressed) return math::distanceSq(c, t.startPos) <= r2;
    if (t.state != Touch::Dragged) return math::distanceSq(c, t.pos) <= r2;
    if (math::distanceSq(c, t.pos) <= r2 || math::distanceSq(c, t.prevPos) <= r2) return true;
    math::line2<float> seg(t.prevPos, t.pos);
    return seg.getIntersectionWithCircle(c, radius, 0);
}

// @0x496704
const Touch *TouchMgr::FindTouch(const math::rect<float> &r, int mask) const {
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask) && Hits(t, r)) return &t;
    return 0;
}

// @0x4967f0
const Touch *TouchMgr::FindTouch(const math::vec2<float> &c, float radius, int mask) const {
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask) && Hits(t, c, radius)) return &t;
    return 0;
}

std::vector<const Touch*> TouchMgr::FindTouches(const math::rect<float> &r, int mask) const {
    std::vector<const Touch*> out;
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask) && Hits(t, r)) out.push_back(&t);
    return out;
}

std::vector<const Touch*> TouchMgr::FindTouches(const math::vec2<float> &c, float radius, int mask) const {
    std::vector<const Touch*> out;
    for (const Touch &t : m_touches)
        if (t.id != -1 && (t.state & (unsigned)mask) && Hits(t, c, radius)) out.push_back(&t);
    return out;
}

// @0x496930: the slot of a finger, a free one when asked to create
Touch *TouchMgr::FindTouchById(int id, bool create) {
    int free = -1;
    for (int i = 0; i < k_maxTouches; i++) {
        if (m_touches[i].id == id) return &m_touches[i];
        if (free == -1 && m_touches[i].id == -1) free = i;
    }
    if (!create || free == -1) return 0;
    m_touches[free].id = id;
    return &m_touches[free];
}

// @0x496ce0: applies the queued events; a touch left alone moves on (pressed -> held, released and cancelled -> free)
void TouchMgr::Update() {
    for (Touch &t : m_touches) {
        t.changedThisFrame = false;
        t.prevPos = t.pos;
    }
    size_t n = m_queue.size();
    for (size_t i = 0; i < n; i++) {
        Touch e = m_queue.front();
        m_queue.pop_front();
        Touch *t = FindTouchById(e.id, e.state == Touch::Pressed);
        if (!t) continue;
        if (t->changedThisFrame) {
            m_queue.push_back(e);
            continue;
        }
        if (e.state == Touch::Pressed) {
            t->pressFrames = 0;
            t->startPos = e.pos;
            t->prevPos = e.pos;
            t->pos = e.pos;
        } else {
            t->pos = e.pos;
        }
        unsigned old = t->state;
        t->state = e.state;
        t->changedThisFrame = old != e.state;
    }
    s_updateCount++;
    for (Touch &t : m_touches) {
        if (t.id == -1 || t.changedThisFrame) continue;
        if (t.state & (Touch::Released | Touch::Cancelled)) {
            t.state = 0;
            t.id = -1;
        }
        if (t.state == Touch::Pressed) t.state = Touch::Dragged;
    }
}
