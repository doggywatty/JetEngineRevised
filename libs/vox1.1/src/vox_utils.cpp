// libs/vox1.1/src/vox_utils.cpp
#include "vox_fader.h"

namespace vox {

Fader::Fader(float start, float end, float time) : m_startValue(start), m_endValue(end), m_time(0), m_totalTime(time), m_done(time <= 0) {}

Fader::Fader() : m_startValue(0), m_endValue(0), m_time(0), m_totalTime(0), m_done(true) {}

Fader::~Fader() {}

void Fader::Update(float dt) {
    if (m_done) return;
    m_time += dt;
    if (m_time >= m_totalTime) {
        m_time = m_totalTime;
        m_done = true;
    }
}

float Fader::GetCurrentValue() const {
    if (m_done) return m_endValue;
    return m_startValue + (m_endValue - m_startValue) * (m_time / m_totalTime);
}

float Fader::GetRemainingTime() const { return m_done ? 0.0f : m_totalTime - m_time; }

}  // namespace vox
