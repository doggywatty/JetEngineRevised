// core/smath/include/detail/triangle3.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_TRIANGLE3_H
#define CORE_SMATH_INCLUDE_DETAIL_TRIANGLE3_H

#include "plane.h"

namespace math {

template <class T>
struct triangle3 {
    vec3<T> pointA, pointB, pointC;

    triangle3() {}
    triangle3(vec3<T> a, vec3<T> b, vec3<T> c) : pointA(a), pointB(b), pointC(c) {}

    void set(const vec3<T>& a, const vec3<T>& b, const vec3<T>& c) { pointA = a; pointB = b; pointC = c; }
    vec3<T> getNormal() const { return cross(pointB - pointA, pointC - pointA); }
    plane<T> getPlane() const { return plane<T>(pointA, pointB, pointC); }
    T getArea() const { return cross(pointB - pointA, pointC - pointA).getLength() * T(0.5); }
    bool isFrontFacing(const vec3<T>& dir) const { return dot(normalized(getNormal()), dir) <= T(0); }
    bool getIntersectionOfPlaneWithLine(const vec3<T>& p, const vec3<T>& dir, vec3<T>& out) const
    {
        vec3<T> n = cross(pointB - pointA, pointC - pointA);
        n.normalize();
        T t2 = dot(n, dir);
        if (!(std::fabs(t2) > T(1.1920928955078125e-07)))
            return false;
        T t = (dot(pointA, n) - dot(n, p)) / t2;
        out = p + dir * t;
        return true;
    }
    // Moller-Trumbore. With cullBack, rays from behind the triangle miss. Outputs distance and barycentrics.
    bool intersectRay(const vec3<T>& orig, const vec3<T>& dir, bool cullBack, T& t, T& u, T& v) const
    {
        vec3<T> e1 = pointB - pointA, e2 = pointC - pointA;
        if (cullBack && dot(cross(e1, e2), orig - pointA) < T(0))
            return false;
        vec3<T> p = cross(dir, e2);
        T det = dot(e1, p);
        if (!(std::fabs(det) > T(1.1920928955078125e-07)))
            return false;
        T inv = T(1) / det;
        vec3<T> s = orig - pointA;
        u = dot(s, p) * inv;
        if (!(T(0) <= u && u <= T(1)))
            return false;
        vec3<T> q = cross(s, e1);
        v = dot(dir, q) * inv;
        if (!(T(0) <= v && v + u <= T(1)))
            return false;
        t = dot(e2, q) * inv;
        return true;
    }
    bool isPointInside(const vec3<T>& p) const
    {
        return isOnSameSide(p, pointA, pointB, pointC) && isOnSameSide(p, pointB, pointA, pointC) && isOnSameSide(p, pointC, pointA, pointB);
    }
    vec3<T> closestPointOnTriangle(const vec3<T>& p) const
    {
        line3<T> ab(pointA, pointB), bc(pointB, pointC), ca(pointC, pointA);
        vec3<T> r1 = ab.getClosestPoint(p), r2 = bc.getClosestPoint(p), r3 = ca.getClosestPoint(p);
        T d1 = distanceSq(r1, p), d2 = distanceSq(r2, p), d3 = distanceSq(r3, p);
        return d1 < d2 ? (d1 < d3 ? r1 : r3) : (d2 < d3 ? r2 : r3);
    }

private:
    static bool isOnSameSide(const vec3<T>& p1, const vec3<T>& p2, const vec3<T>& a, const vec3<T>& b)
    {
        vec3<T> bma = b - a;
        return dot(cross(bma, p1 - a), cross(bma, p2 - a)) >= T(0);
    }
};

typedef triangle3<float> triangle3f;

}

#endif
