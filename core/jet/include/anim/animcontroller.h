// core/jet/include/anim/animcontroller.h
#ifndef CORE_JET_INCLUDE_ANIM_ANIMCONTROLLER_H
#define CORE_JET_INCLUDE_ANIM_ANIMCONTROLLER_H

namespace jet::anim {

// Playback clock of an animated object (AnimController<T>, Update @0x69d7c0): time in ms, speed 16.16
// fixed point; at the end a looping controller wraps into [loopStart, end), else it stops (inactive).
// Node controllers (type 0) tell the owner when their time is set so it can refresh its movement delta.
template <class T>
class AnimController {
public:
    enum Type { Nodes = 0, Textures = 1 };
    explicit AnimController(T *owner = 0, int type = Nodes)
        : m_owner(owner), m_type(type), m_time(0), m_prevTime(-1), m_totalTime(0), m_speed(0x10000), m_frac(0),
          m_loopStart(0), m_end(0), m_loopCount(0), m_looping(true), m_finished(false), m_active(true) {}
    void Reset() {
        m_loopCount = 0;
        m_finished = false;
        m_time = 0;
        if (m_type == Nodes) m_owner->RefreshMovementDelta(false);
        m_prevTime = -1;
        m_active = true;
    }
    void Update(int dt) {
        int acc = m_speed * dt + m_frac;
        int step = acc >> 16;
        m_frac = acc & 0xffff;
        m_prevTime = m_time;
        m_time += step;
        m_totalTime += step;
        if (m_time < m_end) return;
        m_finished = true;
        if (m_looping) {
            int len = m_end - m_loopStart;
            if (len < 1) m_time = m_loopStart;
            ++m_loopCount;
            if (len >= 1) m_time = m_loopStart + (m_time - m_end) % len;
        } else {
            m_time = m_end;
            m_active = false;
        }
    }
    void SetTime(int t) { SetTimeImpl(t, false); }
    void SetTimeFull(int t) { SetTimeImpl(t, true); }
    int GetTime() const { return m_time; }
    int GetPrevTime() const { return m_prevTime; }
    // speed as 16.16 fixed point
    void SetSpeedFixed(int s) { m_speed = s; }
    int GetSpeedFixed() const { return m_speed; }
    void SetSpeed(float s) { m_speed = (int)(s * 65536.0f); }
    float GetSpeed() const { return m_speed / 65536.0f; }
    void SetLoop(int start, int end) { m_loopStart = start; m_end = end; }
    int GetLoopStart() const { return m_loopStart; }
    void SetEnd(int end) { m_end = end; }
    int GetEnd() const { return m_end; }
    void SetLooping(bool l) { m_looping = l; }
    bool IsLooping() const { return m_looping; }
    bool IsFinished() const { return m_finished; }
    void SetFinished(bool f) { m_finished = f; }
    bool IsActive() const { return m_active; }
    void SetActive(bool a) { m_active = a; }
    int GetLoopCount() const { return m_loopCount; }
    void SetLoopCount(int c) { m_loopCount = c; }
    int GetTotalTime() const { return m_totalTime; }
    void SetTotalTime(int t) { m_totalTime = t; }
    void SetPrevTime(int t) { m_prevTime = t; }
    int GetType() const { return m_type; }
private:
    void SetTimeImpl(int t, bool full) {
        int old = m_time;
        if (t == old) return;
        m_time = t < m_loopStart ? m_loopStart : (t > m_end ? m_end : t);
        if (m_type == Nodes) m_owner->RefreshMovementDelta(full);
        m_prevTime = old;
    }
    T *m_owner;         // 0x0
    int m_type;         // 0x4
    int m_time;         // 0x8
    int m_prevTime;     // 0xc
    int m_totalTime;    // 0x10
    int m_speed;        // 0x14
    int m_frac;         // 0x18
    int m_loopStart;    // 0x1c
    int m_end;          // 0x20
    int m_loopCount;    // 0x24
    bool m_looping;     // 0x28
    bool m_finished;    // 0x29
    bool m_active;      // 0x2a
};
}

#endif
