// core/smath/include/detail/vec2.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_VEC2_H
#define CORE_SMATH_INCLUDE_DETAIL_VEC2_H

#include "standard_types.h"

namespace math {

template <class T>
struct vec2 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T x, y;

    vec2() : x(0), y(0) {}
    explicit vec2(ZUninitialized) {}
    vec2(T x_, T y_) : x(x_), y(y_) {}
    explicit vec2(T s) : x(s), y(s) {}
    template <class U>
    explicit vec2(const vec2<U>& v) : x(T(v.x)), y(T(v.y)) {}
    explicit vec2(const vec3<T>& v);
    explicit vec2(const vec4<T>& v);

    vec2& set(T x_, T y_) { x = x_; y = y_; return *this; }
    T getLengthSq() const { return x * x + y * y; }
    T getLength() const { return T(std::sqrt(getLengthSq())); }
    void setLength(T l) { normalize(); *this *= l; }
    // Leaves vectors shorter than FLT_EPSILON unchanged.
    vec2& normalize()
    {
        T l = getLength();
        if (std::fabs(l) > T(1.1920928955078125e-07)) {
            T inv = T(1) / l;
            x *= inv;
            y *= inv;
        }
        return *this;
    }

    const T& operator[](unsigned i) const { return (&x)[i]; }
    T& operator[](unsigned i) { return (&x)[i]; }
    bool operator==(const vec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const vec2& v) const { return !(*this == v); }
    vec2 operator-() const { return vec2(-x, -y); }
    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(const vec2& v) const { return vec2(x * v.x, y * v.y); }
    vec2 operator/(const vec2& v) const { return vec2(x / v.x, y / v.y); }
    vec2 operator+(T s) const { return vec2(x + s, y + s); }
    vec2 operator-(T s) const { return vec2(x - s, y - s); }
    vec2 operator*(T s) const { return vec2(x * s, y * s); }
    vec2 operator/(T s) const { return vec2(x / s, y / s); }
    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    vec2& operator*=(const vec2& v) { x *= v.x; y *= v.y; return *this; }
    vec2& operator/=(const vec2& v) { x /= v.x; y /= v.y; return *this; }
    vec2& operator+=(T s) { x += s; y += s; return *this; }
    vec2& operator-=(T s) { x -= s; y -= s; return *this; }
    vec2& operator*=(T s) { x *= s; y *= s; return *this; }
    vec2& operator/=(T s) { x /= s; y /= s; return *this; }
};

template <class T>
ZUninitialized vec2<T>::uninitialized;

template <class T>
vec2<T> operator*(T s, const vec2<T>& v) { return v * s; }

typedef vec2<float> vec2f;
typedef vec2<int> vec2i;
typedef vec2<unsigned> vec2u;

}

#endif
