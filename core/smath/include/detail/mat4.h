// core/smath/include/detail/mat4.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_MAT4_H
#define CORE_SMATH_INCLUDE_DETAIL_MAT4_H

#include <cstring>
#include "mat3.h"

namespace math {

// Column-major (OpenGL): element (row r, column c) is m[c * 4 + r]; translation is m[12..14].
template <class T>
struct mat4 {
    typedef ZUninitialized ZUninitialized;
    static ZUninitialized uninitialized;

    T m[16];

    mat4() { setIdentity(); }
    explicit mat4(ZUninitialized) {}
    explicit mat4(T d) { for (int i = 0; i < 16; ++i) m[i] = 0; m[0] = m[5] = m[10] = m[15] = d; }
    explicit mat4(const T* p) { set(p); }
    explicit mat4(const mat2<T>& a)
    {
        setIdentity();
        m[0] = a.m[0]; m[1] = a.m[1]; m[4] = a.m[2]; m[5] = a.m[3];
    }
    explicit mat4(const mat3<T>& a) { setIdentity(); setRotationPart(a); }

    mat4& operator=(const mat3<T>& a) { setIdentity(); return setRotationPart(a); }

    mat4& set(const T* p) { for (int i = 0; i < 16; ++i) m[i] = p[i]; return *this; }
    mat4& setIdentity()
    {
        for (int i = 0; i < 16; ++i)
            m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = 1;
        return *this;
    }
    mat4& setRotationIdentity()
    {
        m[0] = m[5] = m[10] = 1;
        m[1] = m[2] = m[4] = m[6] = m[8] = m[9] = 0;
        return *this;
    }
    mat4& setRotationPartIdentity() { return setRotationIdentity(); }
    bool invert();
    mat4& transpose()
    {
        for (int r = 0; r < 4; ++r)
            for (int c = r + 1; c < 4; ++c) {
                T t = m[c * 4 + r];
                m[c * 4 + r] = m[r * 4 + c];
                m[r * 4 + c] = t;
            }
        return *this;
    }
    vec4<T> getRow(unsigned r) const { return vec4<T>(m[r], m[4 + r], m[8 + r], m[12 + r]); }
    mat4& setRow(unsigned r, const vec4<T>& v) { m[r] = v.x; m[4 + r] = v.y; m[8 + r] = v.z; m[12 + r] = v.w; return *this; }
    vec4<T> getColumn(unsigned c) const { return vec4<T>(m[c * 4], m[c * 4 + 1], m[c * 4 + 2], m[c * 4 + 3]); }
    mat4& setColumn(unsigned c, const vec4<T>& v) { m[c * 4] = v.x; m[c * 4 + 1] = v.y; m[c * 4 + 2] = v.z; m[c * 4 + 3] = v.w; return *this; }
    vec3<T> getAxisX() const { return vec3<T>(m[0], m[1], m[2]); }
    mat4& setAxisX(const vec3<T>& v) { m[0] = v.x; m[1] = v.y; m[2] = v.z; return *this; }
    vec3<T> getAxisY() const { return vec3<T>(m[4], m[5], m[6]); }
    mat4& setAxisY(const vec3<T>& v) { m[4] = v.x; m[5] = v.y; m[6] = v.z; return *this; }
    vec3<T> getAxisZ() const { return vec3<T>(m[8], m[9], m[10]); }
    mat4& setAxisZ(const vec3<T>& v) { m[8] = v.x; m[9] = v.y; m[10] = v.z; return *this; }
    const vec3<T>& getTranslation() const { return *reinterpret_cast<const vec3<T>*>(m + 12); }
    mat4& setTranslation(const vec3<T>& v) { m[12] = v.x; m[13] = v.y; m[14] = v.z; return *this; }
    vec3<T> getScale() const { return vec3<T>(getAxisX().getLength(), getAxisY().getLength(), getAxisZ().getLength()); }
    mat4& setScale(const vec3<T>& s)
    {
        vec3<T> x = getAxisX(), y = getAxisY(), z = getAxisZ();
        setAxisX(x.normalize() * s.x);
        setAxisY(y.normalize() * s.y);
        return setAxisZ(z.normalize() * s.z);
    }
    mat3<T> getRotation() const
    {
        vec3<T> x = getAxisX(), y = getAxisY(), z = getAxisZ();
        x.normalize();
        y.normalize();
        z.normalize();
        mat3<T> r(mat3<T>::uninitialized);
        r.setColumn(0, x);
        r.setColumn(1, y);
        r.setColumn(2, z);
        return r;
    }
    mat4& setRotation(const mat3<T>& a)
    {
        vec3<T> s = getScale();
        setRotationPart(a);
        return postScale(s);
    }
    mat3<T> getRotationPart() const
    {
        mat3<T> r(mat3<T>::uninitialized);
        r.m[0] = m[0]; r.m[1] = m[1]; r.m[2] = m[2];
        r.m[3] = m[4]; r.m[4] = m[5]; r.m[5] = m[6];
        r.m[6] = m[8]; r.m[7] = m[9]; r.m[8] = m[10];
        return r;
    }
    mat4& setRotationPart(const mat3<T>& a)
    {
        m[0] = a.m[0]; m[1] = a.m[1]; m[2] = a.m[2];
        m[4] = a.m[3]; m[5] = a.m[4]; m[6] = a.m[5];
        m[8] = a.m[6]; m[9] = a.m[7]; m[10] = a.m[8];
        return *this;
    }
    mat4& postScale(const vec3<T>& s)
    {
        m[0] *= s.x; m[1] *= s.x; m[2] *= s.x;
        m[4] *= s.y; m[5] *= s.y; m[6] *= s.y;
        m[8] *= s.z; m[9] *= s.z; m[10] *= s.z;
        return *this;
    }
    mat4& postTranslate(const vec3<T>& t)
    {
        m[12] += t.x * m[0] + t.y * m[4] + t.z * m[8];
        m[13] += t.x * m[1] + t.y * m[5] + t.z * m[9];
        m[14] += t.x * m[2] + t.y * m[6] + t.z * m[10];
        return *this;
    }
    T dotColumn(unsigned c, const vec3<T>& v) const { return m[c * 4] * v.x + m[c * 4 + 1] * v.y + m[c * 4 + 2] * v.z; }

