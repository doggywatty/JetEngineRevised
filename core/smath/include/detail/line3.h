// core/smath/include/detail/line3.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_LINE3_H
#define CORE_SMATH_INCLUDE_DETAIL_LINE3_H

#include "func_test.h"
#include "func_transform.h"

namespace math {

template <class T>
struct line3 {
    vec3<T> start, end;

    line3() : start(0, 0, 0), end(1, 1, 1) {}
    line3(T xa, T ya, T za, T xb, T yb, T zb) : start(xa, ya, za), end(xb, yb, zb) {}
    line3(const vec3<T>& a, const vec3<T>& b) : start(a), end(b) {}

    void setLine(const T& xa, const T& ya, const T& za, const T& xb, const T& yb, const T& zb) { start.set(xa, ya, za); end.set(xb, yb, zb); }
    void setLine(const vec3<T>& a, const vec3<T>& b) { start = a; end = b; }
    void setLine(const line3& l) { *this = l; }
    T getLength() const { return (end - start).getLength(); }
    T getLengthSQ() const { return (end - start).getLengthSq(); }
    vec3<T> getMiddle() const { return (start + end) * T(0.5); }
    vec3<T> getVector() const { return end - start; }
    // Normalized in double precision, as shipped.
    vec3<T> getVectorNormalized() const
    {
        double x = double(end.x) - start.x, y = double(end.y) - start.y, z = double(end.z) - start.z;
        double l = std::sqrt(z * z + y * y + x * x);
        if (std::fabs(l) > 2.220446049250313e-16) {
            l = 1.0 / l;
            x *= l;
            y *= l;
            z *= l;
        }
        return vec3<T>(T(x), T(y), T(z));
    }
    bool isBetweenPoints(const vec3<T>& p, const vec3<T>& a, const vec3<T>& b) const
    {
        T f = (b - a).getLengthSq();
        return distanceSq(p, a) <= f && distanceSq(p, b) <= f;
    }
    bool isPointBetweenStartAndEnd(const vec3<T>& p) const { return isBetweenPoints(p, start, end); }
    bool isPointOnLine(const vec3<T>& p) const
    {
        vec3<T> d = end - start;
        return isZero(cross(d, p - start), T(1e-6)) && isPointBetweenStartAndEnd(p);
    }
    // Closest point of the segment to p.
    vec3<T> getClosestPoint(const vec3<T>& p) const
    {
        vec3<T> c = p - start, v = end - start;
        T d = v.getLength();
        if (std::fabs(d) > T(1.1920928955078125e-07)) {
            T i = T(1) / d;
            v = vec3<T>(v.x * i, v.y * i, v.z * i);
            T t = dot(v, c);
            if (t >= T(0)) {
                if (t <= d) {
                    v *= t;
                    return start + v;
                }
                return end;
            }
        }
        return start;
    }
    // Parameter along the (unbounded) line of the projection of p, in units of the segment length.
    T getClosestPointProjected(const vec3<T>& p) const
    {
        vec3<T> v = end - start;
        T l = v.getLengthSq();
        return l > T(0) ? dot(p - start, v) / l : T(0);
    }
    bool getIntersectionWithSphere(const vec3<T>& c, T r, double& outDist) const
    {
        vec3<T> q = c - start, v = getVectorNormalized();
        double cl = q.getLength(), vd = dot(q, v);
        double d = double(r) * r - (cl * cl - vd * vd);
        if (d < 0.0)
            return false;
        outDist = vd - std::sqrt(d);
        return true;
    }
    bool getIntersectionWithSphere(const vec3<T>& c, T r, double& d0, double& d1) const
    {
        vec3<T> q = c - start, v = getVectorNormalized();
        double cl = q.getLength(), vd = dot(q, v);
        double d = double(r) * r - (cl * cl - vd * vd);
        if (d < 0.0)
            return false;
        d = std::sqrt(d);
        d0 = vd - d;
        d1 = vd + d;
        return true;
    }
    bool isIntersectingWithSphere(const vec3<T>& c, T r) const { return distanceSq(getClosestPoint(c), c) <= r * r; }
    // Intersection of the two infinite lines when they cross (closest points coincide).
    bool getIntersectionWithLine(const line3& o, vec3<T>& out) const
    {
        vec3<T> d1 = getVector(), d2 = o.getVector(), r = start - o.start;
        T a = dot(d1, d1), e = dot(d2, d2), f = dot(d2, r), c = dot(d1, r), b = dot(d1, d2);
        T den = a * e - b * b;
        if (std::fabs(den) <= T(1.1920928955078125e-07))
            return false;
        T s = (b * f - c * e) / den, t = (a * f - b * c) / den;
        vec3<T> p1 = start + d1 * s, p2 = o.start + d2 * t;
        if (!isZero(p1 - p2, T(1e-4)))
            return false;
        out = p1;
        return true;
    }
};

typedef line3<float> line3f;

}

#endif
