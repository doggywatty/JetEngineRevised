// framework/src/movements/swaymovement.cpp
#include "movements/swaymovement.h"
#include "scene/scenemgr.h"

using jet::core::SmootherStep;

// @0x597248
SwayMovement::SwayMovement()
    : m_position(0, 0, 0), m_rotation(0, 0, 0, 1), m_periodFrom(0), m_period(0), m_periodTo(0), m_periodTime(1000),
      m_periodElapsed(0), m_periodMoving(false), m_displacement(0.0f, 0.0f, 1000), m_angle(0.0f, 0.0f, 1000),
      m_yaw(0.0f, 0.0f, 1000), m_curTime(0) {}

// @0x5872fc: the preset strengths, 1 slow, 2 medium, 3 fast
void SwayMovement::Start(float yaw, int type, float strength, int ms) {
    int period;
    float disp, angle;
    switch (type) {
    case 1: period = 10000; disp = strength * 0.4f; angle = strength * 0.05235988f; break;
    case 2: period = 7500; disp = strength * 0.3f; angle = strength * 0.07853982f; break;
    case 3: period = 5000; disp = strength * 0.2f; angle = strength * 0.10471976f; break;
    default: Stop(ms); return;
    }
    Start(yaw, period, disp, angle, ms);
}

// @0x5873ac
void SwayMovement::Start(float yaw, int period, float displacement, float angle, int ms) {
    m_yaw.SetTargetValue(yaw);
    if (ms == 0) {
        m_periodMoving = false;
        m_periodElapsed = 0;
    }
    m_periodTime = ms;
    if (ms == 0) m_period = m_periodTo;
    m_displacement.SetDelay(ms);
    m_angle.SetDelay(ms);
    bool moving;
    if (m_periodTime < 1) {
        moving = false;
        m_period = m_periodTo = period;
        m_periodMoving = moving;
    } else if (m_periodTo != period) {
        m_periodTo = period;
        m_periodElapsed = 0;
        m_periodFrom = m_period;
        m_periodMoving = m_period != period;
    }
    if (!IsMoving()) {
        m_periodFrom = m_period = period;
        m_periodElapsed = 0;
        m_periodMoving = period != m_periodTo;
    }
    m_displacement.SetTargetValue(displacement);
    m_angle.SetTargetValue(angle);
}

// @0x58749c
void SwayMovement::Stop(int ms) {
    m_periodTime = ms;
    if (ms == 0) {
        m_periodMoving = false;
        m_periodElapsed = 0;
        m_period = m_periodTo;
    }
    m_displacement.SetDelay(ms);
    m_angle.SetDelay(ms);
    m_displacement.SetTargetValue(0.0f);
    m_angle.SetTargetValue(0.0f);
}

// @0x587510
void SwayMovement::Render(int dt) {
    m_displacement.Update(dt);
    m_angle.Update(dt);
    int prevPeriod = m_period;
    if (m_periodMoving) {
        m_periodElapsed += dt;
        if (m_periodElapsed < m_periodTime) {
            if (m_periodElapsed < 0)
                m_period = m_periodFrom;
            else {
                float f = SmootherStep((float)m_periodElapsed / (float)m_periodTime).v;
                m_period = m_periodFrom + (int)((double)(m_periodTo - m_periodFrom) * (double)f);
            }
        } else {
            m_period = m_periodTo;
            m_periodMoving = false;
        }
    }
    if (prevPeriod != 0 && m_period != prevPeriod) m_curTime = m_curTime * m_period / prevPeriod;
    m_yaw.Update(dt);
    if (!IsMoving()) {
        m_periodFrom = m_period = m_periodTo = 0;
        m_periodElapsed = 0;
        m_periodMoving = false;
        return;
    }
    if (m_period > 0) {
        m_curTime = (dt + m_curTime) % m_period;
        math::quat<float> q(0, 0, 0, 1);
        q.setFromAngleAxis(m_yaw.GetCurrentValue(), jet::scene::SceneMgr::GetUpVector());
        float f = cosf(1.5707964f + ((float)m_curTime / (float)m_period) * 6.2831855f);
        math::vec3<float> side = q * jet::scene::SceneMgr::GetRightVector();
        m_position = side * (f * m_displacement.GetCurrentValue());
        math::vec3<float> axis = q * jet::scene::SceneMgr::GetFrontVector();
        m_rotation.setFromAngleAxis(-(m_angle.GetCurrentValue() * f), axis);
    }
}

bool SwayMovement::Serialize(jet::stream::IStreamW &s) const {
    s.Write(&m_position, sizeof(m_position));
    s << m_rotation;
    s.Write(&m_periodFrom, 0x18);
    m_displacement.Write(s);
    m_angle.Write(s);
    m_yaw.Write(s);
    s << m_curTime;
    return true;
}

void SwayMovement::Deserialize(const jet::stream::IStream &s) {
    s.Read(&m_position, sizeof(m_position));
    s >> m_rotation;
    s.Read(&m_periodFrom, 0x18);
    m_displacement.Read(const_cast<jet::stream::IStream&>(s));
    m_angle.Read(const_cast<jet::stream::IStream&>(s));
    m_yaw.Read(const_cast<jet::stream::IStream&>(s));
    s >> m_curTime;
}
