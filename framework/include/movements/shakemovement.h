// framework/include/movements/shakemovement.h
#ifndef FRAMEWORK_INCLUDE_MOVEMENTS_SHAKEMOVEMENT_H
#define FRAMEWORK_INCLUDE_MOVEMENTS_SHAKEMOVEMENT_H

#include "smath.h"
#include "stream/istream.h"

enum ShakeType { SHAKE_NONE, SHAKE_VERTICAL, SHAKE_HORIZONTAL, SHAKE_BOTH };

// A camera shake: a pitch and a yaw oscillating in opposite phase, decaying once the time is over
class ShakeMovement {  // sizeof 0x3C
public:
    ShakeMovement();
    void Start(ShakeType, float, int);
    void Reset();
    void Update(int dt);
    void Render(int dt);
    bool IsMoving() const { return m_hMax != 0 || m_vMax != 0; }
    const math::quat<float> &GetRotation() const { return m_rotation; }
    bool Serialize(jet::stream::IStreamW&) const;
    void Deserialize(const jet::stream::IStream&);
    float m_hMax, m_vMax;  // 0x20, 0x24
private:
    math::quat<float> m_rotation;  // 0x0
    float m_hDir, m_vDir;  // 0x10, 0x14
    float m_hOffset, m_vOffset;  // 0x18, 0x1C
    float m_hTime, m_vTime;  // 0x28, 0x2C
    float m_hDuration, m_vDuration;  // 0x30, 0x34
    int m_time;  // 0x38
};

#endif
