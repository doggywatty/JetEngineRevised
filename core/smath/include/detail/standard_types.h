// core/smath/include/detail/standard_types.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_STANDARD_TYPES_H
#define CORE_SMATH_INCLUDE_DETAIL_STANDARD_TYPES_H

#include <cmath>
#include <cstdint>

namespace math {

template <class T> struct vec2;
template <class T> struct vec3;
template <class T> struct vec4;
template <class T> struct quat;
template <class T> struct mat2;
template <class T> struct mat3;
template <class T> struct mat4;
template <class T> struct aabb;
template <class T> struct aabb2;
template <class T> struct rect;
template <class T> struct sphere;
template <class T> struct plane;
template <class T> struct line2;
template <class T> struct line3;
template <class T> struct triangle3;

struct ZUninitialized {};

enum RelationEnum { REL_FRONT, REL_BACK, REL_PLANAR, REL_SPANNING, REL_CLIPPED };

}

#endif
