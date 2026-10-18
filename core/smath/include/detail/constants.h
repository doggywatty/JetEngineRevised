// core/smath/include/detail/constants.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_CONSTANTS_H
#define CORE_SMATH_INCLUDE_DETAIL_CONSTANTS_H

namespace math {

template <class T>
struct constants {
    static T pi() { return T(3.14159265358979323846); }
    static T twoPi() { return T(6.28318530717958647692); }
    static T halfPi() { return T(1.57079632679489661923); }
    static T epsilon() { return T(1e-6); }
    static T roundingError() { return T(1e-6); }
};

const float PI = 3.14159265358979323846f;
const float TWO_PI = 6.28318530717958647692f;
const float HALF_PI = 1.57079632679489661923f;
const float DEG_TO_RAD = PI / 180.0f;
const float RAD_TO_DEG = 180.0f / PI;

}

#endif
