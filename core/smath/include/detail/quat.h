// core/smath/include/detail/quat.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_QUAT_H
#define CORE_SMATH_INCLUDE_DETAIL_QUAT_H

#include <type_traits>
#include "mat4.h"

namespace math {

// (x, y, z, w). Composition is reversed relative to the Hamilton product: a * b == hamilton(b, a),
// and getAsMat3() is the transpose of the textbook matrix, so mat(a * b) == mat(a) * mat(b).
template <class T>
struct quat {
    struct ZUninitialized {};
    struct ZAxisX {};
    struct ZAxisY {};
    struct ZAxisZ {};
    struct ZAngleAxis {};
    struct ZEulerXYZ {};
    struct ZEulerXZY {};
    struct ZEulerYXZ {};
    struct ZEulerYZX {};
    struct ZEulerZXY {};
    struct ZEulerZYX {};
    static ZUninitialized uninitialized;
    static ZAxisX axisX;
    static ZAxisY axisY;
    static ZAxisZ axisZ;
    static ZAngleAxis angleAxis;
    static ZEulerXYZ eulerXYZ;
    static ZEulerXZY eulerXZY;
    static ZEulerYXZ eulerYXZ;
    static ZEulerYZX eulerYZX;
    static ZEulerZXY eulerZXY;
    static ZEulerZYX eulerZYX;

    T x, y, z, w;

    quat() : x(0), y(0), z(0), w(1) {}
    explicit quat(ZUninitialized) {}
    quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    explicit quat(const mat3<T>& a) { setFromMat3(a); }
    explicit quat(const mat4<T>& a) { setFromMat4(a); }
    quat(ZAxisX, T a) : x(T(std::sin(a * T(0.5)))), y(0), z(0), w(T(std::cos(a * T(0.5)))) {}
    quat(ZAxisY, T a) : x(0), y(T(std::sin(a * T(0.5)))), z(0), w(T(std::cos(a * T(0.5)))) {}
    quat(ZAxisZ, T a) : x(0), y(0), z(T(std::sin(a * T(0.5)))), w(T(std::cos(a * T(0.5)))) {}
    quat(ZAngleAxis, T a, const vec3<T>& axis) { setFromAngleAxis(a, axis); }
    quat(ZEulerXYZ, const vec3<T>& e) { setFromEulerXYZ(e); }
    quat(ZEulerXZY, const vec3<T>& e) { setFromEulerXZY(e); }
    quat(ZEulerYXZ, const vec3<T>& e) { setFromEulerYXZ(e); }
    quat(ZEulerYZX, const vec3<T>& e) { setFromEulerYZX(e); }
    quat(ZEulerZXY, const vec3<T>& e) { setFromEulerZXY(e); }
    quat(ZEulerZYX, const vec3<T>& e) { setFromEulerZYX(e); }

    quat& set(T x_, T y_, T z_, T w_) { x = x_; y = y_; z = z_; w = w_; return *this; }
    T getLengthSq() const { return x * x + y * y + z * z + w * w; }
    T getLength() const { return T(std::sqrt(getLengthSq())); }
    quat& normalize()
    {
        T i = T(1) / getLength();
        x *= i;
        y *= i;
        z *= i;
        w *= i;
        return *this;
    }
    quat& invert() { x = -x; y = -y; z = -z; return *this; }
    quat& setIdentity() { x = y = z = 0; w = 1; return *this; }

    quat& setFromMat3(const mat3<T>& a) { return fromMatrix(a.m, 3); }
    quat& setFromMat4(const mat4<T>& a) { return fromMatrix(a.m, 4); }
    mat3<T> getAsMat3() const
    {
        T x2 = x + x, y2 = y + y, zz2 = (z + z) * z, zw2 = (z + z) * w;
        mat3<T> r(mat3<T>::uninitialized);
        r.m[0] = (-(y * y2) - zz2) + T(1);
        r.m[3] = zw2 + x2 * y;
        r.m[6] = x2 * z - y2 * w;
        r.m[4] = T(1) - (zz2 + x2 * x);
        r.m[1] = x2 * y - zw2;
        r.m[7] = y2 * z + x2 * w;
        r.m[2] = y2 * w + x2 * z;
        r.m[5] = y2 * z - x2 * w;
        r.m[8] = (-(y * y2) - x2 * x) + T(1);
        return r;
    }
    mat4<T> getAsMat4() const { return mat4<T>(getAsMat3()); }

