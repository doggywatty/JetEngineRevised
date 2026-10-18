// core/smath/include/detail/aabb2.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_AABB2_H
#define CORE_SMATH_INCLUDE_DETAIL_AABB2_H

#include "line2.h"

namespace math {

template <class T>
struct aabb2 {
    vec2<T> minPoint, maxPoint;

    aabb2() : minPoint(-1, -1), maxPoint(1, 1) {}
    aabb2(T x0, T y0, T x1, T y1) : minPoint(x0, y0), maxPoint(x1, y1) {}
    explicit aabb2(const vec2<T>& p) : minPoint(p), maxPoint(p) {}
    aabb2(const vec2<T>& mn, const vec2<T>& mx) : minPoint(mn), maxPoint(mx) {}

    void reset(T x, T y) { minPoint.set(x, y); maxPoint = minPoint; }
    void reset(const vec2<T>& p) { minPoint = maxPoint = p; }
    void reset(const vec2<T>& mn, const vec2<T>& mx) { minPoint = mn; maxPoint = mx; }
    void reset(const aabb2& b) { *this = b; }
    // Same transposed-rotation extent as aabb::reset(mat4, aabb).
    void reset(const mat3<T>& m, const aabb2& b)
    {
        vec2<T> c = (b.minPoint + b.maxPoint) * T(0.5), h = (b.maxPoint - b.minPoint) * T(0.5);
        vec2<T> e(h.x * std::fabs(m.m[0]) + h.y * std::fabs(m.m[1]), h.x * std::fabs(m.m[3]) + h.y * std::fabs(m.m[4]));
        c = transform(m, c);
        maxPoint = c + e;
        minPoint = c - e;
    }
    vec2<T> getExtent() const { return maxPoint - minPoint; }
    T getRadius() const { return getExtent().getLength() * T(0.5); }
    T getRadiusFast() const { return T(std::sqrt(getExtent().getLengthSq() * T(0.25))); }
    T getRadiusSq() const { return getExtent().getLengthSq() * T(0.25); }
    T getInnerRadius() const { vec2<T> e = getExtent(); return (e.x < e.y ? e.x : e.y) * T(0.5); }
    T getInnerRadiusSq() const { T r = getInnerRadius(); return r * r; }
    void getEdges(vec2<T>* e) const
    {
        e[0] = minPoint;
        e[1].set(maxPoint.x, minPoint.y);
        e[2] = maxPoint;
        e[3].set(minPoint.x, maxPoint.y);
    }
    bool isEmpty() const { return minPoint == maxPoint; }
    void repair()
    {
        for (int i = 0; i < 2; ++i)
            if (maxPoint[i] < minPoint[i]) { T t = minPoint[i]; minPoint[i] = maxPoint[i]; maxPoint[i] = t; }
    }
    aabb2 getInterpolated(const aabb2& o, T d) const
    {
        T i = T(1) - d;
        return aabb2(o.minPoint * i + minPoint * d, o.maxPoint * i + maxPoint * d);
    }
    vec2<T> getCenter() const { return (minPoint + maxPoint) / T(2); }
    T getArea() const { vec2<T> e = getExtent(); return e.x * e.y; }
    bool isPointInside(const vec2<T>& p) const { return p.x >= minPoint.x && p.x <= maxPoint.x && p.y >= minPoint.y && p.y <= maxPoint.y; }
    bool isPointTotalInside(const vec2<T>& p) const { return p.x > minPoint.x && p.x < maxPoint.x && p.y > minPoint.y && p.y < maxPoint.y; }
    bool isFullInside(const aabb2& o) const
    {
        return minPoint.x >= o.minPoint.x && minPoint.y >= o.minPoint.y && maxPoint.x <= o.maxPoint.x && maxPoint.y <= o.maxPoint.y;
    }
    void addInternalPoint(T x, T y)
    {
        if (maxPoint.x < x) maxPoint.x = x;
        if (maxPoint.y < y) maxPoint.y = y;
        if (x < minPoint.x) minPoint.x = x;
        if (y < minPoint.y) minPoint.y = y;
    }
    void addInternalPoint(const vec2<T>& p) { addInternalPoint(p.x, p.y); }
    void addInternalBox(const aabb2& b) { addInternalPoint(b.maxPoint); addInternalPoint(b.minPoint); }
    void addInternalBox(const mat3<T>& m, const aabb2& b) { aabb2 t; t.reset(m, b); addInternalBox(t); }
    bool intersectsWithBox(const aabb2& o) const
    {
        return minPoint.x <= o.maxPoint.x && minPoint.y <= o.maxPoint.y && maxPoint.x >= o.minPoint.x && maxPoint.y >= o.minPoint.y;
    }
    bool intersectsWithLine(const line2<T>& l) const { return intersectsWithLine(l.getMiddle(), l.getVectorNormalized(), l.getLength() * T(0.5)); }
    bool intersectsWithLine(const vec2<T>& mid, const vec2<T>& dir, T half) const
    {
        const vec2<T> e = getExtent() * T(0.5), t = getCenter() - mid;
        if (std::fabs(t.x) > e.x + half * std::fabs(dir.x) || std::fabs(t.y) > e.y + half * std::fabs(dir.y))
            return false;
        return std::fabs(t.x * dir.y - t.y * dir.x) <= e.x * std::fabs(dir.y) + e.y * std::fabs(dir.x);
    }
    bool intersectsWithRay(const line2<T>& ray, vec2<T>& in, vec2<T>& out) const
    {
        vec2<T> o = ray.start, d = ray.getVector();
        T t0 = 0, t1 = 1;
        for (int i = 0; i < 2; ++i) {
            if (std::fabs(d[i]) < T(1e-12)) {
                if (o[i] < minPoint[i] || o[i] > maxPoint[i])
                    return false;
                continue;
            }
            T a = (minPoint[i] - o[i]) / d[i], b = (maxPoint[i] - o[i]) / d[i];
            if (a > b) { T s = a; a = b; b = s; }
            if (a > t0) t0 = a;
            if (b < t1) t1 = b;
            if (t0 > t1)
                return false;
        }
        in = o + d * t0;
        out = o + d * t1;
        return true;
    }
};

typedef aabb2<float> aabb2f;

}

#endif
