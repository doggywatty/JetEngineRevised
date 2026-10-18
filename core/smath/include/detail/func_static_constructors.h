// core/smath/include/detail/func_static_constructors.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_STATIC_CONSTRUCTORS_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_STATIC_CONSTRUCTORS_H

#include "func_projection.h"

namespace math {

template <class T> quat<T> quatAxisX(T a) { return quat<T>(quat<T>::axisX, a); }
template <class T> quat<T> quatAxisY(T a) { return quat<T>(quat<T>::axisY, a); }
template <class T> quat<T> quatAxisZ(T a) { return quat<T>(quat<T>::axisZ, a); }

// Same basis as lookAt, converted to a quaternion.
template <class T>
quat<T> quatLookAt(const vec3<T>& dir, const vec3<T>& up)
{
    return quat<T>(lookAt(dir, up));
}

// Shortest rotation taking direction a onto direction b.
template <class T>
quat<T> quatRotationFromTo(const vec3<T>& from, const vec3<T>& to)
{
    vec3<T> a = normalized(from), b = normalized(to);
    T d = dot(a, b);
    if (d >= T(1) - T(1e-6))
        return quat<T>();
    if (d <= T(-1) + T(1e-6)) {
        vec3<T> axis = cross(vec3<T>(1, 0, 0), a);
        if (axis.getLengthSq() < T(1e-6))
            axis = cross(vec3<T>(0, 1, 0), a);
        return quat<T>(quat<T>::angleAxis, T(3.14159265358979323846), normalized(axis));
    }
    // The engine rotates with the conjugate of the Hamilton quaternion (see quat.h).
    vec3<T> c = cross(a, b);
    quat<T> q(-c.x, -c.y, -c.z, T(1) + d);
    return q.normalize();
}

}

#endif
