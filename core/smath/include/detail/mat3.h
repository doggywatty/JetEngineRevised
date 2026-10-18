// core/smath/include/detail/mat3.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_MAT3_H
#define CORE_SMATH_INCLUDE_DETAIL_MAT3_H

#include "mat2.h"

namespace math {

// Column-major 3x3; as a 2D transform the translation is m[6], m[7].
template <class T>
struct mat3 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T m[9];

    mat3() { setIdentity(); }
    explicit mat3(ZUninitialized) {}
    explicit mat3(T d) { for (int i = 0; i < 9; ++i) m[i] = 0; m[0] = m[4] = m[8] = d; }
    explicit mat3(const T* p) { set(p); }
    explicit mat3(const mat2<T>& a)
    {
        setIdentity();
        m[0] = a.m[0]; m[1] = a.m[1]; m[3] = a.m[2]; m[4] = a.m[3];
    }
    explicit mat3(const mat4<T>& a);

    mat3& set(const T* p) { for (int i = 0; i < 9; ++i) m[i] = p[i]; return *this; }
    mat3& setIdentity()
    {
        m[0] = m[4] = m[8] = 1;
        m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0;
        return *this;
    }
    mat3& setRotationIdentity() { m[0] = m[4] = 1; m[1] = m[3] = 0; return *this; }
    mat3& setRotationPartIdentity() { return setRotationIdentity(); }
    bool invert()
    {
        T c0 = m[4] * m[8] - m[5] * m[7], c1 = m[5] * m[6] - m[3] * m[8], c2 = m[3] * m[7] - m[4] * m[6];
        T d = m[0] * c0 + m[1] * c1 + m[2] * c2;
        if (std::fabs(d) <= T(1.1920928955078125e-07))
            return false;
        T i = T(1) / d;
        mat3 r(uninitialized);
        r.m[0] = c0 * i;
        r.m[1] = (m[2] * m[7] - m[1] * m[8]) * i;
        r.m[2] = (m[1] * m[5] - m[2] * m[4]) * i;
        r.m[3] = c1 * i;
        r.m[4] = (m[0] * m[8] - m[2] * m[6]) * i;
        r.m[5] = (m[2] * m[3] - m[0] * m[5]) * i;
        r.m[6] = c2 * i;
        r.m[7] = (m[1] * m[6] - m[0] * m[7]) * i;
        r.m[8] = (m[0] * m[4] - m[1] * m[3]) * i;
        *this = r;
        return true;
    }
    mat3& transpose()
    {
        T t;
        t = m[1]; m[1] = m[3]; m[3] = t;
        t = m[2]; m[2] = m[6]; m[6] = t;
        t = m[5]; m[5] = m[7]; m[7] = t;
        return *this;
    }
    vec3<T> getRow(unsigned r) const { return vec3<T>(m[r], m[3 + r], m[6 + r]); }
    void setRow(unsigned r, const vec3<T>& v) { m[r] = v.x; m[3 + r] = v.y; m[6 + r] = v.z; }
    vec3<T> getColumn(unsigned c) const { return vec3<T>(m[c * 3], m[c * 3 + 1], m[c * 3 + 2]); }
    mat3& setColumn(unsigned c, const vec3<T>& v) { m[c * 3] = v.x; m[c * 3 + 1] = v.y; m[c * 3 + 2] = v.z; return *this; }
    vec2<T> getAxisX() const { return vec2<T>(m[0], m[1]); }
    mat3& setAxisX(const vec2<T>& v) { m[0] = v.x; m[1] = v.y; return *this; }
    vec2<T> getAxisY() const { return vec2<T>(m[3], m[4]); }
    mat3& setAxisY(const vec2<T>& v) { m[3] = v.x; m[4] = v.y; return *this; }
    const vec2<T>& getTranslation() const { return *reinterpret_cast<const vec2<T>*>(m + 6); }
    mat3& setTranslation(const vec2<T>& v) { m[6] = v.x; m[7] = v.y; return *this; }
    T getRotation() const { return T(std::atan2(m[1], m[0])); }
    mat3& setRotation(T a)
    {
        vec2<T> s = getScale();
        setRotationPart(a);
        return postScale(s);
    }
    T getRotationPart() const { return getRotation(); }
    mat3& setRotationPart(T a)
    {
        T c = T(std::cos(a)), s = T(std::sin(a));
        m[0] = c;
        m[4] = c;
        m[1] = s;
        m[3] = -s;
        return *this;
    }
    vec2<T> getScale() const { return vec2<T>(getAxisX().getLength(), getAxisY().getLength()); }
    mat3& setScale(const vec2<T>& s)
    {
        vec2<T> x = getAxisX(), y = getAxisY();
        setAxisX(x.normalize() * s.x);
        return setAxisY(y.normalize() * s.y);
    }
    mat3& postScale(const vec2<T>& s)
    {
        m[0] *= s.x;
        m[1] *= s.x;
        m[3] *= s.y;
        m[4] *= s.y;
        return *this;
    }
    mat3& postTranslate(const vec2<T>& t)
    {
        m[6] = m[6] + t.y * m[3] + t.x * m[0];
        m[7] = m[7] + t.y * m[4] + t.x * m[1];
        return *this;
    }

    operator T*() { return m; }
    operator const T*() const { return m; }
    const T& operator()(unsigned r, unsigned c) const { return m[c * 3 + r]; }
    T& operator()(unsigned r, unsigned c) { return m[c * 3 + r]; }
    bool operator==(const mat3& o) const { for (int i = 0; i < 9; ++i) if (m[i] != o.m[i]) return false; return true; }
    bool operator!=(const mat3& o) const { return !(*this == o); }
    mat3 operator*(const mat3& b) const
    {
        mat3 r(uninitialized);
        for (int c = 0; c < 3; ++c)
            for (int i = 0; i < 3; ++i)
                r.m[c * 3 + i] = m[i] * b.m[c * 3] + m[3 + i] * b.m[c * 3 + 1] + m[6 + i] * b.m[c * 3 + 2];
        return r;
    }
    mat3& operator*=(const mat3& b) { return *this = *this * b; }
};

template <class T>
ZUninitialized mat3<T>::uninitialized;

template <class T>
mat2<T>::mat2(const mat3<T>& a) { m[0] = a.m[0]; m[1] = a.m[1]; m[2] = a.m[3]; m[3] = a.m[4]; }

typedef mat3<float> mat3f;

}

#endif
