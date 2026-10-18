// framework/src/paths/pathcommon.cpp
#include <cmath>
#include <cfloat>
#include "paths/pathcommon.h"

using math::vec3;
using math::quat;

// @0x597fa4: the points are interpolated (a time of 1000 per key) and resampled into samplesPerSegment + 1 pieces per segment
PathCommon::PathCommon(const std::vector<vec3<float> > &positions, const std::vector<quat<float> > &rotations, const std::vector<float> &widths,
                       float defaultWidth, jet::core::InterpolationType type, unsigned samples, float tension, float bias, jet::video::Color color)
    : m_pathPositions(positions), m_pathRotations(rotations), m_pathWidths(widths), m_defaultPathWidth(defaultWidth), m_interpolationType(type),
      m_numSamplesPerSegment(samples), m_tension(tension), m_bias(bias), m_totalDistance(0), m_dirtyBoundingSpheres(true), m_lastCollidedGroup(0), m_color(color) {
    std::vector<jet::core::InterpPoint<vec3<float> > > pp;
    for (size_t i = 0; i < m_pathPositions.size(); i++) pp.push_back(jet::core::InterpPoint<vec3<float> >(m_pathPositions[i], (int)i * 1000, tension, bias));
    m_posInterpolator = jet::core::Interpolator<vec3<float> >(pp, type);
    std::vector<vec3<float> > pos(1, m_pathPositions[0]);
    for (int i = 0; i < (int)(samples + 1) * ((int)m_pathPositions.size() - 1);) {
        i++;
        pos.push_back(m_posInterpolator.GetValueAtTime(i * 1000 / (int)(samples + 1)));
    }
    m_pathPositions.swap(pos);
    if (!m_pathRotations.empty()) {
        std::vector<jet::core::InterpPoint<quat<float> > > rp;
        for (size_t i = 0; i < m_pathRotations.size(); i++) rp.push_back(jet::core::InterpPoint<quat<float> >(m_pathRotations[i], (int)i * 1000, tension, bias));
        m_rotInterpolator = jet::core::Interpolator<quat<float> >(rp, type);
        std::vector<quat<float> > rot(1, m_pathRotations[0]);
        for (int i = 0; i < (int)(samples + 1) * ((int)m_pathRotations.size() - 1);) {
            i++;
            rot.push_back(m_rotInterpolator.GetValueAtTime(i * 1000 / (int)(samples + 1)));
        }
        m_pathRotations.swap(rot);
    }
    if (!m_pathWidths.empty()) {
        std::vector<jet::core::InterpPoint<float> > wp;
        for (size_t i = 0; i < m_pathWidths.size(); i++) wp.push_back(jet::core::InterpPoint<float>(m_pathWidths[i], (int)i * 1000, tension, bias));
        m_widthInterpolator = jet::core::Interpolator<float>(wp, type);
        std::vector<float> w(1, m_pathWidths[0]);
        for (int i = 0; i < (int)(samples + 1) * ((int)m_pathWidths.size() - 1);) {
            i++;
            w.push_back(m_widthInterpolator.GetValueAtTime(i * 1000 / (int)(samples + 1)));
        }
        m_pathWidths.swap(w);
    }
    for (size_t i = 0; i + 1 < m_pathPositions.size(); i++) {
        float d = math::distance(m_pathPositions[i], m_pathPositions[i + 1]);
        m_segmentLengths.push_back(d);
        m_totalDistance += d;
    }
}

// @0x599228: a sphere for each group of 8 segments and one for the whole path
void PathCommon::RebuildBoundingSpheres() {
    m_pathBoundingSphere = math::sphere<float>();
    m_segmentSpheres.clear();
    math::sphere<float> group;
    size_t n = m_pathPositions.size();
    for (size_t i = 1; i < n; i++) {
        const vec3<float> &a = m_pathPositions[i - 1], &b = m_pathPositions[i];
        vec3<float> center = (a + b) * 0.5f;
        float w = m_pathWidths.empty() ? GetDefaultWidth() : std::max(m_pathWidths[i - 1], m_pathWidths[i]);
        float len = (a - b).getLength();
        group.meld(math::sphere<float>(center, (len + w) * 0.5f));
        if ((i & 7) == 0 || i == n - 1) {
            m_segmentSpheres.push_back(group);
            m_pathBoundingSphere.meld(group);
            group = math::sphere<float>();
        }
    }
    m_dirtyBoundingSpheres = false;
}

// @0x599478
bool PathCommon::SphereTouchesPath(const math::sphere<float> &s) {
    if (m_dirtyBoundingSpheres) RebuildBoundingSpheres();
    if (!m_pathBoundingSphere.overlaps(s)) return false;
    if (SphereTouchesSegmentGroup(m_lastCollidedGroup, s)) return true;
    for (unsigned g = 0; g < m_segmentSpheres.size(); g++) {
        if (g != m_lastCollidedGroup && SphereTouchesSegmentGroup(g, s)) {
            m_lastCollidedGroup = g;
            return true;
        }
    }
    return false;
}

// @0x599510: some segment of the group passes within the sphere's radius plus the path's width of its centre
bool PathCommon::SphereTouchesSegmentGroup(unsigned group, const math::sphere<float> &s) const {
    if (!m_segmentSpheres[group].overlaps(s)) return false;
    unsigned end = std::min<unsigned>((unsigned)m_pathPositions.size() - 1, group * 8 + 8);
    for (unsigned i = group * 8; i < end; i++) {
        const vec3<float> &a = m_pathPositions[i], &b = m_pathPositions[i + 1];
        vec3<float> p = s.center - a, d = b - a;
        float len = d.getLength();
        vec3<float> closest;
        if (std::fabs(len) <= 1.1920929e-07f) {
            closest = a;
        } else {
            d = d * (1.0f / len);
            float t = math::dot(d, p);
            if (t < 0.0f) closest = a;
            else if (len < t) closest = b;
            else closest = a + d * t;
        }
        if (math::distance(closest, s.center) < s.radius + m_defaultPathWidth) return true;
    }
    return false;
}

