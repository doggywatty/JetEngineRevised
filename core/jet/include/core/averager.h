// core/jet/include/core/averager.h
#ifndef CORE_JET_INCLUDE_CORE_AVERAGER_H
#define CORE_JET_INCLUDE_CORE_AVERAGER_H

#include <vector>

namespace jet {
namespace core {

// Running mean over the last N samples (ring buffer once full).
template <class T>
class Averager {
public:
    explicit Averager(unsigned size = 10) : m_index(0), m_size(size) { m_samplesList.reserve(size); }

    void Reset() { m_samplesList.clear(); m_index = 0; }
    void Resize(unsigned n) { Reset(); m_size = n; m_samplesList.reserve(n); }
    void Push(const T& v)
    {
        if (m_samplesList.size() < m_size) {
            m_samplesList.push_back(v);
        } else {
            m_samplesList[m_index] = v;
            m_index = (m_index + 1) % unsigned(m_samplesList.size());
        }
    }
    T GetMean() const
    {
        T sum = T();
        if (m_samplesList.empty())
            return sum;
        for (const T& v : m_samplesList)
            sum = sum + v;
        return sum * (1.0f / float(m_samplesList.size()));
    }
    T GetMax() const
    {
        T m = m_samplesList.empty() ? T() : m_samplesList[0];
        for (const T& v : m_samplesList)
            if (m < v) m = v;
        return m;
    }
    T GetMin() const
    {
        T m = m_samplesList.empty() ? T() : m_samplesList[0];
        for (const T& v : m_samplesList)
            if (v < m) m = v;
        return m;
    }
    void GetMinMaxMean(T& mn, T& mx, T& mean) const { mn = GetMin(); mx = GetMax(); mean = GetMean(); }
    unsigned GetSamples() const { return unsigned(m_samplesList.size()); }
    const T& operator[](unsigned i) const { return m_samplesList[i]; }

private:
    std::vector<T> m_samplesList;
    unsigned m_index;
    unsigned m_size;
};

}
}

#endif
