// core/smath/include/detail/mat2.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_MAT2_H
#define CORE_SMATH_INCLUDE_DETAIL_MAT2_H

#include "vec4.h"

namespace math {

// Column-major: element (row r, column c) is m[c * 2 + r].
template <class T>
struct mat2 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T m[4];

    mat2() { setIdentity(); }
    explicit mat2(ZUninitialized) {}
    explicit mat2(T d) { m[0] = m[3] = d; m[1] = m[2] = 0; }
    explicit mat2(const T* p) { set(p); }
    explicit mat2(const mat3<T>& a);
    explicit mat2(const mat4<T>& a);

    mat2& set(const T* p) { for (int i = 0; i < 4; ++i) m[i] = p[i]; return *this; }
    mat2& setIdentity() { m[0] = m[3] = 1; m[1] = m[2] = 0; return *this; }
    bool invert()
    {
        T d = m[0] * m[3] - m[1] * m[2];
        if (std::fabs(d) <= T(1.1920928955078125e-07))
            return false;
        T i = T(1) / d, a = m[0];
        m[0] = m[3] * i;
        m[3] = a * i;
        m[1] = -m[1] * i;
        m[2] = -m[2] * i;
        return true;
    }
    mat2& transpose() { T t = m[1]; m[1] = m[2]; m[2] = t; return *this; }
    vec2<T> getRow(unsigned r) const { return vec2<T>(m[r], m[2 + r]); }
    mat2& setRow(unsigned r, const vec2<T>& v) { m[r] = v.x; m[2 + r] = v.y; return *this; }
    vec2<T> getColumn(unsigned c) const { return vec2<T>(m[c * 2], m[c * 2 + 1]); }
    mat2& setColumn(unsigned c, const vec2<T>& v) { m[c * 2] = v.x; m[c * 2 + 1] = v.y; return *this; }
    vec2<T> getAxisX() const { return getColumn(0); }
    mat2& setAxisX(const vec2<T>& v) { return setColumn(0, v); }
    vec2<T> getAxisY() const { return getColumn(1); }
    mat2& setAxisY(const vec2<T>& v) { return setColumn(1, v); }
    vec2<T> getScale() const { return vec2<T>(getAxisX().getLength(), getAxisY().getLength()); }
    mat2& setScale(const vec2<T>& s)
    {
        vec2<T> x = getAxisX(), y = getAxisY();
        setAxisX(x.normalize() * s.x);
        return setAxisY(y.normalize() * s.y);
    }

    operator T*() { return m; }
    operator const T*() const { return m; }
    const T& operator()(unsigned r, unsigned c) const { return m[c * 2 + r]; }
    T& operator()(unsigned r, unsigned c) { return m[c * 2 + r]; }
    bool operator==(const mat2& o) const { for (int i = 0; i < 4; ++i) if (m[i] != o.m[i]) return false; return true; }
    bool operator!=(const mat2& o) const { return !(*this == o); }
    mat2 operator*(const mat2& b) const
    {
        mat2 r(uninitialized);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 2; ++i)
                r.m[c * 2 + i] = m[i] * b.m[c * 2] + m[2 + i] * b.m[c * 2 + 1];
        return r;
    }
};

template <class T>
ZUninitialized mat2<T>::uninitialized;

typedef mat2<float> mat2f;

}

#endif
