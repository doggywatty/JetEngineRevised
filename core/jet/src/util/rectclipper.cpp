// core/jet/src/util/rectclipper.cpp
// Sutherland-Hodgman polygon clipping against a rect or an arbitrary convex/concave
// shape's edges. The clip-edge layout (winding, which side is "inside") is inferred, not
// independently verified against the .so, but the algorithm itself produces correct
// clipped output for any primitive given as an interleaved float array (x,y,... per vertex).
#include "util/rectclipper.h"
#include <algorithm>

namespace jet::util {

RectClipper::RectClipper() : m_numberOfOutputPoints(0), m_numberOfClips(0), m_nLines(0) {}

void RectClipper::SetRect(const math::rect<float> &r) {
    const math::vec2<float> &a = r.upperLeftCorner, &b = r.lowerRightCorner;
    m_nLines = 4;
    m_lines[0] = math::line2<float>(a.x, a.y, b.x, a.y);
    m_lines[1] = math::line2<float>(b.x, a.y, b.x, b.y);
    m_lines[2] = math::line2<float>(b.x, b.y, a.x, b.y);
    m_lines[3] = math::line2<float>(a.x, b.y, a.x, a.y);
}

void RectClipper::SetShape(const math::vec2<float> *points, unsigned count, bool close) {
    unsigned n = std::min(count, close ? count : (count > 0 ? count - 1 : 0));
    m_nLines = 0;
    for (unsigned i = 0; i < n && m_nLines < 16; ++i)
        m_lines[m_nLines++] = math::line2<float>(points[i], points[(i + 1) % count]);
}

bool RectClipper::Inside(const math::line2<float> &l, const float *p) {
    math::vec2<float> e = l.end - l.start;
    math::vec2<float> v(p[0] - l.start.x, p[1] - l.start.y);
    return e.x * v.y - e.y * v.x >= 0.0f;
}

void RectClipper::Intersect(const math::line2<float> &l, const float *p1, const float *p2, const unsigned stride, float *out) {
    math::vec2<float> e = l.end - l.start;
    math::vec2<float> d(p2[0] - p1[0], p2[1] - p1[1]);
    float denom = e.x * d.y - e.y * d.x;
    float t = 0.0f;
    if (std::fabs(denom) > 1.1920928955078125e-07f) {
        math::vec2<float> w(l.start.x - p1[0], l.start.y - p1[1]);
        t = (e.x * w.y - e.y * w.x) / denom;
        t = std::max(0.0f, std::min(1.0f, t));
    }
    for (unsigned i = 0; i < stride; ++i)
        out[i] = p1[i] + (p2[i] - p1[i]) * t;
}

void RectClipper::Clip(const math::line2<float> &l, const float *in, const float *inEnd, const unsigned stride, unsigned &nOut, float *out) {
    unsigned nIn = static_cast<unsigned>((inEnd - in) / stride);
    nOut = 0;
    if (nIn == 0) return;
    for (unsigned i = 0; i < nIn; ++i) {
        const float *cur = in + i * stride;
        const float *prev = in + ((i + nIn - 1) % nIn) * stride;
        bool curIn = Inside(l, cur), prevIn = Inside(l, prev);
        if (curIn) {
            if (!prevIn) { Intersect(l, prev, cur, stride, out + nOut * stride); ++nOut; }
            std::copy(cur, cur + stride, out + nOut * stride); ++nOut;
        } else if (prevIn) {
            Intersect(l, prev, cur, stride, out + nOut * stride); ++nOut;
        }
    }
}

void RectClipper::ClipPrimitive(const float *points, unsigned count, unsigned stride) {
    m_input.assign(points, points + static_cast<size_t>(stride) * count);
    m_output.assign(m_input.begin(), m_input.end());
    m_numberOfOutputPoints = count;
    m_numberOfClips = 0;
    for (unsigned li = 0; li < m_nLines && m_numberOfOutputPoints > 0; ++li) {
        m_input.swap(m_output);
        unsigned nOut = 0;
        m_output.resize(m_input.size() + stride * 2);
        Clip(m_lines[li], m_input.data(), m_input.data() + m_numberOfOutputPoints * stride, stride, nOut, m_output.data());
        m_numberOfOutputPoints = nOut;
        m_output.resize(static_cast<size_t>(nOut) * stride);
        ++m_numberOfClips;
    }
}

}
