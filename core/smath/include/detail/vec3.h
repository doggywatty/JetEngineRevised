// core/smath/include/detail/vec3.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_VEC3_H
#define CORE_SMATH_INCLUDE_DETAIL_VEC3_H

#include "vec2.h"

namespace math {

template <class T>
struct vec3 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T x, y, z;

    vec3() : x(0), y(0), z(0) {}
    explicit vec3(ZUninitialized) {}
    vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    explicit vec3(T s) : x(s), y(s), z(s) {}
    template <class U>
    explicit vec3(const vec3<U>& v) : x(T(v.x)), y(T(v.y)), z(T(v.z)) {}
    explicit vec3(const vec2<T>& v) : x(v.x), y(v.y), z(0) {}
    vec3(const vec2<T>& v, T z_) : x(v.x), y(v.y), z(z_) {}
    explicit vec3(const vec4<T>& v);

    vec3& operator=(const vec2<T>& v) { x = v.x; y = v.y; z = 0; return *this; }
    vec3& operator=(const vec4<T>& v);

    vec3& set(T x_, T y_, T z_) { x = x_; y = y_; z = z_; return *this; }
    T getLengthSq() const { return x * x + y * y + z * z; }
    T getLength() const { return T(std::sqrt(getLengthSq())); }
    void setLength(T l) { normalize(); *this *= l; }
    // Leaves vectors shorter than FLT_EPSILON unchanged.
    vec3& normalize()
    {
        T l = getLength();
        if (std::fabs(l) > T(1.1920928955078125e-07)) {
            T inv = T(1) / l;
            x *= inv;
            y *= inv;
            z *= inv;
        }
        return *this;
    }

    const T& operator[](unsigned i) const { return (&x)[i]; }
    T& operator[](unsigned i) { return (&x)[i]; }
    bool operator==(const vec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const vec3& v) const { return !(*this == v); }
    vec3 operator-() const { return vec3(-x, -y, -z); }
    vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
    vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
    vec3 operator*(const vec3& v) const { return vec3(x * v.x, y * v.y, z * v.z); }
    vec3 operator/(const vec3& v) const { return vec3(x / v.x, y / v.y, z / v.z); }
    vec3 operator+(T s) const { return vec3(x + s, y + s, z + s); }
    vec3 operator-(T s) const { return vec3(x - s, y - s, z - s); }
    vec3 operator*(T s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(T s) const { return vec3(x / s, y / s, z / s); }
    vec3& operator+=(const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3& operator-=(const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    vec3& operator*=(const vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    vec3& operator/=(const vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    vec3& operator+=(T s) { x += s; y += s; z += s; return *this; }
    vec3& operator-=(T s) { x -= s; y -= s; z -= s; return *this; }
    vec3& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
    vec3& operator/=(T s) { x /= s; y /= s; z /= s; return *this; }
};

template <class T>
ZUninitialized vec3<T>::uninitialized;

template <class T>
vec3<T> operator*(T s, const vec3<T>& v) { return v * s; }

template <class T>
vec2<T>::vec2(const vec3<T>& v) : x(v.x), y(v.y) {}

typedef vec3<float> vec3f;
typedef vec3<int> vec3i;

}

#endif
