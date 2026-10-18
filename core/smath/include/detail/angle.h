// core/smath/include/detail/angle.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_ANGLE_H
#define CORE_SMATH_INCLUDE_DETAIL_ANGLE_H

#include "constants.h"

namespace math {

// Wraps an angle into (-pi, pi].
template <class T>
T wrapAngle(T a)
{
    const T tp = constants<T>::twoPi();
    a = T(std::fmod(a + constants<T>::pi(), tp));
    if (a <= T(0))
        a += tp;
    return a - constants<T>::pi();
}

// Signed shortest difference b - a.
template <class T>
T angleDiff(T a, T b) { return wrapAngle(b - a); }

}

#endif