// @0x599704
void PathCommon::GetClosestSegment(const vec3<float> &p, unsigned &segment, float &t, float *distance) {
    if (m_dirtyBoundingSpheres) RebuildBoundingSpheres();
    float minDist = FLT_MAX;
    MinDistFromSegmentGroup(m_lastCollidedGroup, p, segment, t, minDist);
    for (unsigned g = 0; g < m_segmentSpheres.size(); g++) {
        if (g == m_lastCollidedGroup) continue;
        if ((m_segmentSpheres[g].center - p).getLength() - m_segmentSpheres[g].radius < minDist) {
            MinDistFromSegmentGroup(g, p, segment, t, minDist);
            m_lastCollidedGroup = g;
        }
    }
    if (distance) *distance = minDist;
}

// @0x599c44
void PathCommon::MinDistFromSegmentGroup(unsigned group, const vec3<float> &p, unsigned &segment, float &t, float &minDist) const {
    unsigned end = std::min<unsigned>((unsigned)m_pathPositions.size() - 1, group * 8 + 8);
    for (unsigned i = group * 8; i < end; i++) {
        const vec3<float> &a = m_pathPositions[i], &b = m_pathPositions[i + 1];
        vec3<float> q = p - a, d = b - a;
        float len = d.getLength();
        d = d * (1.0f / len);
        float proj = math::dot(d, q), s;
        if (proj < 0.0f) s = 0.0f;
        else s = len < proj ? 1.0f : proj / len;
        float dist = math::distance(math::lerp(a, b, s), p);
        if (dist < minDist) {
            minDist = dist;
            segment = i;
            t = s;
        }
    }
}

// @0x5997fc
void PathCommon::TravelAlongPath(unsigned segment, float t, float distance, unsigned &outSegment, float &outT, float *travelled) {
    if (m_dirtyBoundingSpheres) RebuildBoundingSpheres();
    if (distance < 0.0f) {
        float want = std::fabs(distance) + (1.0f - t) * m_segmentLengths[segment];
        unsigned s = segment;
        float acc = 0.0f;
        for (const float *p = &m_segmentLengths[segment] + 1;;) {
            --p;
            float total = acc + *p;
            if (want < total) {
                outT = 1.0f - (want - acc) / *p;
                outSegment = s;
                if (travelled) *travelled += want;
                return;
            }
            if (s == 0) {
                outT = t;
                outSegment = segment;
                if (travelled) *travelled += acc;
                return;
            }
            s--;
            acc = total;
        }
    }
    distance += t * m_segmentLengths[segment];
    unsigned count = (unsigned)m_pathPositions.size() - 1;
    float acc = 0.0f;
    while (segment < count) {
        float len = m_segmentLengths[segment], total = len + acc;
        if (distance < total) {
            outT = (distance - acc) / len;
            outSegment = segment;
            if (travelled) *travelled += acc;
            return;
        }
        segment = (segment + 1) % count;
        acc = total;
    }
}

// @0x599950
float PathCommon::GetTravelDistanceTo(unsigned segment, float t) const {
    float d = 0.0f;
    for (unsigned i = 0; i != segment; i++) d += GetSegmentLength(i);
    return d + GetSegmentLength(segment) * t;
}

// @0x5999ac
float PathCommon::GetDistanceFromTo(unsigned seg1, float t1, unsigned seg2, float t2, bool byTime) const {
    if (byTime) {
        float segCount = (float)m_segmentLengths.size();
        return std::fabs((((float)seg2 + t2) * m_totalDistance - (t1 + (float)seg1) * m_totalDistance) / segCount);
    }
    if (seg2 < seg1) {
        std::swap(seg1, seg2);
        std::swap(t1, t2);
    }
    float d = GetSegmentLength(seg1) - t1 * GetSegmentLength(seg1);
    for (unsigned i = seg1 + 1; i < seg2; i++) d += GetSegmentLength(i);
    return d + GetSegmentLength(seg2) * t2;
}

// @0x599a94
vec3<float> PathCommon::GetPositionAt(unsigned segment, float t) const {
    if (segment < m_pathPositions.size()) return math::lerp(m_pathPositions[segment], m_pathPositions[segment + 1], t);
    return m_pathPositions.back();
}

// @0x599b24
quat<float> PathCommon::GetRotationAt(unsigned segment, float t) const { return math::lerp(m_pathRotations[segment], m_pathRotations[segment + 1], t); }

// @0x599b4c
float PathCommon::GetWidthAt(unsigned segment, float t) const {
    if (m_pathWidths.empty()) return GetDefaultWidth();
    return m_pathWidths[segment] + (m_pathWidths[segment + 1] - m_pathWidths[segment]) * t;
}

// @0x599de0
void PathCommon::Translate(const vec3<float> &d) {
    m_dirtyBoundingSpheres = true;
    for (size_t i = 0; i < m_pathPositions.size(); i++) m_pathPositions[i] += d;
    std::vector<jet::core::InterpPoint<vec3<float> > > path = m_posInterpolator.GetPath();
    for (size_t i = 0; i < path.size(); i++) path[i].value = path[i].value + d;
    m_posInterpolator.SetOrderedPath(path);
}
