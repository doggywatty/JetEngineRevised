// core/smath/include/detail/line2.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_LINE2_H
#define CORE_SMATH_INCLUDE_DETAIL_LINE2_H

#include "func_transform.h"
#include "rect.h"

namespace math {

template <class T>
struct line2 {
    vec2<T> start, end;

    line2() : start(0, 0), end(1, 1) {}
    line2(T xa, T ya, T xb, T yb) : start(xa, ya), end(xb, yb) {}
    line2(const vec2<T>& a, const vec2<T>& b) : start(a), end(b) {}

    void setLine(const T& xa, const T& ya, const T& xb, const T& yb) { start.set(xa, ya); end.set(xb, yb); }
    void setLine(const vec2<T>& a, const vec2<T>& b) { start = a; end = b; }
    void setLine(const line2& l) { *this = l; }
    T getLength() const { return (end - start).getLength(); }
    T getLengthSQ() const { return (end - start).getLengthSq(); }
    vec2<T> getMiddle() const { return (start + end) * T(0.5); }
    vec2<T> getVector() const { return end - start; }
    vec2<T> getVectorNormalized() const { vec2<T> v = end - start; return v.normalize(); }
    bool isPointBetweenStartAndEnd(const vec2<T>& p) const
    {
        T f = (end - start).getLengthSq();
        return distanceSq(p, start) <= f && distanceSq(p, end) <= f;
    }
    vec2<T> getClosestPoint(const vec2<T>& p) const
    {
        vec2<T> c = p - start, v = end - start;
        T d = v.getLength();
        if (std::fabs(d) <= T(1.1920928955078125e-07))
            return start;
        v /= d;
        T t = dot(v, c);
        if (t < T(0))
            return start;
        if (t > d)
            return end;
        return start + v * t;
    }
    // Segment/segment; outT is the parameter along this segment.
    bool getIntersectionWithLine(const line2& l, vec2<T>* out, T* outT) const
    {
        T dy = end.y - start.y, dx = end.x - start.x;
        T ex = l.start.x - l.end.x, ey = l.start.y - l.end.y;
        T den = ey * dx - ex * dy;
        if (std::fabs(den) <= T(1.1920928955078125e-07))
            return false;
        T fy = l.start.y - start.y, fx = l.start.x - start.x;
        T a = (fx * ey - fy * ex) / den;
        if (!(a <= T(1) && T(0) <= a))
            return false;
        T b = (fy * dx - fx * dy) / den;
        if (!(b <= T(1) && T(0) <= b))
            return false;
        if (out) {
            out->x = start.x + a * dx;
            out->y = start.y + a * dy;
        }
        if (outT)
            *outT = a;
        return true;
    }
    bool getIntersectionWithCircle(vec2<T> c, T r, double* outDist) const
    {
        vec2<T> v = end - start, q = c - start;
        T l = v.getLength();
        if (std::fabs(l) > T(1.1920928955078125e-07))
            v *= T(1) / l;
        T ql = q.getLength();
        T vd = v.x * q.x + v.y * q.y;
        T d = (r * r - ql * ql) + vd * vd;
        if (d < T(0))
            return false;
        if (outDist)
            *outDist = double(vd) - std::sqrt(double(d));
        return true;
    }
    // Tests the top, right, bottom and left edges in that order.
    bool getIntersectionWithRect(const rect<T>& rc, vec2<T>* out, T* outT)
    {
        const vec2<T>&a = rc.upperLeftCorner, &b = rc.lowerRightCorner;
        return getIntersectionWithLine(line2(a.x, a.y, b.x, a.y), out, outT) || getIntersectionWithLine(line2(b.x, a.y, b.x, b.y), out, outT) ||
               getIntersectionWithLine(line2(b.x, b.y, a.x, b.y), out, outT) || getIntersectionWithLine(line2(a.x, b.y, a.x, a.y), out, outT);
    }
};

typedef line2<float> line2f;

}

#endif
