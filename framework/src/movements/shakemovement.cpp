// framework/src/movements/shakemovement.cpp
#include "movements/shakemovement.h"

// @0x596e78
ShakeMovement::ShakeMovement() : m_rotation(0, 0, 0, 1) { Reset(); }

// @0x596ea4: the amplitudes per type, in radians-ish units scaled by the strength
void ShakeMovement::Start(ShakeType type, float strength, int time) {
    static const float h[4] = { 0, 0, 0.3f, 1.0f }, v[4] = { 0.3f, 1.0f, 0.3f, 1.0f };
    float ah = 0, av = 0;
    if ((unsigned)type < 4) { ah = h[type]; av = v[type]; }
    m_time = time;
    m_hOffset = m_vOffset = 0;
    m_hDuration = m_vDuration = 60.0f;
    m_hDir = 1.0f;
    m_vDir = 0.8f;
    m_hMax = ah * strength;
    m_vMax = av * strength;
}

// @0x596f30
void ShakeMovement::Reset() {
    m_time = 0;
    m_hDir = m_vDir = m_hOffset = m_vOffset = m_hMax = m_vMax = m_hTime = m_vTime = m_hDuration = m_vDuration = 0;
}

// @0x596f68: holds for the shake time, then decays by 0.8 a frame
void ShakeMovement::Update(int dt) {
    if (m_time < 1) {
        m_hMax *= 0.8f;
        m_vMax *= 0.8f;
    } else
        m_time -= dt;
}

// @0x596f9c
void ShakeMovement::Render(int dt) {
    if (m_hDuration > 0.0f) {
        m_hTime += (float)dt * m_hDir;
        if (m_hTime * m_hDir < m_hDuration * 0.5f) m_hDir = -m_hDir;
        float t = math::clamp((m_hTime + m_hDuration * 0.5f) / m_hDuration, 0.0f, 1.0f);
        m_hOffset = (m_hMax + m_hMax) * t - m_hMax;
    }
    if (m_vDuration > 0.0f) {
        m_vTime += (float)dt * m_vDir;
        if (m_vDuration * 0.5f < m_vTime * m_vDir) m_vDir = -m_vDir;
        float t = math::clamp((m_vTime + m_vDuration * 0.5f) / m_vDuration, 0.0f, 1.0f);
        m_vOffset = (m_vMax + m_vMax) * t - m_vMax;
    }
    float sv, cv, sh, ch;
    math::sincos(m_vOffset * 0.008726647f, sv, cv);
    math::sincos(m_hOffset * 0.008726647f, sh, ch);
    m_rotation = math::quat<float>(sv, 0, 0, cv) * math::quat<float>(0, 0, sh, ch);
}

bool ShakeMovement::Serialize(jet::stream::IStreamW &s) const {
    s << m_rotation << m_hDir << m_vDir << m_hOffset << m_vOffset << m_hMax << m_vMax << m_hTime << m_vTime
      << m_hDuration << m_vDuration << m_time;
    return true;
}

void ShakeMovement::Deserialize(const jet::stream::IStream &s) {
    s >> m_rotation >> m_hDir >> m_vDir >> m_hOffset >> m_vOffset >> m_hMax >> m_vMax >> m_hTime >> m_vTime >>
        m_hDuration >> m_vDuration >> m_time;
}
