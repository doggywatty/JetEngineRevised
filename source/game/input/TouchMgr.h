// source/game/input/TouchMgr.h
#ifndef SOURCE_GAME_INPUT_TOUCHMGR_H
#define SOURCE_GAME_INPUT_TOUCHMGR_H

#include <deque>
#include <vector>
#include "jsingleton.h"
#include "smath.h"

// One finger. State: 1 pressed, 2 held, 4 released, 8 cancelled; positions in screen pixels.
struct Touch {  // sizeof 0x30
    enum State { Pressed = 1, Dragged = 2, Released = 4, Cancelled = 8 };
    Touch();
    int id;  // 0x0: -1 for a free slot
    unsigned state;  // 0x4
    math::vec2<float> startPos;  // 0x8
    math::vec2<float> prevPos;  // 0x10
    math::vec2<float> pos;  // 0x18
    bool changedThisFrame;  // 0x20
    int pressFrames;  // 0x24
    int unused0, unused1;  // 0x28, 0x2C
};

// Collects the window's touch events and turns them into per frame touch states
class TouchMgr : public Singleton<TouchMgr> {  // sizeof 0x218
public:
    enum { k_maxTouches = 10 };
    TouchMgr();
    virtual ~TouchMgr();
    void Update();
    const Touch *FindTouch(int stateMask) const;
    const Touch *FindTouch(const math::vec2<float> &center, float radius, int stateMask) const;
    const Touch *FindTouch(const math::rect<float>&, int stateMask) const;
    Touch *GetFirstTouch(int stateMask);
    Touch *GetNextTouch();
    std::vector<const Touch*> GetAllActiveTouches() const;
    std::vector<const Touch*> GetAllValidTouches() const;
    std::vector<const Touch*> FindTouches(const math::vec2<float> &center, float radius, int stateMask) const;
    std::vector<const Touch*> FindTouches(const math::rect<float>&, int stateMask) const;
    std::vector<const Touch*> FindTouches(int stateMask) const;
    void OnTouchPress(int id, float x, float y);
    void OnTouchDrag(int id, float x, float y);
    void OnTouchRelease(int id, float x, float y);
    void OnTouchCanceled(int id, float x, float y);
    int NumberOfTouchesQueued() const { return (int)m_queue.size(); }
    void Invalidate();
    Touch *GetTouch(int id) { return FindTouchById(id, false); }
private:
    Touch *FindTouchById(int id, bool create);
    bool Hits(const Touch&, const math::rect<float>&) const;
    bool Hits(const Touch&, const math::vec2<float>&, float) const;
    Touch m_touches[k_maxTouches];  // 0x8
    std::deque<Touch> m_queue;  // 0x1E8
    int m_getNextTouchId;  // 0x210
    int m_getNextTouchStateMask;  // 0x214
    static int s_updateCount;
};

#endif
