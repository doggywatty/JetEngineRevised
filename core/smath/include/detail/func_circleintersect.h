// core/smath/include/detail/func_circleintersect.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_CIRCLEINTERSECT_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_CIRCLEINTERSECT_H

#include "func_transform.h"

namespace math {

// Intersection points of two circles; returns their count.
template <class T>
int circleIntersect(const vec2<T>& c0, T r0, const vec2<T>& c1, T r1, vec2<T>& p0, vec2<T>& p1)
{
    vec2<T> d = c1 - c0;
    T l = d.getLength();
    if (l > r0 + r1 || l < std::fabs(r0 - r1) || l == T(0))
        return 0;
    T a = (r0 * r0 - r1 * r1 + l * l) / (2 * l), h = T(std::sqrt(r0 * r0 - a * a));
    vec2<T> m = c0 + d * (a / l);
    p0.set(m.x + h * d.y / l, m.y - h * d.x / l);
    p1.set(m.x - h * d.y / l, m.y + h * d.x / l);
    return h == T(0) ? 1 : 2;
}

}

#endif