    operator T*() { return m; }
    operator const T*() const { return m; }
    const T& operator()(unsigned r, unsigned c) const { return m[c * 4 + r]; }
    T& operator()(unsigned r, unsigned c) { return m[c * 4 + r]; }
    bool operator==(const mat4& o) const { return memcmp(m, o.m, sizeof(m)) == 0; }
    bool operator!=(const mat4& o) const { return memcmp(m, o.m, sizeof(m)) != 0; }
    mat4 operator*(const mat4& b) const
    {
        mat4 r(uninitialized);
        for (int c = 0; c < 4; ++c)
            for (int i = 0; i < 4; ++i)
                r.m[c * 4 + i] = m[i] * b.m[c * 4] + m[4 + i] * b.m[c * 4 + 1] + m[8 + i] * b.m[c * 4 + 2] + m[12 + i] * b.m[c * 4 + 3];
        return r;
    }
    mat4& operator*=(const mat4& b) { return *this = *this * b; }
    mat4 operator*(T s) const { mat4 r(*this); return r *= s; }
    mat4& operator*=(T s) { for (int i = 0; i < 16; ++i) m[i] *= s; return *this; }
    mat4 operator+(const mat4& b) const { mat4 r(*this); return r += b; }
    mat4 operator-(const mat4& b) const { mat4 r(*this); return r -= b; }
    mat4& operator+=(const mat4& b) { for (int i = 0; i < 16; ++i) m[i] += b.m[i]; return *this; }
    mat4& operator-=(const mat4& b) { for (int i = 0; i < 16; ++i) m[i] -= b.m[i]; return *this; }
};

template <class T>
ZUninitialized mat4<T>::uninitialized;

// Determinant and cofactors in double precision (Irrlicht's CMatrix4::getInverse).
template <class T>
bool mat4<T>::invert()
{
    const T* a = m;
    double d = (double(a[0]) * a[5] - double(a[1]) * a[4]) * (double(a[10]) * a[15] - double(a[11]) * a[14]) -
               (double(a[0]) * a[6] - double(a[2]) * a[4]) * (double(a[9]) * a[15] - double(a[11]) * a[13]) +
               (double(a[0]) * a[7] - double(a[3]) * a[4]) * (double(a[9]) * a[14] - double(a[10]) * a[13]) +
               (double(a[1]) * a[6] - double(a[2]) * a[5]) * (double(a[8]) * a[15] - double(a[11]) * a[12]) -
               (double(a[1]) * a[7] - double(a[3]) * a[5]) * (double(a[8]) * a[14] - double(a[10]) * a[12]) +
               (double(a[2]) * a[7] - double(a[3]) * a[6]) * (double(a[8]) * a[13] - double(a[9]) * a[12]);
    if (std::fabs(d) <= 2.220446049250313e-16)
        return false;
    d = 1.0 / d;
    double o[16];
    o[0] = d * (a[5] * (double(a[10]) * a[15] - double(a[11]) * a[14]) + a[6] * (double(a[11]) * a[13] - double(a[9]) * a[15]) +
                a[7] * (double(a[9]) * a[14] - double(a[10]) * a[13]));
    o[1] = d * (a[9] * (double(a[2]) * a[15] - double(a[3]) * a[14]) + a[10] * (double(a[3]) * a[13] - double(a[1]) * a[15]) +
                a[11] * (double(a[1]) * a[14] - double(a[2]) * a[13]));
    o[2] = d * (a[13] * (double(a[2]) * a[7] - double(a[3]) * a[6]) + a[14] * (double(a[3]) * a[5] - double(a[1]) * a[7]) +
                a[15] * (double(a[1]) * a[6] - double(a[2]) * a[5]));
    o[3] = d * (a[1] * (double(a[7]) * a[10] - double(a[6]) * a[11]) + a[2] * (double(a[5]) * a[11] - double(a[7]) * a[9]) +
                a[3] * (double(a[6]) * a[9] - double(a[5]) * a[10]));
    o[4] = d * (a[6] * (double(a[8]) * a[15] - double(a[11]) * a[12]) + a[7] * (double(a[10]) * a[12] - double(a[8]) * a[14]) +
                a[4] * (double(a[11]) * a[14] - double(a[10]) * a[15]));
    o[5] = d * (a[10] * (double(a[0]) * a[15] - double(a[3]) * a[12]) + a[11] * (double(a[2]) * a[12] - double(a[0]) * a[14]) +
                a[8] * (double(a[3]) * a[14] - double(a[2]) * a[15]));
    o[6] = d * (a[14] * (double(a[0]) * a[7] - double(a[3]) * a[4]) + a[15] * (double(a[2]) * a[4] - double(a[0]) * a[6]) +
                a[12] * (double(a[3]) * a[6] - double(a[2]) * a[7]));
    o[7] = d * (a[2] * (double(a[7]) * a[8] - double(a[4]) * a[11]) + a[3] * (double(a[4]) * a[10] - double(a[6]) * a[8]) +
                a[0] * (double(a[6]) * a[11] - double(a[7]) * a[10]));
    o[8] = d * (a[7] * (double(a[8]) * a[13] - double(a[9]) * a[12]) + a[4] * (double(a[9]) * a[15] - double(a[11]) * a[13]) +
                a[5] * (double(a[11]) * a[12] - double(a[8]) * a[15]));
    o[9] = d * (a[11] * (double(a[0]) * a[13] - double(a[1]) * a[12]) + a[8] * (double(a[1]) * a[15] - double(a[3]) * a[13]) +
                a[9] * (double(a[3]) * a[12] - double(a[0]) * a[15]));
    o[10] = d * (a[15] * (double(a[0]) * a[5] - double(a[1]) * a[4]) + a[12] * (double(a[1]) * a[7] - double(a[3]) * a[5]) +
                 a[13] * (double(a[3]) * a[4] - double(a[0]) * a[7]));
    o[11] = d * (a[3] * (double(a[5]) * a[8] - double(a[4]) * a[9]) + a[0] * (double(a[7]) * a[9] - double(a[5]) * a[11]) +
                 a[1] * (double(a[4]) * a[11] - double(a[7]) * a[8]));
    o[12] = d * (a[4] * (double(a[10]) * a[13] - double(a[9]) * a[14]) + a[5] * (double(a[8]) * a[14] - double(a[10]) * a[12]) +
                 a[6] * (double(a[9]) * a[12] - double(a[8]) * a[13]));
    o[13] = d * (a[8] * (double(a[2]) * a[13] - double(a[1]) * a[14]) + a[9] * (double(a[0]) * a[14] - double(a[2]) * a[12]) +
                 a[10] * (double(a[1]) * a[12] - double(a[0]) * a[13]));
    o[14] = d * (a[12] * (double(a[2]) * a[5] - double(a[1]) * a[6]) + a[13] * (double(a[0]) * a[6] - double(a[2]) * a[4]) +
                 a[14] * (double(a[1]) * a[4] - double(a[0]) * a[5]));
    o[15] = d * (a[0] * (double(a[5]) * a[10] - double(a[6]) * a[9]) + a[1] * (double(a[6]) * a[8] - double(a[4]) * a[10]) +
                 a[2] * (double(a[4]) * a[9] - double(a[5]) * a[8]));
    for (int i = 0; i < 16; ++i)
        m[i] = T(o[i]);
    return true;
}

template <class T>
mat2<T>::mat2(const mat4<T>& a) { m[0] = a.m[0]; m[1] = a.m[1]; m[2] = a.m[4]; m[3] = a.m[5]; }

template <class T>
mat3<T>::mat3(const mat4<T>& a) { *this = a.getRotationPart(); }

typedef mat4<float> mat4f;

}

#endif