    quat& setFromAngleAxis(T a, const vec3<T>& axis)
    {
        T s = T(std::sin(a * T(0.5)));
        x = s * axis.x;
        y = s * axis.y;
        z = s * axis.z;
        w = T(std::cos(a * T(0.5)));
        return *this;
    }
    void getAsAngleAxis(T& a, vec3<T>& axis) const
    {
        T s = T(std::sqrt(x * x + y * y + z * z));
        if (s > T(1e-6)) {
            a = T(2) * T(std::acos(w < T(-1) ? T(-1) : w > T(1) ? T(1) : w));
            axis.set(x / s, y / s, z / s);
        } else {
            a = 0;
            axis.set(0, 1, 0);
        }
    }

    // Euler "ABC" is hamilton(qA, qB, qC) of the half-angle axis rotations.
    quat& setFromEulerXYZ(const vec3<T>& e) { return fromEuler(e, 0, 1, 2); }
    quat& setFromEulerXZY(const vec3<T>& e) { return fromEuler(e, 0, 2, 1); }
    quat& setFromEulerYXZ(const vec3<T>& e) { return fromEuler(e, 1, 0, 2); }
    quat& setFromEulerYZX(const vec3<T>& e) { return fromEuler(e, 1, 2, 0); }
    quat& setFromEulerZXY(const vec3<T>& e) { return fromEuler(e, 2, 0, 1); }
    quat& setFromEulerZYX(const vec3<T>& e) { return fromEuler(e, 2, 1, 0); }
    vec3<T> getAsEulerXYZ() const
    {
        vec3<T> r;
        T t = (x * z + y * w) * 2;
        if (t >= T(0.9999)) {
            r.y = T(1.5707963705062866);
            r.x = T(std::atan2((x * y + z * w) * 2, T(1) - (z * z + x * x) * 2));
        } else if (t > T(-0.9999)) {
            r.y = T(std::asin(t));
            r.x = T(std::atan2((y * z - x * w) * T(-2), T(1) - (x * x + y * y) * 2));
            r.z = T(std::atan2((y * x - z * w) * T(-2), T(1) - (z * z + y * y) * 2));
        } else {
            r.y = T(-1.5707963705062866);
            r.x = -T(std::atan2((x * y + z * w) * 2, T(1) - (z * z + x * x) * 2));
        }
        return r;
    }
    vec3<T> getAsEulerXZY() const { return toEuler(0, 2, 1); }
    vec3<T> getAsEulerYXZ() const { return toEuler(1, 0, 2); }
    vec3<T> getAsEulerYZX() const { return toEuler(1, 2, 0); }
    vec3<T> getAsEulerZXY() const { return toEuler(2, 0, 1); }
    vec3<T> getAsEulerZYX() const { return toEuler(2, 1, 0); }

