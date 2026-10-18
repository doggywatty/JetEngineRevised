// core/smath/include/detail/func_transform.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_TRANSFORM_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_TRANSFORM_H

#include "func_common.h"
#include "quat.h"

namespace math {

template <class T> T dot(const vec2<T>& a, const vec2<T>& b) { return a.x * b.x + a.y * b.y; }
template <class T> T dot(const vec3<T>& a, const vec3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
template <class T> T dot(const vec4<T>& a, const vec4<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
template <class T> T dot(const quat<T>& a, const quat<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
template <class T>
vec3<T> cross(const vec3<T>& a, const vec3<T>& b)
{
    return vec3<T>(b.z * a.y - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
}
template <class T> T distanceSq(const vec2<T>& a, const vec2<T>& b) { return (a - b).getLengthSq(); }
template <class T> T distanceSq(const vec3<T>& a, const vec3<T>& b) { return (a - b).getLengthSq(); }
template <class T> T distance(const vec2<T>& a, const vec2<T>& b) { return (a - b).getLength(); }
template <class T> T distance(const vec3<T>& a, const vec3<T>& b) { return (a - b).getLength(); }
template <class T> vec2<T> normalized(const vec2<T>& v) { vec2<T> r(v); return r.normalize(); }
template <class T> vec3<T> normalized(const vec3<T>& v) { vec3<T> r(v); return r.normalize(); }
template <class T> vec4<T> normalized(const vec4<T>& v) { vec4<T> r(v); return r.normalize(); }
template <class T> quat<T> normalized(const quat<T>& v) { quat<T> r(v); return r.normalize(); }
template <class T>
vec3<T> normalizedFast(const vec3<T>& v)
{
    vec3<T> r(v);
    r *= fastInvSqrt(r.getLengthSq());
    return r;
}
template <class T> vec3<T> operator/(const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x / b.x, a.y / b.y, a.z / b.z); }
template <class T> vec3<T> operator*(const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z); }

// Point transforms (with translation) and direction rotations.
template <class T>
vec2<T> transform(const mat3<T>& m, const vec2<T>& v)
{
    return vec2<T>(v.x * m.m[0] + v.y * m.m[3] + m.m[6], v.x * m.m[1] + v.y * m.m[4] + m.m[7]);
}
template <class T>
vec3<T> transform(const mat3<T>& m, const vec3<T>& v)
{
    return vec3<T>(v.x * m.m[0] + v.y * m.m[3] + v.z * m.m[6], v.x * m.m[1] + v.y * m.m[4] + v.z * m.m[7],
                   v.x * m.m[2] + v.y * m.m[5] + v.z * m.m[8]);
}
template <class T>
vec3<T> transform(const mat4<T>& m, const vec3<T>& v)
{
    return vec3<T>(v.y * m.m[4] + v.x * m.m[0] + m.m[12] + v.z * m.m[8], v.y * m.m[5] + v.x * m.m[1] + m.m[13] + v.z * m.m[9],
                   v.y * m.m[6] + v.x * m.m[2] + m.m[14] + v.z * m.m[10]);
}
template <class T>
vec4<T> transform(const mat4<T>& m, const vec4<T>& v)
{
    vec4<T> r(vec4<T>::uninitialized);
    for (int i = 0; i < 4; ++i)
        r[i] = v.x * m.m[i] + v.y * m.m[4 + i] + v.z * m.m[8 + i] + v.w * m.m[12 + i];
    return r;
}
template <class T>
vec3<T> rotate(const mat4<T>& m, const vec3<T>& v)
{
    return vec3<T>(v.x * m.m[0] + v.y * m.m[4] + v.z * m.m[8], v.x * m.m[1] + v.y * m.m[5] + v.z * m.m[9],
                   v.x * m.m[2] + v.y * m.m[6] + v.z * m.m[10]);
}
template <class T>
void transformArray(const mat4<T>& m, const vec3<T>* in, vec3<T>* out, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
        out[i] = transform(m, in[i]);
}
// Rotates v by q (the engine's convention, see quat.h).
template <class T, class U>
vec3<T> operator*(const quat<T>& q, const vec3<U>& v)
{
    vec3<T> t(v.y * q.z - v.z * q.y, v.z * q.x - v.x * q.z, v.x * q.y - v.y * q.x);
    T w2 = q.w + q.w;
    vec3<T> a(t.x * w2, t.y * w2, t.z * w2);
    vec3<T> b((t.y * q.z - t.z * q.y) * 2, (t.z * q.x - t.x * q.z) * 2, (t.x * q.y - t.y * q.x) * 2);
    return vec3<T>(v) + a + b;
}
template <class T> mat4<T> inverse(const mat4<T>& m) { mat4<T> r(m); r.invert(); return r; }
template <class T> mat4<T> transposed(const mat4<T>& m) { mat4<T> r(m); return r.transpose(); }
// a * b for affine matrices (the last rows are taken as 0 0 0 1).
template <class T>
mat4<T> multiply43(const mat4<T>& a, const mat4<T>& b)
{
    mat4<T> r(mat4<T>::uninitialized);
    for (int c = 0; c < 4; ++c)
        for (int i = 0; i < 3; ++i)
            r.m[c * 4 + i] = a.m[i] * b.m[c * 4] + a.m[4 + i] * b.m[c * 4 + 1] + a.m[8 + i] * b.m[c * 4 + 2] + (c == 3 ? a.m[12 + i] : T(0));
    r.m[3] = r.m[7] = r.m[11] = 0;
    r.m[15] = 1;
    return r;
}

}

#endif
