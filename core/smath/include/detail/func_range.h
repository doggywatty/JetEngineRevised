// core/smath/include/detail/func_range.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_FUNC_RANGE_H
#define CORE_SMATH_INCLUDE_DETAIL_FUNC_RANGE_H

#include <cstdint>

namespace math {

inline int clamp(int v, int lo, int hi) { v = v < lo ? lo : v; return hi <= v ? hi : v; }
inline unsigned clamp(unsigned v, unsigned lo, unsigned hi) { v = v < lo ? lo : v; return hi <= v ? hi : v; }
inline long long clamp(long long v, long long lo, long long hi) { v = v < lo ? lo : v; return hi < v ? hi : v; }
inline float clamp(float v, float lo, float hi) { v = lo <= v ? v : lo; return v <= hi ? v : hi; }
template <class T> T clamp(const T& v, const T& lo, const T& hi) { return v < lo ? lo : hi < v ? hi : v; }

}

#endif
