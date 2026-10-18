// core/smath/include/detail/plane.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_PLANE_H
#define CORE_SMATH_INCLUDE_DETAIL_PLANE_H

#include "func_transform.h"

namespace math {

// Points p with dot(normal, p) + d == 0.
template <class T>
struct plane {
    vec3<T> normal;
    T d;

    plane() : normal(0, 1, 0), d(0) {}
    plane(const vec3<T>& n, T d_) : normal(n), d(d_) {}
    plane(const vec3<T>& point, const vec3<T>& n) : normal(n), d(-dot(point, normal)) {}
    plane(const vec3<T>& a, const vec3<T>& b, const vec3<T>& c) { setPlane(a, b, c); }

    void setPlane(const vec3<T>& point, const vec3<T>& n) { normal = n; d = -dot(point, normal); }
    void setPlane(const vec3<T>& n, T d_) { normal = n; d = d_; }
    void setPlane(const vec3<T>& a, const vec3<T>& b, const vec3<T>& c)
    {
        normal = cross(b - a, c - a);
        normal.normalize();
        d = -dot(a, normal);
    }
    void recalculateD(const vec3<T>& p) { d = -dot(p, normal); }
    void translate(const vec3<T>& t) { setPlane(normal * -d + t, normal); }
    vec3<T> getMemberPoint() const { return normal * -d; }
    bool isFrontFacing(const vec3<T>& dir) const { return dot(normal, dir) <= T(0); }
    T getDistanceTo(const vec3<T>& p) const { return dot(p, normal) + d; }
    vec3<T> projectPoint(const vec3<T>& p) const { return p - normal * (dot(normal, p) + d); }
    plane& invert() { normal = -normal; d = -d; return *this; }
    bool getIntersectionWithLine(const vec3<T>& p, const vec3<T>& dir, vec3<T>& out) const
    {
        T t2 = dot(normal, dir);
        if (t2 == T(0))
            return false;
        T t = -(dot(normal, p) + d) / t2;
        out = p + dir * t;
        return true;
    }
    T getKnownIntersectionWithLine(const vec3<T>& a, const vec3<T>& b) const
    {
        vec3<T> v = b - a;
        return (dot(normal, a) + d) / -dot(normal, v);
    }
    bool getIntersectionWithLimitedLine(const vec3<T>& a, const vec3<T>& b, vec3<T>& out) const
    {
        if (!getIntersectionWithLine(a, b - a, out))
            return false;
        T f = (b - a).getLengthSq();
        return distanceSq(out, a) <= f && distanceSq(out, b) <= f;
    }
    RelationEnum classifyPointRelation(const vec3<T>& p) const
    {
        T x = dot(normal, p) + d;
        if (x < -T(1e-6))
            return REL_BACK;
        if (x > T(1e-6))
            return REL_FRONT;
        return REL_PLANAR;
    }
    bool existsIntersection(const plane& o) const { return cross(o.normal, normal).getLength() > T(1e-6); }
    bool getIntersectionWithPlane(const plane& o, vec3<T>& linePoint, vec3<T>& lineVect) const
    {
        T fn00 = normal.getLength(), fn01 = dot(normal, o.normal), fn11 = o.normal.getLength();
        double det = double(fn00) * fn11 - double(fn01) * fn01;
        if (std::fabs(det) < 1e-8)
            return false;
        double inv = 1.0 / det;
        double fc0 = (double(fn11) * -d + double(fn01) * o.d) * inv;
        double fc1 = (double(fn00) * -o.d + double(fn01) * d) * inv;
        lineVect = cross(normal, o.normal);
        linePoint = normal * T(fc0) + o.normal * T(fc1);
        return true;
    }
    bool getIntersectionWithPlanes(const plane& o1, const plane& o2, vec3<T>& out) const
    {
        vec3<T> lp, lv;
        return getIntersectionWithPlane(o1, lp, lv) && o2.getIntersectionWithLine(lp, lv, out);
    }
    // 1 in front, -1 behind, 0 when the sphere of radius |halfExtent| around c touches the plane.
    int overlap(const vec3<T>& c, vec3<T>& halfExtent) const
    {
        T r = std::fabs(halfExtent.x * normal.x) + std::fabs(halfExtent.y * normal.y) + std::fabs(halfExtent.z * normal.z);
        T x = getDistanceTo(c);
        return x > r ? 1 : x < -r ? -1 : 0;
    }
};

typedef plane<float> planef;

}

#endif
