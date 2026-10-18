// core/smath/include/detail/vec4.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_VEC4_H
#define CORE_SMATH_INCLUDE_DETAIL_VEC4_H

#include "vec3.h"

namespace math {

template <class T>
struct vec4 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T x, y, z, w;

    vec4() : x(0), y(0), z(0), w(0) {}
    explicit vec4(ZUninitialized) {}
    vec4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    explicit vec4(T s) : x(s), y(s), z(s), w(s) {}
    template <class U>
    explicit vec4(const vec4<U>& v) : x(T(v.x)), y(T(v.y)), z(T(v.z)), w(T(v.w)) {}
    explicit vec4(const vec2<T>& v) : x(v.x), y(v.y), z(0), w(0) {}
    vec4(const vec2<T>& v, T z_, T w_) : x(v.x), y(v.y), z(z_), w(w_) {}
    explicit vec4(const vec3<T>& v) : x(v.x), y(v.y), z(v.z), w(0) {}  // w = 0, as shipped
    vec4(const vec3<T>& v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    vec4& operator=(const vec3<T>& v) { x = v.x; y = v.y; z = v.z; w = 0; return *this; }

    vec4& set(T x_, T y_, T z_, T w_) { x = x_; y = y_; z = z_; w = w_; return *this; }
    T getLengthSq() const { return x * x + y * y + z * z + w * w; }
    T getLength() const { return T(std::sqrt(getLengthSq())); }
    void setLength(T l) { normalize(); *this *= l; }
    // Leaves vectors shorter than FLT_EPSILON unchanged.
    vec4& normalize()
    {
        T l = getLength();
        if (std::fabs(l) > T(1.1920928955078125e-07)) {
            T inv = T(1) / l;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
        }
        return *this;
    }

    const T& operator[](unsigned i) const { return (&x)[i]; }
    T& operator[](unsigned i) { return (&x)[i]; }
    bool operator==(const vec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const vec4& v) const { return !(*this == v); }
    vec4 operator-() const { return vec4(-x, -y, -z, -w); }
    vec4 operator+(const vec4& v) const { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    vec4 operator-(const vec4& v) const { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    vec4 operator*(const vec4& v) const { return vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    vec4 operator/(const vec4& v) const { return vec4(x / v.x, y / v.y, z / v.z, w / v.w); }
    vec4 operator+(T s) const { return vec4(x + s, y + s, z + s, w + s); }
    vec4 operator-(T s) const { return vec4(x - s, y - s, z - s, w - s); }
    vec4 operator*(T s) const { return vec4(x * s, y * s, z * s, w * s); }
    vec4 operator/(T s) const { return vec4(x / s, y / s, z / s, w / s); }
    vec4& operator+=(const vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    vec4& operator-=(const vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    vec4& operator*=(const vec4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
    vec4& operator/=(const vec4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
    vec4& operator*=(T s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    vec4& operator/=(T s) { x /= s; y /= s; z /= s; w /= s; return *this; }
};

template <class T>
ZUninitialized vec4<T>::uninitialized;

template <class T>
vec4<T> operator*(T s, const vec4<T>& v) { return v * s; }

template <class T>
vec2<T>::vec2(const vec4<T>& v) : x(v.x), y(v.y) {}

template <class T>
vec3<T>::vec3(const vec4<T>& v) : x(v.x), y(v.y), z(v.z) {}

template <class T>
vec3<T>& vec3<T>::operator=(const vec4<T>& v) { x = v.x; y = v.y; z = v.z; return *this; }

typedef vec4<float> vec4f;

}

#endif
