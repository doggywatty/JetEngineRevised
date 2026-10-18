// core/smath/include/detail/func_test.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_TEST_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_TEST_H

#include "quat.h"

namespace math {

// Relative comparison: |a - b| <= max(1, |a|, |b|) * eps.
inline bool equals(float a, float b, float eps = 1e-6f)
{
    float m = std::fabs(a) < 1.0f ? 1.0f : std::fabs(a);
    if (m < std::fabs(b))
        m = std::fabs(b);
    return std::fabs(a - b) <= m * eps;
}
inline bool equals(const vec2<float>& a, const vec2<float>& b, float e = 1e-6f) { return equals(a.x, b.x, e) && equals(a.y, b.y, e); }
inline bool equals(const vec3<float>& a, const vec3<float>& b, float e = 1e-6f)
{
    return equals(a.x, b.x, e) && equals(a.y, b.y, e) && equals(a.z, b.z, e);
}
inline bool equals(const vec4<float>& a, const vec4<float>& b, float e = 1e-6f)
{
    return equals(a.x, b.x, e) && equals(a.y, b.y, e) && equals(a.z, b.z, e) && equals(a.w, b.w, e);
}
inline bool equals(const quat<float>& a, const quat<float>& b, float e = 1e-6f)
{
    return equals(a.x, b.x, e) && equals(a.y, b.y, e) && equals(a.z, b.z, e) && equals(a.w, b.w, e);
}
inline bool equals(const mat4<float>& a, const mat4<float>& b, float e = 1e-6f)
{
    for (int i = 0; i < 16; ++i)
        if (!equals(a.m[i], b.m[i], e))
            return false;
    return true;
}

inline bool isZero(float a, float e = 1e-6f) { return std::fabs(a) <= e; }
inline bool isZero(double a, double e = 1e-12) { return std::fabs(a) <= e; }
inline bool isZero(const vec2<float>& v, float e = 1e-6f) { return isZero(v.x, e) && isZero(v.y, e); }
inline bool isZero(const vec3<float>& v, float e = 1e-6f) { return isZero(v.x, e) && isZero(v.y, e) && isZero(v.z, e); }
inline bool isOne(float a, float e = 1e-6f) { return std::fabs(a - 1.0f) <= e; }
inline bool isOne(const vec2<float>& v, float e = 1e-6f) { return isOne(v.x, e) && isOne(v.y, e); }
inline bool isOne(const vec3<float>& v, float e = 1e-6f) { return isOne(v.x, e) && isOne(v.y, e) && isOne(v.z, e); }
inline bool isIdentity(const mat4<float>& a, float e = 1e-6f)
{
    for (int i = 0; i < 16; ++i)
        if (std::fabs(a.m[i] - ((i % 5) ? 0.0f : 1.0f)) > e)
            return false;
    return true;
}
inline bool isIdentity(const quat<float>& q, float e = 1e-6f) { return isZero(q.x, e) && isZero(q.y, e) && isZero(q.z, e) && isOne(q.w, e); }

}

#endif
