// core/jet/include/core/fader.h
#ifndef CORE_JET_INCLUDE_CORE_FADER_H
#define CORE_JET_INCLUDE_CORE_FADER_H

namespace vox {
class Fader {  // sizeof 0x14
public:
    Fader(float, float, float);
    Fader();
    ~Fader();
    void Update(float);
    float GetStartValue();
    float GetEndValue();
    float GetCurrentValue();
    bool IsFinished();
    float GetRemainingTime();
private:
    float m_startValue;  // 0x0
    float m_endValue;  // 0x4
    float m_time;  // 0x8
    float m_totalTime;  // 0xC
    bool m_done;  // 0x10
};
}

#endif
