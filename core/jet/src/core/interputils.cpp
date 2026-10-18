// core/jet/src/core/interputils.cpp
#include "core/interputils.h"

namespace jet {
namespace core {

float GetInterpolated(Interpolation type, float t)
{
    t = math::clamp(t, 0.0f, 1.0f);
    switch (type) {
    case InterpolationDef::STEP: return t >= 1.0f ? 1.0f : 0.0f;
    case InterpolationDef::LINEAR: return t;
    case InterpolationDef::SMOOTH: return SmoothStep(t).v;
    case InterpolationDef::SMOOTHER: return SmootherStep(t).v;
    case InterpolationDef::QUADRATIC_EASE_IN: return QuadraticEaseIn(t).v;
    case InterpolationDef::QUADRATIC_EASE_OUT: return QuadraticEaseOut(t).v;
    case InterpolationDef::QUADRATIC_EASE_IN_OUT: return QuadraticEaseInOut(t).v;
    case InterpolationDef::QUARTIC_EASE_IN: return QuarticEaseIn(t).v;
    case InterpolationDef::QUARTIC_EASE_OUT: return QuarticEaseOut(t).v;
    case InterpolationDef::QUARTIC_EASE_IN_OUT: return QuarticEaseInOut(t).v;
    case InterpolationDef::OCTIC_EASE_IN: return OcticEaseIn(t).v;
    case InterpolationDef::OCTIC_EASE_OUT: return OcticEaseOut(t).v;
    case InterpolationDef::OCTIC_EASE_IN_OUT: return OcticEaseInOut(t).v;
    }
    return t;
}

math::quat<float> QLog(const math::quat<float>& q)
{
    float a = std::acos(math::clamp(q.w, 0.0f, 1.0f));
    float s = std::sin(a);
    if (s <= 0.0f)
        return math::quat<float>(0, 0, 0, 0);
    float k = a / s;
    return math::quat<float>(k * q.x, k * q.y, k * q.z, 0);
}

math::quat<float> SplineTangent(const math::quat<float>& q0, const math::quat<float>& q1, const math::quat<float>& q2)
{
    math::quat<float> inv(-q1.x, -q1.y, -q1.z, q1.w);
    math::quat<float> s = (QLog(inv * q0) + QLog(inv * q2)) * -0.25f;
    float a = std::sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
    float sn = std::sin(a);
    math::quat<float> e(0, 0, 0, std::cos(a));
    if (a > 0.0f) {
        float k = sn / a;
        e.x = k * s.x;
        e.y = k * s.y;
        e.z = k * s.z;
    }
    return q1 * e;
}

math::quat<float> SlerpNoInvert(const math::quat<float>& a, const math::quat<float>& b, float t)
{
    float d = math::dot(a, b), wa = 1.0f - t, wb = t;
    if (!(d > 0.999f || d < -0.999f)) {
        float th = std::acos(d), s = 1.0f / std::sin(th);
        wa = s * std::sin(th * (1.0f - t));
        wb = s * std::sin(th * t);
    }
    return a * wa + b * wb;
}

math::quat<float> InterpolateSpline(const math::quat<float>& q0, const math::quat<float>& q1i, const math::quat<float>& q2i,
                                    const math::quat<float>& q3i, float t)
{
    math::quat<float> q1 = q1i, q2 = q2i, q3 = q3i;
    if (math::dot(q0, q1) < 0.0f) q1 = -q1i;
    if (math::dot(q1, q2) < 0.0f) q2 = -q2i;
    if (math::dot(q2, q3) < 0.0f) q3 = -q3i;
    math::quat<float> a = SplineTangent(q0, q1, q2), b = SplineTangent(q1, q2, q3);
    return SlerpNoInvert(SlerpNoInvert(q1, q2, t), SlerpNoInvert(a, b, t), (t + t) * (1.0f - t));
}

}
}
