// core/smath/include/detail/sphere.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_SPHERE_H
#define CORE_SMATH_INCLUDE_DETAIL_SPHERE_H

#include "func_transform.h"

namespace math {

template <class T>
struct sphere {
    vec3<T> center;
    T radius;

    sphere() : radius(0) {}
    sphere(const vec3<T>& c, T r) : center(c), radius(r) {}

    void setCenter(const vec3<T>& c) { center = c; }
    void setRadius(T r) { radius = r; }
    const vec3<T>& getCenter() const { return center; }
    T getRadius() const { return radius; }
    bool contains(const vec3<T>& p) const { return distanceSq(center, p) <= radius * radius; }
    bool contains(const sphere& s) const { return math::distance(center, s.center) + s.radius <= radius; }
    bool overlaps(const sphere& s) const { T r = s.radius + radius; return (center - s.center).getLengthSq() <= r * r; }
    T distance(const vec3<T>& p) const { return math::distance(center, p) - radius; }
    void meld(const vec3<T>& p) { meld(sphere(p, 0)); }
    // Smallest sphere holding both; a zero radius counts as empty.
    void meld(const sphere& s)
    {
        if (radius == T(0)) {
            if (s.radius == T(0))
                return;
        } else {
            vec3<T> d = s.center - center;
            T l = d.getLength();
            if (l + s.radius <= radius)
                return;
            if (s.radius < l + radius) {
                T r = (l + s.radius + radius) * T(0.5);
                T k = (r - radius) / l;
                center = center + vec3<T>(d.x * k, d.y * k, d.z * k);
                radius = r;
                return;
            }
        }
        *this = s;
    }
};

typedef sphere<float> spheref;

}

#endif
