// core/jet/include/core/delayer.h
#ifndef CORE_JET_INCLUDE_CORE_DELAYER_H
#define CORE_JET_INCLUDE_CORE_DELAYER_H

#include "core/interputils.h"
#include "stream/istream.h"

namespace jet {
namespace core {

// Eases a value from its origin to a target over a delay in ms, with the Step curve.
template <class T, class Step = LinearStep>
class Delayer {
public:
    Delayer() : m_originValue(), m_currentValue(), m_targetValue(), m_delay(0), m_accumTime(0), m_updating(false) {}
    Delayer(const T& current, const T& target, int delay)
        : m_originValue(current), m_currentValue(current), m_targetValue(target), m_delay(delay), m_accumTime(0),
          m_updating(!math::equals(current, target, 1.1920929e-07f))
    {
    }

    // Returns true while still moving.
    bool Update(int dtMs)
    {
        if (!m_updating)
            return false;
        m_accumTime += dtMs;
        if (m_accumTime >= m_delay) {
            m_currentValue = m_targetValue;
            m_updating = false;
        } else if (m_accumTime < 0) {
            m_currentValue = m_originValue;
        } else {
            float f = Step(float(m_accumTime) / float(m_delay)).v;
            m_currentValue = Mix(m_originValue, m_targetValue, f);
        }
        return m_updating;
    }
    bool IsUpdating() const { return m_updating; }
    const T& GetCurrentValue() const { return m_currentValue; }
    void SetCurrentValue(const T& v)
    {
        m_originValue = m_currentValue = v;
        m_accumTime = 0;
        m_updating = !math::equals(m_currentValue, m_targetValue, 1.1920929e-07f);
    }
    const T& GetTargetValue() const { return m_targetValue; }
    void SetTargetValue(const T& v)
    {
        if (m_delay < 1) {
            m_currentValue = m_targetValue = v;
            m_updating = false;
            return;
        }
        if (math::equals(m_targetValue, v, 1.1920929e-07f))
            return;
        m_originValue = m_currentValue;
        m_targetValue = v;
        m_accumTime = 0;
        m_updating = !math::equals(m_currentValue, m_targetValue, 1.1920929e-07f);
    }
    void SetDelay(int d)
    {
        m_delay = d;
        if (!d) {
            m_updating = false;
            m_accumTime = 0;
            m_currentValue = m_targetValue;
        }
    }
    int GetDelay() const { return m_delay; }
    int GetAccumTime() const { return m_accumTime; }
    void ResetValues(const T& current, const T& target)
    {
        m_originValue = m_currentValue = current;
        m_targetValue = target;
        m_accumTime = 0;
        m_updating = !math::equals(current, target, 1.1920929e-07f);
    }
    void ResetValues(const T& v) { ResetValues(v, v); }
    void Write(stream::IStreamW& s) const { s << *this; }
    void Read(stream::IStream& s) { s >> *this; }

private:
    // Floats blend as origin + f * (target - origin), vectors and quaternions through math::lerp, as shipped.
    static float Mix(float a, float b, float f) { return a + f * (b - a); }
    template <class U>
    static U Mix(const U& a, const U& b, float f) { return math::lerp(a, b, f); }

    T m_originValue;
    T m_currentValue;
    T m_targetValue;
    int m_delay;
    int m_accumTime;
    bool m_updating;
};

}
}

#endif
