// core/smath/include/detail/func_common.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_COMMON_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_COMMON_H

#include <cstdint>
#include <cstring>
#include "vec4.h"

namespace math {

template <class T> T min(T a, T b) { return b < a ? b : a; }
template <class T> T max(T a, T b) { return a < b ? b : a; }
template <class T> T abs(const T& a) { return a < T(0) ? -a : a; }
template <class T> T sgn(const T& a) { return a > T(0) ? T(1) : a < T(0) ? T(-1) : T(0); }
template <class T> T signOf(T a) { return a < T(0) ? T(-1) : T(1); }
template <class T> T floor(const T& a) { return T(std::floor(a)); }
template <class T> T ceil(const T& a) { return T(std::ceil(a)); }
inline float round(float a) { return std::floor(a + 0.5f); }
inline double round(double a) { return std::floor(a + 0.5); }
template <class T> T sqrt(const T& a) { return T(std::sqrt(a)); }
template <class T> T pow(const T& a, const T& b) { return T(std::pow(a, b)); }
template <class T> T invSqrt(const T& a) { return T(1) / T(std::sqrt(a)); }

// The classic 0x5f3759df estimate with one Newton step.
inline float fastInvSqrt(float a)
{
    uint32_t i;
    memcpy(&i, &a, 4);
    i = 0x5f3759df - (i >> 1);
    float y;
    memcpy(&y, &i, 4);
    return (1.5f - a * 0.5f * y * y) * y;
}
inline float fastSqrt(float a) { return a * fastInvSqrt(a); }

template <class T> vec2<T> min(const vec2<T>& a, const vec2<T>& b) { return vec2<T>(min(a.x, b.x), min(a.y, b.y)); }
template <class T> vec2<T> max(const vec2<T>& a, const vec2<T>& b) { return vec2<T>(max(a.x, b.x), max(a.y, b.y)); }
template <class T> vec3<T> min(const vec3<T>& a, const vec3<T>& b) { return vec3<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
template <class T> vec3<T> max(const vec3<T>& a, const vec3<T>& b) { return vec3<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }
template <class T> vec4<T> min(const vec4<T>& a, const vec4<T>& b)
{
    return vec4<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w));
}
template <class T> vec4<T> max(const vec4<T>& a, const vec4<T>& b)
{
    return vec4<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w));
}

inline bool isPow2(unsigned v) { return v && !(v & (v - 1)); }
inline unsigned getSmallestPow2(unsigned v)
{
    unsigned p = 1;
    while (p < v)
        p <<= 1;
    return p;
}
template <class T> T nextMultiple(const T& v, const T& m) { return (v + m - 1) / m * m; }

// Linear interpolation (1 - t) * a + t * b.
template <class T> struct quat;
template <class T> T lerp(const T& a, const T& b, float t) { return a * (1.0f - t) + b * t; }
inline float lerp(float a, float b, float t) { return (1.0f - t) * a + t * b; }
inline int lerp(int a, int b, float t) { return int(float(a) + float(b - a) * t); }
inline unsigned lerp(unsigned a, unsigned b, float t) { return unsigned(float(a) + (float(b) - float(a)) * t); }
template <class T>
vec3<T> lerp(const vec3<T>& a, const vec3<T>& b, float t) { return vec3<T>(a * (T(1) - t)) + b * t; }

}

#endif
