// core/smath/include/detail/func_trig.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_TRIG_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_TRIG_H

#include "constants.h"
#include "vec4.h"

namespace math {

inline float sin(float a) { return std::sin(a); }
inline float cos(float a) { return std::cos(a); }
inline float tan(float a) { return std::tan(a); }
inline float acos(float a) { return std::acos(a < -1.0f ? -1.0f : a > 1.0f ? 1.0f : a); }
inline float asin(float a) { return std::asin(a < -1.0f ? -1.0f : a > 1.0f ? 1.0f : a); }
inline float atan2(float y, float x) { return std::atan2(y, x); }
inline void sincos(float a, float& s, float& c) { s = std::sin(a); c = std::cos(a); }
inline float radians(float d) { return d * DEG_TO_RAD; }
inline float degrees(float r) { return r * RAD_TO_DEG; }

template <class T> vec3<T> sin(const vec3<T>& v) { return vec3<T>(std::sin(v.x), std::sin(v.y), std::sin(v.z)); }
template <class T> vec3<T> cos(const vec3<T>& v) { return vec3<T>(std::cos(v.x), std::cos(v.y), std::cos(v.z)); }
template <class T> void sincos(const vec3<T>& v, vec3<T>& s, vec3<T>& c) { s = sin(v); c = cos(v); }
template <class T> T radians(const T& v) { return v * DEG_TO_RAD; }

}

#endif