    const T& operator[](unsigned i) const { return (&x)[i]; }
    T& operator[](unsigned i) { return (&x)[i]; }
    bool operator==(const quat& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
    bool operator!=(const quat& q) const { return !(*this == q); }
    quat operator+(const quat& q) const { return quat(x + q.x, y + q.y, z + q.z, w + q.w); }
    quat operator-(const quat& q) const { return quat(x - q.x, y - q.y, z - q.z, w - q.w); }
    quat operator-() const { return quat(-x, -y, -z, -w); }
    quat operator~() const { return quat(-x, -y, -z, w); }
    template <class U, class = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    quat operator*(U s) const { return quat(T(s) * x, T(s) * y, T(s) * z, T(s) * w); }
    quat operator*(const quat& b) const
    {
        return quat((x * b.w + w * b.x + z * b.y) - y * b.z, (y * b.w + w * b.y + x * b.z) - z * b.x,
                    (z * b.w + w * b.z + y * b.x) - x * b.y, ((w * b.w - x * b.x) - y * b.y) - z * b.z);
    }
    quat& operator*=(const quat& b) { return *this = *this * b; }

private:
    static quat hamilton(const quat& p, const quat& q) { return q * p; }
    quat& fromEuler(const vec3<T>& e, int a, int b, int c)
    {
        quat q[3];
        for (int i = 0; i < 3; ++i) {
            T s = T(std::sin(e[i] * T(0.5))), co = T(std::cos(e[i] * T(0.5)));
            q[i].set(i == 0 ? s : 0, i == 1 ? s : 0, i == 2 ? s : 0, co);
        }
        return *this = hamilton(hamilton(q[a], q[b]), q[c]);
    }
    // Inverse of fromEuler, via the rotation matrix.
    vec3<T> toEuler(int a, int b, int c) const
    {
        mat3<T> r = getAsMat3();
        auto R = [&](int i, int j) { return r.m[i * 3 + j]; };  // textbook R(i, j) is the transpose
        T sign = ((b - a + 3) % 3 == 1) ? T(1) : T(-1);
        vec3<T> e;
        T sb = sign * R(a, c);
        sb = sb > T(1) ? T(1) : sb < T(-1) ? T(-1) : sb;
        e[b] = T(std::asin(sb));
        if (std::fabs(sb) < T(0.9999)) {
            e[a] = T(std::atan2(-sign * R(b, c), R(c, c)));
            e[c] = T(std::atan2(-sign * R(a, b), R(a, a)));
        } else {
            e[a] = T(std::atan2(sign * R(c, b), R(b, b)));
            e[c] = 0;
        }
        return e;
    }
    quat& fromMatrix(const T* p, int n)
    {
        auto M = [&](int col, int row) { return p[col * n + row]; };
        T tr = M(1, 1) + M(0, 0) + M(2, 2);
        if (tr <= 0) {
            int i = M(0, 0) < M(1, 1) ? 1 : 0;
            if (M(i, i) < M(2, 2))
                i = 2;
            int j = (i + 1) % 3, k = (j + 1) % 3;
            T s = T(std::sqrt(((M(i, i) + T(1)) - M(j, j)) - M(k, k)));
            T f = T(0.5) / s;
            (&x)[i] = s * T(0.5);
            w = (M(k, j) - M(j, k)) * f;
            (&x)[j] = (M(j, i) + M(i, j)) * f;
            (&x)[k] = (M(k, i) + M(i, k)) * f;
        } else {
            T s = T(std::sqrt(tr + T(1)));
            T f = T(0.5) / s;
            w = s * T(0.5);
            x = f * (M(2, 1) - M(1, 2));
            y = f * (M(0, 2) - M(2, 0));
            z = f * (M(1, 0) - M(0, 1));
        }
        return *this;
    }
};

#define JET_QUAT_TAG(t, n) template <class T> typename quat<T>::t quat<T>::n;
JET_QUAT_TAG(ZUninitialized, uninitialized)
JET_QUAT_TAG(ZAxisX, axisX)
JET_QUAT_TAG(ZAxisY, axisY)
JET_QUAT_TAG(ZAxisZ, axisZ)
JET_QUAT_TAG(ZAngleAxis, angleAxis)
JET_QUAT_TAG(ZEulerXYZ, eulerXYZ)
JET_QUAT_TAG(ZEulerXZY, eulerXZY)
JET_QUAT_TAG(ZEulerYXZ, eulerYXZ)
JET_QUAT_TAG(ZEulerYZX, eulerYZX)
JET_QUAT_TAG(ZEulerZXY, eulerZXY)
JET_QUAT_TAG(ZEulerZYX, eulerZYX)
#undef JET_QUAT_TAG

typedef quat<float> quatf;

}

#endif
