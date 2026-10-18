// libs/vox1.1/include/vox_fader.h
// A linear ramp of a value over a time (the gain, the pitch and the play / stop fades of an emitter use it).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_FADER_H
#define LIBS_VOX1_1_INCLUDE_VOX_FADER_H

namespace vox {
class Fader {  // sizeof 0x14
public:
    Fader(float start, float end, float time);
    Fader();
    ~Fader();
    void Update(float dt);
    float GetStartValue() const { return m_startValue; }
    float GetEndValue() const { return m_endValue; }
    float GetCurrentValue() const;
    bool IsFinished() const { return m_done; }
    float GetRemainingTime() const;
private:
    float m_startValue;  // 0x0
    float m_endValue;  // 0x4
    float m_time;  // 0x8
    float m_totalTime;  // 0xC
    bool m_done;  // 0x10
};
}

#endif
