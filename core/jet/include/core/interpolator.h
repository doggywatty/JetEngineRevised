// core/jet/include/core/interpolator.h
#ifndef CORE_JET_INCLUDE_CORE_INTERPOLATOR_H
#define CORE_JET_INCLUDE_CORE_INTERPOLATOR_H

#include <algorithm>
#include <vector>
#include "core/interputils.h"

namespace jet {
namespace core {

template <class T>
struct InterpPoint {
    InterpPoint() : value(), time(0), tension(0), bias(0) {}
    InterpPoint(const T& v, int t, float tn = 0, float b = 0) : value(v), time(t), tension(tn), bias(b) {}
    int GetTime() const { return time; }
    const T& GetValue() const { return value; }
    bool operator<(const InterpPoint& o) const { return time < o.time; }
    bool operator<(int t) const { return time < t; }
    bool operator==(const InterpPoint& o) const { return time == o.time && value == o.value; }

    T value;
    int time;
    float tension, bias;
};

struct InterpolationTypeDef {
    enum type { NONE, LINEAR, CUBIC, CATMULL_ROM, SPLINE };  // NONE picks the nearest key
};
typedef safe_enum<InterpolationTypeDef, unsigned char> InterpolationType;

// A time-sorted key path sampled with one of five interpolation types.
template <class T>
class Interpolator {
public:
    Interpolator() : m_interpolationType(InterpolationTypeDef::LINEAR) {}
    Interpolator(const std::vector<InterpPoint<T>>& path, InterpolationType type) : m_path(path), m_interpolationType(type)
    {
        std::stable_sort(m_path.begin(), m_path.end());
    }
    virtual ~Interpolator() {}

    virtual T GetValueAtTime(int time) const
    {
        if (m_path.empty())
            return T();
        if (m_path.size() == 1 || time <= m_path.front().time)
            return m_path.front().value;
        if (time >= m_path.back().time)
            return m_path.back().value;
        auto next = std::lower_bound(m_path.begin(), m_path.end(), time, [](const InterpPoint<T>& p, int t) { return p.time < t; });
        auto prev = next == m_path.begin() ? next : next - 1;
        switch (m_interpolationType.underlying()) {
        case InterpolationTypeDef::type::NONE:
            return time - prev->time < next->time - time ? prev->value : next->value;
        case InterpolationTypeDef::type::LINEAR:
            return math::lerp(prev->value, next->value, float(time - prev->time) / float(next->time - prev->time));
        default: {
            auto before = prev == m_path.begin() ? prev : prev - 1;
            auto after = next + 1 == m_path.end() ? next : next + 1;
            float t = float(time - prev->time) / float(next->time - prev->time);
            if (m_interpolationType == InterpolationTypeDef::CUBIC)
                return InterpolateCubic(before->value, prev->value, next->value, after->value, t);
            if (m_interpolationType == InterpolationTypeDef::CATMULL_ROM)
                return InterpolateCatmullRom(before->value, prev->value, next->value, after->value, t);
            if (m_interpolationType == InterpolationTypeDef::SPLINE)
                return InterpolateSpline(before->value, prev->value, next->value, after->value, t, prev->tension, prev->bias,
                                         float(prev->time - before->time), float(next->time - prev->time), float(after->time - next->time));
            return T();
        }
        }
    }

    void InsertPoint(const InterpPoint<T>& p) { m_path.insert(std::upper_bound(m_path.begin(), m_path.end(), p), p); }
    void RemovePoint(unsigned i) { if (i < m_path.size()) m_path.erase(m_path.begin() + i); }
    void RemovePoint(int time)
    {
        m_path.erase(std::remove_if(m_path.begin(), m_path.end(), [time](const InterpPoint<T>& p) { return p.time == time; }), m_path.end());
    }
    void RemovePoint(const InterpPoint<T>& p) { m_path.erase(std::remove(m_path.begin(), m_path.end(), p), m_path.end()); }
    void SetOrderedPath(const std::vector<InterpPoint<T>>& path) { m_path = path; }
    const std::vector<InterpPoint<T>>& GetPath() const { return m_path; }
    void ClearPath() { m_path.clear(); }
    bool IsEmpty() const { return m_path.empty(); }
    void SetInterpolationType(InterpolationType t) { m_interpolationType = t; }

private:
    std::vector<InterpPoint<T>> m_path;
    InterpolationType m_interpolationType;
};

// Quaternion paths: nearest, slerp, or squad for the curve types.
template <>
inline math::quat<float> Interpolator<math::quat<float>>::GetValueAtTime(int time) const
{
    if (m_path.empty())
        return math::quat<float>(0, 0, 0, 1);
    if (m_path.size() == 1 || time <= m_path.front().time)
        return m_path.front().value;
    if (time >= m_path.back().time)
        return m_path.back().value;
    auto next = std::lower_bound(m_path.begin(), m_path.end(), time, [](const InterpPoint<math::quat<float>>& p, int t) { return p.time < t; });
    auto prev = next == m_path.begin() ? next : next - 1;
    float t = float(time - prev->time) / float(next->time - prev->time);
    if (m_interpolationType == InterpolationTypeDef::NONE)
        return time - prev->time < next->time - time ? prev->value : next->value;
    if (m_interpolationType == InterpolationTypeDef::LINEAR)
        return math::lerp(prev->value, next->value, t);
    auto before = prev == m_path.begin() ? prev : prev - 1;
    auto after = next + 1 == m_path.end() ? next : next + 1;
    return InterpolateSpline(before->value, prev->value, next->value, after->value, t);
}

}
}

#endif
