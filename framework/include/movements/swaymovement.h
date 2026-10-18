// framework/include/movements/swaymovement.h
#ifndef FRAMEWORK_INCLUDE_MOVEMENTS_SWAYMOVEMENT_H
#define FRAMEWORK_INCLUDE_MOVEMENTS_SWAYMOVEMENT_H

#include "smath.h"
#include "core/delayer.h"
#include "stream/istream.h"

// A camera sway: the camera drifts to the side and rolls back and forth on a sine of the given period
class SwayMovement {  // sizeof 0x80
public:
    SwayMovement();
    void Start(float yaw, int type, float strength, int ms);
    void Start(float yaw, int periodMs, float displacement, float angle, int ms);
    void Stop(int ms);
    void Update(int) {}
    void Render(int dt);
    bool IsMoving() const { return m_displacement.GetCurrentValue() > 0.0f || m_angle.GetCurrentValue() > 0.0f; }
    const math::vec3<float> &GetPosition() const { return m_position; }
    const math::quat<float> &GetRotation() const { return m_rotation; }
    bool Serialize(jet::stream::IStreamW&) const;
    void Deserialize(const jet::stream::IStream&);
private:
    math::vec3<float> m_position;  // 0x0
    math::quat<float> m_rotation;  // 0xC
    int m_periodFrom, m_period, m_periodTo;  // 0x1C, 0x20, 0x24: the period eases between them
    int m_periodTime, m_periodElapsed;  // 0x28, 0x2C
    bool m_periodMoving;  // 0x30
    jet::core::Delayer<float, jet::core::SmootherStep> m_displacement, m_angle, m_yaw;  // 0x34, 0x4C, 0x64
    int m_curTime;  // 0x7C
};

#endif
