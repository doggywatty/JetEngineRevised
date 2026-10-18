// core/jet/include/core/interputils.h
#ifndef CORE_JET_INCLUDE_CORE_INTERPUTILS_H
#define CORE_JET_INCLUDE_CORE_INTERPUTILS_H

#include "jenum.h"
#include "smath.h"

namespace jet {
namespace core {

// Easing curves over t in [0, 1]; each is a tiny functor holding the eased value.
struct LinearStep { float v; explicit LinearStep(float t) : v(t) {} };
struct SmoothStep { float v; explicit SmoothStep(float t) : v(t * t * (3.0f - (t + t))) {} };
struct SmootherStep { float v; explicit SmootherStep(float t) : v(t * t * t * ((t * 6.0f - 15.0f) * t + 10.0f)) {} };
struct QuadraticEaseIn { float v; explicit QuadraticEaseIn(float t) : v(t * t) {} };
struct QuadraticEaseOut { float v; explicit QuadraticEaseOut(float t) : v(1.0f - (1.0f - t) * (1.0f - t)) {} };
struct QuadraticEaseInOut {
    float v;
    explicit QuadraticEaseInOut(float t)
    {
        t += t;
        v = t < 1.0f ? 0.5f * t * t : -0.5f * ((t - 2.0f) * (t - 2.0f) - 2.0f);
    }
};
struct QuarticEaseIn { float v; explicit QuarticEaseIn(float t) : v(t * t * t * t) {} };
struct QuarticEaseOut { float v; explicit QuarticEaseOut(float t) { float u = (1.0f - t) * (1.0f - t); v = 1.0f - u * u; } };
struct QuarticEaseInOut {
    float v;
    explicit QuarticEaseInOut(float t)
    {
        t += t;
        if (t < 1.0f)
            v = t * t * t * t * 0.5f;
        else {
            float u = (t - 2.0f) * (t - 2.0f);
            v = (u * u - 2.0f) * -0.5f;
        }
    }
};
struct OcticEaseIn { float v; explicit OcticEaseIn(float t) { float u = t * t; u *= u; v = u * u; } };
struct OcticEaseOut { float v; explicit OcticEaseOut(float t) { float u = (1.0f - t) * (1.0f - t); u *= u; v = 1.0f - u * u; } };
struct OcticEaseInOut {
    float v;
    explicit OcticEaseInOut(float t)
    {
        t += t;
        if (t >= 1.0f) {
            float u = (t - 2.0f) * (t - 2.0f);
            u *= u;
            v = (u * u - 2.0f) * -0.5f;
        } else {
            float u = t * t * t * t;
            v = u * u * 0.5f;
        }
    }
};

struct InterpolationDef {
    enum type {
        STEP, LINEAR, SMOOTH, SMOOTHER, QUADRATIC_EASE_IN, QUADRATIC_EASE_OUT, QUADRATIC_EASE_IN_OUT,
        QUARTIC_EASE_IN, QUARTIC_EASE_OUT, QUARTIC_EASE_IN_OUT, OCTIC_EASE_IN, OCTIC_EASE_OUT, OCTIC_EASE_IN_OUT
    };
};
typedef safe_enum<InterpolationDef> Interpolation;

// t is clamped to [0, 1]. STEP jumps at t = 1.
float GetInterpolated(Interpolation type, float t);

math::quat<float> QLog(const math::quat<float>& q);
math::quat<float> SplineTangent(const math::quat<float>& q0, const math::quat<float>& q1, const math::quat<float>& q2);
math::quat<float> SlerpNoInvert(const math::quat<float>& a, const math::quat<float>& b, float t);
// Squad through q1..q2 with neighbours q0 and q3.
math::quat<float> InterpolateSpline(const math::quat<float>& q0, const math::quat<float>& q1, const math::quat<float>& q2,
                                    const math::quat<float>& q3, float t);

// Cubic through p1..p2 using p0/p3 as neighbours (value, tangent from the outer points).
template <class T>
T InterpolateCubic(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
    float t2 = t * t;
    T a = p3 - p2 - p0 + p1;
    T b = p0 - p1 - a;
    T c = p2 - p0;
    return a * (t * t2) + b * t2 + c * t + p1;
}

template <class T>
T InterpolateCatmullRom(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
    float t2 = t * t, t3 = t2 * t;
    T r = p1 * 2.0f + (p2 - p0) * t + (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 + (-p0 + p1 * 3.0f - p2 * 3.0f + p3) * t3;
    return r * 0.5f;
}

// Kochanek-Bartels style Hermite (tension, bias), scaled by the time spans of the neighbouring segments.
template <class T>
T InterpolateSpline(const T& p0, const T& p1, const T& p2, const T& p3, float t, float tension, float bias, float dtPrev,
                    float dtCur, float dtNext)
{
    float c1 = bias + 1.0f, c0 = 1.0f - bias, tn = 1.0f - tension;
    T d1 = (p1 - p0) * (tn * c1) + (p2 - p1) * (tn * c0);
    T d2 = (p2 - p1) * (tn * c1) + (p3 - p2) * (tn * c0);
    float s1 = 0.5f / ((dtCur + dtCur) / (dtCur + dtPrev)), s2 = 0.5f / ((dtCur + dtCur) / (dtCur + dtNext));
    float t2 = t * t, t3 = t2 * t;
    float h1 = (t3 + t3) - t2 * 3.0f + 1.0f, h2 = t2 * 3.0f + t3 * -2.0f, h3 = (t3 + t) - (t2 + t2), h4 = t3 - t2;
    return p1 * h1 + d1 * (h3 * s1) + d2 * (h4 * s2) + p2 * h2;
}

}
}

#endif
