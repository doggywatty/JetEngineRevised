// core/smath/include/detail/func_rayintersect.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_RAYINTERSECT_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_RAYINTERSECT_H

#include "aabb.h"
#include "plane.h"

namespace math {

// Real roots of a*t^2 + b*t + c = 0, ascending; returns their count.
template <class T>
int quadraticEquation(T a, T b, T c, T& t0, T& t1)
{
    if (std::fabs(a) < T(1e-12)) {
        if (std::fabs(b) < T(1e-12))
            return 0;
        t0 = t1 = -c / b;
        return 1;
    }
    T d = b * b - T(4) * a * c;
    if (d < T(0))
        return 0;
    T s = T(std::sqrt(d)), q = b < T(0) ? T(-0.5) * (b - s) : T(-0.5) * (b + s);
    t0 = q / a;
    t1 = q != T(0) ? c / q : t0;
    if (t0 > t1) { T x = t0; t0 = t1; t1 = x; }
    return d == T(0) ? 1 : 2;
}

// The ray-* tests return the number of hits (0..2) with distances along dir (not necessarily normalized) in t0 <= t1.
template <class T>
int rayIntersectSphere(const vec3<T>& o, const vec3<T>& dir, const vec3<T>& c, T r, T& t0, T& t1)
{
    vec3<T> l = o - c;
    return quadraticEquation(dot(dir, dir), T(2) * dot(dir, l), dot(l, l) - r * r, t0, t1);
}

template <class T>
bool rayIntersectPlane(const vec3<T>& o, const vec3<T>& dir, const plane<T>& p, T& t)
{
    T den = dot(p.normal, dir);
    if (std::fabs(den) < T(1e-12))
        return false;
    t = -(dot(p.normal, o) + p.d) / den;
    return true;
}

template <class T>
bool rayIntersectDisk(const vec3<T>& o, const vec3<T>& dir, const vec3<T>& c, const vec3<T>& n, T r, T& t)
{
    if (!rayIntersectPlane(o, dir, plane<T>(c, n), t))
        return false;
    return distanceSq(o + dir * t, c) <= r * r;
}

// Infinite cylinder through a with axis direction axis (unit).
template <class T>
int rayIntersectCylinder(const vec3<T>& o, const vec3<T>& dir, const vec3<T>& a, const vec3<T>& axis, T r, T& t0, T& t1)
{
    vec3<T> d = dir - axis * dot(dir, axis), l = (o - a) - axis * dot(o - a, axis);
    return quadraticEquation(dot(d, d), T(2) * dot(d, l), dot(l, l) - r * r, t0, t1);
}

// Hemisphere of center c facing n.
template <class T>
int rayIntersectHemisphere(const vec3<T>& o, const vec3<T>& dir, const vec3<T>& c, const vec3<T>& n, T r, T& t0, T& t1)
{
    T a, b;
    int k = rayIntersectSphere(o, dir, c, r, a, b);
    int m = 0;
    T h[2];
    for (int i = 0; i < k; ++i) {
        T t = i ? b : a;
        if (dot(o + dir * t - c, n) >= T(0))
            h[m++] = t;
    }
    if (m) { t0 = h[0]; t1 = h[m - 1]; }
    return m;
}

// Capsule between p and q.
template <class T>
int rayIntersectCapsule(const vec3<T>& o, const vec3<T>& dir, const vec3<T>& p, const vec3<T>& q, T r, T& t0, T& t1)
{
    vec3<T> axis = normalized(q - p);
    T len = distance(p, q), best0 = 0, best1 = 0;
    int n = 0;
    auto add = [&](T t) {
        if (!n) best0 = best1 = t;
        else { if (t < best0) best0 = t; if (t > best1) best1 = t; }
        ++n;
    };
    T a, b;
    int k = rayIntersectCylinder(o, dir, p, axis, r, a, b);
    for (int i = 0; i < k; ++i) {
        T t = i ? b : a, s = dot(o + dir * t - p, axis);
        if (s >= T(0) && s <= len)
            add(t);
    }
    k = rayIntersectHemisphere(o, dir, p, -axis, r, a, b);
    for (int i = 0; i < k; ++i)
        add(i ? b : a);
    k = rayIntersectHemisphere(o, dir, q, axis, r, a, b);
    for (int i = 0; i < k; ++i)
        add(i ? b : a);
    if (!n)
        return 0;
    t0 = best0;
    t1 = best1;
    return best0 == best1 ? 1 : 2;
}

template <class T>
int rayIntersectAABB(const vec3<T>& o, const vec3<T>& dir, const aabb<T>& box, T& t0, T& t1)
{
    T lo = -T(1e30), hi = T(1e30);
    for (int i = 0; i < 3; ++i) {
        if (std::fabs(dir[i]) < T(1e-12)) {
            if (o[i] < box.minPoint[i] || o[i] > box.maxPoint[i])
                return 0;
            continue;
        }
        T a = (box.minPoint[i] - o[i]) / dir[i], b = (box.maxPoint[i] - o[i]) / dir[i];
        if (a > b) { T s = a; a = b; b = s; }
        if (a > lo) lo = a;
        if (b < hi) hi = b;
        if (lo > hi)
            return 0;
    }
    t0 = lo;
    t1 = hi;
    return lo == hi ? 1 : 2;
}

}

#endif
