// core/smath/include/detail/aabb.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_AABB_H
#define CORE_SMATH_INCLUDE_DETAIL_AABB_H

#include "line3.h"

namespace math {

template <class T>
struct aabb {
    vec3<T> minPoint, maxPoint;

    aabb() : minPoint(-1, -1, -1), maxPoint(1, 1, 1) {}
    aabb(const vec3<T>& mn, const vec3<T>& mx) : minPoint(mn), maxPoint(mx) {}
    explicit aabb(const vec3<T>& p) : minPoint(p), maxPoint(p) {}

    void reset(T x, T y, T z) { minPoint.set(x, y, z); maxPoint = minPoint; }
    void reset(const vec3<T>& p) { minPoint = maxPoint = p; }
    void reset(const vec3<T>& mn, const vec3<T>& mx) { minPoint = mn; maxPoint = mx; }
    void reset(const aabb& b) { *this = b; }
    // As shipped: the extents are rotated with m[0..2], m[4..6], m[8..10] taken as rows (the transpose).
    void reset(const mat4<T>& m, const aabb& b)
    {
        vec3<T> c = (b.minPoint + b.maxPoint);
        c = vec3<T>(c.x * T(0.5), c.y * T(0.5), c.z * T(0.5));
        vec3<T> h = b.maxPoint - b.minPoint;
        h = vec3<T>(h.x * T(0.5), h.y * T(0.5), h.z * T(0.5));
        vec3<T> e(h.x * std::fabs(m.m[0]) + h.y * std::fabs(m.m[1]) + h.z * std::fabs(m.m[2]),
                  h.x * std::fabs(m.m[4]) + h.y * std::fabs(m.m[5]) + h.z * std::fabs(m.m[6]),
                  h.x * std::fabs(m.m[8]) + h.y * std::fabs(m.m[9]) + h.z * std::fabs(m.m[10]));
        c = transform(m, c);
        maxPoint = c + e;
        minPoint = c - e;
    }
    vec3<T> getExtent() const { return maxPoint - minPoint; }
    T getRadius() const { return getExtent().getLength() * T(0.5); }
    T getRadiusFast() const { return T(std::sqrt(getExtent().getLengthSq() * T(0.25))); }
    T getRadiusSq() const { return getExtent().getLengthSq() * T(0.25); }
    T getInnerRadius() const
    {
        vec3<T> e = getExtent();
        T m = e.x < e.y ? e.x : e.y;
        return (m < e.z ? m : e.z) * T(0.5);
    }
    T getInnerRadiusSq() const { T r = getInnerRadius(); return r * r; }
    void getEdges(vec3<T>* e) const
    {
        const vec3<T> c = getCenter(), d = c - maxPoint;
        e[0].set(c.x + d.x, c.y + d.y, c.z + d.z);
        e[1].set(c.x + d.x, c.y - d.y, c.z + d.z);
        e[2].set(c.x + d.x, c.y + d.y, c.z - d.z);
        e[3].set(c.x + d.x, c.y - d.y, c.z - d.z);
        e[4].set(c.x - d.x, c.y + d.y, c.z + d.z);
        e[5].set(c.x - d.x, c.y - d.y, c.z + d.z);
        e[6].set(c.x - d.x, c.y + d.y, c.z - d.z);
        e[7].set(c.x - d.x, c.y - d.y, c.z - d.z);
    }
    bool isEmpty() const { return minPoint == maxPoint; }
    void repair()
    {
        for (int i = 0; i < 3; ++i)
            if (maxPoint[i] < minPoint[i]) { T t = minPoint[i]; minPoint[i] = maxPoint[i]; maxPoint[i] = t; }
    }
    aabb getInterpolated(const aabb& o, T d) const
    {
        T i = T(1) - d;
        return aabb(o.minPoint * i + minPoint * d, o.maxPoint * i + maxPoint * d);
    }
    vec3<T> getCenter() const { return (minPoint + maxPoint) / T(2); }
    T getVolume() const { vec3<T> e = getExtent(); return e.x * e.y * e.z; }
    T getArea() const { vec3<T> e = getExtent(); return 2 * (e.x * e.y + e.x * e.z + e.y * e.z); }
    bool isPointInside(const vec3<T>& p) const
    {
        return p.x >= minPoint.x && p.x <= maxPoint.x && p.y >= minPoint.y && p.y <= maxPoint.y && p.z >= minPoint.z && p.z <= maxPoint.z;
    }
    bool isPointTotalInside(const vec3<T>& p) const
    {
        return p.x > minPoint.x && p.x < maxPoint.x && p.y > minPoint.y && p.y < maxPoint.y && p.z > minPoint.z && p.z < maxPoint.z;
    }
    bool isFullInside(const aabb& o) const
    {
        return minPoint.x >= o.minPoint.x && minPoint.y >= o.minPoint.y && minPoint.z >= o.minPoint.z && maxPoint.x <= o.maxPoint.x &&
               maxPoint.y <= o.maxPoint.y && maxPoint.z <= o.maxPoint.z;
    }
    void addInternalPoint(T x, T y, T z)
    {
        if (maxPoint.x < x) maxPoint.x = x;
        if (maxPoint.y < y) maxPoint.y = y;
        if (maxPoint.z < z) maxPoint.z = z;
        if (x < minPoint.x) minPoint.x = x;
        if (y < minPoint.y) minPoint.y = y;
        if (z < minPoint.z) minPoint.z = z;
    }
    void addInternalPoint(const vec3<T>& p) { addInternalPoint(p.x, p.y, p.z); }
    void addInternalBox(const aabb& b) { addInternalPoint(b.maxPoint); addInternalPoint(b.minPoint); }
    void addInternalBox(const mat4<T>& m, const aabb& b) { aabb t; t.reset(m, b); addInternalBox(t); }
    bool intersectsWithBox(const aabb& o) const
    {
        return minPoint.x <= o.maxPoint.x && minPoint.y <= o.maxPoint.y && minPoint.z <= o.maxPoint.z && maxPoint.x >= o.minPoint.x &&
               maxPoint.y >= o.minPoint.y && maxPoint.z >= o.minPoint.z;
    }
    bool intersectsWithLine(const line3<T>& l) const
    {
        return intersectsWithLine(l.getMiddle(), l.getVectorNormalized(), l.getLength() * T(0.5));
    }
    // Irrlicht's separating-axis test for a segment given by middle, direction and half length.
    bool intersectsWithLine(const vec3<T>& mid, const vec3<T>& dir, T half) const
    {
        const vec3<T> e = getExtent() * T(0.5);
        const vec3<T> t = getCenter() - mid;
        if (std::fabs(t.x) > e.x + half * std::fabs(dir.x) || std::fabs(t.y) > e.y + half * std::fabs(dir.y) ||
            std::fabs(t.z) > e.z + half * std::fabs(dir.z))
            return false;
        T r = e.y * std::fabs(dir.z) + e.z * std::fabs(dir.y);
        if (std::fabs(t.y * dir.z - t.z * dir.y) > r)
            return false;
        r = e.x * std::fabs(dir.z) + e.z * std::fabs(dir.x);
        if (std::fabs(t.z * dir.x - t.x * dir.z) > r)
            return false;
        r = e.x * std::fabs(dir.y) + e.y * std::fabs(dir.x);
        return std::fabs(t.x * dir.y - t.y * dir.x) <= r;
    }
    // Slab test; fills the entry and exit points.
    bool intersectsWithRay(const line3<T>& ray, vec3<T>& in, vec3<T>& out) const
    {
        vec3<T> o = ray.start, d = ray.getVector();
        T t0 = 0, t1 = 1;
        for (int i = 0; i < 3; ++i) {
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
    bool operator==(const aabb& o) const { return minPoint == o.minPoint && maxPoint == o.maxPoint; }
    bool operator!=(const aabb& o) const { return !(*this == o); }
};

typedef aabb<float> aabbf;

}

#endif
