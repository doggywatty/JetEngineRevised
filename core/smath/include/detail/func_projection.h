// core/smath/include/detail/func_projection.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_PROJECTION_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_PROJECTION_H

#include "func_transform.h"

namespace math {

// Despite the name this is glFrustum(left, right, bottom, top, near, far).
template <class T>
mat4<T> perspective(T l, T r, T b, T t, T n, T f)
{
    T rl = T(1) / (r - l), tb = T(1) / (t - b), nf = T(1) / (n - f);
    mat4<T> m;
    m.m[1] = m.m[2] = m.m[3] = m.m[4] = m.m[6] = m.m[7] = m.m[12] = m.m[13] = m.m[15] = 0;
    m.m[11] = -1;
    m.m[0] = rl * (n + n);
    m.m[5] = tb * (n + n);
    m.m[8] = rl * (r + l);
    m.m[9] = tb * (t + b);
    m.m[10] = nf * (n + f);
    m.m[14] = (f + f) * n * nf;
    return m;
}

// glOrtho centered on the origin: width w, height h.
template <class T>
mat4<T> orthoCenter(T w, T h, T n, T f)
{
    mat4<T> m;
    m.m[0] = T(2) / w;
    m.m[5] = T(2) / h;
    m.m[10] = T(-2) / (f - n);
    m.m[14] = -(f + n) / (f - n);
    return m;
}

// Basis for looking along dir: columns (right, -dir, up).
template <class T>
mat3<T> lookAt(const vec3<T>& dir, const vec3<T>& up)
{
    vec3<T> f(dir);
    f.normalize();
    f = -f;
    vec3<T> u(up);
    u.normalize();
    vec3<T> s = cross(f, u);
    s.normalize();
    vec3<T> v = cross(s, f);
    mat3<T> m;
    m.setColumn(0, s);
    m.setColumn(1, f);
    m.setColumn(2, v);
    return m;
}

// Clip-space projection of a point, and its normalized device coordinates.
template <class T> vec4<T> project(const mat4<T>& m, const vec4<T>& v) { return transform(m, v); }
template <class T>
vec3<T> project(const mat4<T>& m, const vec3<T>& v)
{
    vec4<T> c = transform(m, vec4<T>(v, T(1)));
    T i = c.w != T(0) ? T(1) / c.w : T(1);
    return vec3<T>(c.x * i, c.y * i, c.z * i);
}

}

#endif
