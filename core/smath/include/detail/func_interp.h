// core/smath/include/detail/func_interp.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_INTERP_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_INTERP_H

#include "func_transform.h"

namespace math {

// Slerp; linear when nearly aligned, and a hard switch at t = 0.5 when nearly opposite.
template <class T>
quat<T> lerp(const quat<T>& a, const quat<T>& b, float t)
{
    T d = dot(b, a);
    if (d > T(0.998))
        return quat<T>(t * b.x + (T(1) - t) * a.x, t * b.y + (T(1) - t) * a.y, t * b.z + (T(1) - t) * a.z, t * b.w + (T(1) - t) * a.w);
    if (d >= T(0)) {
        T th = T(std::acos(d)), s = T(1) / T(std::sin(th));
        T wa = T(std::sin((T(1) - t) * th)) * s, wb = T(std::sin(t * th)) * s;
        return quat<T>(wa * a.x + wb * b.x, wa * a.y + wb * b.y, wa * a.z + wb * b.z, wa * a.w + wb * b.w);
    }
    if (d <= T(-0.9998999834060669))
        return t >= T(0.5) ? b : a;
    T th = T(std::acos(-d)), s = T(1) / T(std::sin(th));
    T wa = T(std::sin((T(1) - t) * th)) * s, wb = T(std::sin(t * th)) * s;
    return quat<T>(-(a.x * wa) + wb * b.x, -(a.y * wa) + wb * b.y, -(a.z * wa) + wb * b.z, -(a.w * wa) + wb * b.w);
}

// Normalized linear interpolation along the shorter arc.
template <class T>
quat<T> nlerp(const quat<T>& a, const quat<T>& b, float t)
{
    T d = dot(b, a);
    quat<T> r;
    if (d < T(0)) {
        if (double(d) <= -0.9999)
            r = t >= T(0.5) ? b : a;
        else
            r.set(-(b.x * t) + (T(1) - t) * a.x, -(b.y * t) + (T(1) - t) * a.y, -(b.z * t) + (T(1) - t) * a.z,
                  -(b.w * t) + (T(1) - t) * a.w);
    } else {
        r.set(t * b.x + (T(1) - t) * a.x, t * b.y + (T(1) - t) * a.y, t * b.z + (T(1) - t) * a.z, t * b.w + (T(1) - t) * a.w);
    }
    return r.normalize();
}

}

#endif
