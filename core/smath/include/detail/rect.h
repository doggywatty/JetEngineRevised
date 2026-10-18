// core/smath/include/detail/rect.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_RECT_H
#define CORE_SMATH_INCLUDE_DETAIL_RECT_H

#include "vec2.h"

namespace math {

template <class T>
struct rect {
    vec2<T> upperLeftCorner, lowerRightCorner;

    rect() {}
    rect(T x0, T y0, T x1, T y1) : upperLeftCorner(x0, y0), lowerRightCorner(x1, y1) {}
    rect(const vec2<T>& ul, const vec2<T>& lr) : upperLeftCorner(ul), lowerRightCorner(lr) {}

    void set(const vec2<T>& ul, const vec2<T>& lr) { upperLeftCorner = ul; lowerRightCorner = lr; }
    void set(T x0, T y0, T x1, T y1) { upperLeftCorner.set(x0, y0); lowerRightCorner.set(x1, y1); }
    T getWidth() const { return lowerRightCorner.x - upperLeftCorner.x; }
    T getHeight() const { return lowerRightCorner.y - upperLeftCorner.y; }
    T getArea() const { return getWidth() * getHeight(); }
    vec2<T> getCenter() const { return vec2<T>((upperLeftCorner.x + lowerRightCorner.x) / 2, (upperLeftCorner.y + lowerRightCorner.y) / 2); }
    vec2<T> getSize() const { return vec2<T>(getWidth(), getHeight()); }
    vec2<T> getExtent() const { return getSize(); }
    void scale(T s) { upperLeftCorner *= s; lowerRightCorner *= s; }
    void repair()
    {
        if (lowerRightCorner.x < upperLeftCorner.x) { T t = lowerRightCorner.x; lowerRightCorner.x = upperLeftCorner.x; upperLeftCorner.x = t; }
        if (lowerRightCorner.y < upperLeftCorner.y) { T t = lowerRightCorner.y; lowerRightCorner.y = upperLeftCorner.y; upperLeftCorner.y = t; }
    }
    bool isValid() const { return lowerRightCorner.x >= upperLeftCorner.x && lowerRightCorner.y >= upperLeftCorner.y; }
    bool isPointInside(const vec2<T>& p) const
    {
        return upperLeftCorner.x <= p.x && upperLeftCorner.y <= p.y && p.x <= lowerRightCorner.x && p.y <= lowerRightCorner.y;
    }
    bool isRectCollided(const rect& o) const
    {
        return o.upperLeftCorner.y < lowerRightCorner.y && upperLeftCorner.y < o.lowerRightCorner.y && o.upperLeftCorner.x < lowerRightCorner.x &&
               upperLeftCorner.x < o.lowerRightCorner.x;
    }
    void clipAgainst(const rect& o)
    {
        if (o.lowerRightCorner.x < lowerRightCorner.x) lowerRightCorner.x = o.lowerRightCorner.x;
        if (o.lowerRightCorner.y < lowerRightCorner.y) lowerRightCorner.y = o.lowerRightCorner.y;
        if (upperLeftCorner.x < o.upperLeftCorner.x) upperLeftCorner.x = o.upperLeftCorner.x;
        if (upperLeftCorner.y < o.upperLeftCorner.y) upperLeftCorner.y = o.upperLeftCorner.y;
        if (lowerRightCorner.y < upperLeftCorner.y) upperLeftCorner.y = lowerRightCorner.y;
        if (lowerRightCorner.x < upperLeftCorner.x) upperLeftCorner.x = lowerRightCorner.x;
    }
    // Moves (not resizes) the rect inside o; false when it does not fit.
    bool constrainTo(const rect& o)
    {
        if (o.getWidth() < getWidth() || o.getHeight() < getHeight())
            return false;
        vec2<T> d;
        if (lowerRightCorner.x > o.lowerRightCorner.x) d.x = o.lowerRightCorner.x - lowerRightCorner.x;
        if (upperLeftCorner.x < o.upperLeftCorner.x) d.x = o.upperLeftCorner.x - upperLeftCorner.x;
        if (lowerRightCorner.y > o.lowerRightCorner.y) d.y = o.lowerRightCorner.y - lowerRightCorner.y;
        if (upperLeftCorner.y < o.upperLeftCorner.y) d.y = o.upperLeftCorner.y - upperLeftCorner.y;
        upperLeftCorner += d;
        lowerRightCorner += d;
        return true;
    }
    vec2<T> getClosestPoint(const vec2<T>& p) const
    {
        vec2<T> r(p);
        r.x = r.x < upperLeftCorner.x ? upperLeftCorner.x : r.x > lowerRightCorner.x ? lowerRightCorner.x : r.x;
        r.y = r.y < upperLeftCorner.y ? upperLeftCorner.y : r.y > lowerRightCorner.y ? lowerRightCorner.y : r.y;
        return r;
    }
    void grow(const vec2<T>& d) { upperLeftCorner -= d; lowerRightCorner += d; }
    void grow(const T& d) { grow(vec2<T>(d, d)); }
    void clampToMin(const vec2<T>& m)
    {
        if (upperLeftCorner.x < m.x) upperLeftCorner.x = m.x;
        if (upperLeftCorner.y < m.y) upperLeftCorner.y = m.y;
        if (lowerRightCorner.x < m.x) lowerRightCorner.x = m.x;
        if (lowerRightCorner.y < m.y) lowerRightCorner.y = m.y;
    }
    void clampToMin(const T& m) { clampToMin(vec2<T>(m, m)); }
    void clampToMax(const vec2<T>& m)
    {
        if (upperLeftCorner.x > m.x) upperLeftCorner.x = m.x;
        if (upperLeftCorner.y > m.y) upperLeftCorner.y = m.y;
        if (lowerRightCorner.x > m.x) lowerRightCorner.x = m.x;
        if (lowerRightCorner.y > m.y) lowerRightCorner.y = m.y;
    }
    void clampToMax(const T& m) { clampToMax(vec2<T>(m, m)); }
    void addInternalPoint(T x, T y) { addInternalPoint(vec2<T>(x, y)); }
    void addInternalPoint(const vec2<T>& p)
    {
        lowerRightCorner.y = p.y <= lowerRightCorner.y ? lowerRightCorner.y : p.y;
        lowerRightCorner.x = lowerRightCorner.x < p.x ? p.x : lowerRightCorner.x;
        upperLeftCorner.y = upperLeftCorner.y <= p.y ? upperLeftCorner.y : p.y;
        upperLeftCorner.x = upperLeftCorner.x <= p.x ? upperLeftCorner.x : p.x;
    }
    bool operator==(const rect& o) const { return upperLeftCorner == o.upperLeftCorner && lowerRightCorner == o.lowerRightCorner; }
    bool operator!=(const rect& o) const { return !(*this == o); }
};

typedef rect<float> rectf;
typedef rect<int> recti;

}

#endif
