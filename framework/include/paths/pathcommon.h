// framework/include/paths/pathcommon.h
#ifndef FRAMEWORK_INCLUDE_PATHS_PATHCOMMON_H
#define FRAMEWORK_INCLUDE_PATHS_PATHCOMMON_H

#include <vector>
#include "smath.h"
#include "core/interpolator.h"
#include "video/color.h"

// A path of positions, rotations and widths, smoothed by an interpolation and cut into short segments: where a point
// is on it, how far it is along it and whether a sphere touches it. Only the debug drawing of the path is not ported.
class PathCommon {  // sizeof 0x16C
public:
    PathCommon(const std::vector<math::vec3<float> > &positions, const std::vector<math::quat<float> > &rotations, const std::vector<float> &widths,
               float defaultWidth, jet::core::InterpolationType type, unsigned samplesPerSegment, float tension, float bias, jet::video::Color color);
    virtual ~PathCommon() {}
    void SetColor(jet::video::Color c) { m_color = c; }
    bool SphereTouchesPath(const math::sphere<float> &s);
    // the segment of the path closest to a point, where on it (0..1) and optionally how far
    void GetClosestSegment(const math::vec3<float> &p, unsigned &segment, float &t, float *distance);
    // moves a distance (negative backwards) from a place on the path; adds the distance travelled to *travelled
    void TravelAlongPath(unsigned segment, float t, float distance, unsigned &outSegment, float &outT, float *travelled);
    float GetTravelDistanceTo(unsigned segment, float t) const;
    float GetDistanceFromTo(unsigned seg1, float t1, unsigned seg2, float t2, bool byTime) const;
    int GetSampleTime(unsigned i) const { return m_posInterpolator.GetPath()[i].time; }
    math::vec3<float> GetPositionAtTime(int t) const { return m_posInterpolator.GetValueAtTime(t); }
    math::quat<float> GetRotationAtTime(int t) const { return m_rotInterpolator.GetValueAtTime(t); }
    int GetPathDuration() const { return m_posInterpolator.GetPath().back().time - m_posInterpolator.GetPath().front().time; }
    float GetDefaultWidth() const { return m_defaultPathWidth; }
    math::vec3<float> GetPositionAt(unsigned segment, float t) const;
    math::quat<float> GetRotationAt(unsigned segment, float t) const;
    float GetWidthAt(unsigned segment, float t) const;
    float GetSegmentLength(unsigned i) const { return m_segmentLengths[i]; }
    std::vector<math::vec3<float> > &GetInterpolatedPositions() { return m_pathPositions; }
    const std::vector<math::vec3<float> > &GetInterpolatedPositions() const { return m_pathPositions; }
    std::vector<math::quat<float> > &GetInterpolatedRotations() { return m_pathRotations; }
    const std::vector<math::quat<float> > &GetInterpolatedRotations() const { return m_pathRotations; }
    std::vector<float> &GetInterpolatedWidths() { return m_pathWidths; }
    const std::vector<float> &GetInterpolatedWidths() const { return m_pathWidths; }
    const std::vector<float> &GetSegmentLenghts() const { return m_segmentLengths; }
    math::vec3<float> GetLastInterpolatedPosition() const { return m_pathPositions.back(); }
    math::quat<float> GetLastInterpolatedRotation() const { return m_pathRotations.back(); }
    unsigned GetNumSamplesPerSegment() const { return m_numSamplesPerSegment; }
    void Translate(const math::vec3<float> &d);
    float GetTotalDistance() const { return m_totalDistance; }
private:
    void RebuildBoundingSpheres();
    bool SphereTouchesSegmentGroup(unsigned group, const math::sphere<float> &s) const;
    void MinDistFromSegmentGroup(unsigned group, const math::vec3<float> &p, unsigned &segment, float &t, float &minDist) const;
    jet::core::Interpolator<math::vec3<float> > m_posInterpolator;  // 0x3C
    jet::core::Interpolator<math::quat<float> > m_rotInterpolator;  // 0x54
    jet::core::Interpolator<float> m_widthInterpolator;             // 0x6C
    std::vector<math::vec3<float> > m_pathPositions;                // 0x84
    std::vector<math::quat<float> > m_pathRotations;                // 0x94
    std::vector<float> m_pathWidths;                                // 0xA4
    std::vector<float> m_segmentLengths;                            // 0xB4
    float m_defaultPathWidth;                                       // 0xC4
    jet::core::InterpolationType m_interpolationType;               // 0xC8
    unsigned m_numSamplesPerSegment;                                // 0xCC
    float m_tension, m_bias;                                        // 0xD0
    float m_totalDistance;                                          // 0xD8
    std::vector<math::sphere<float> > m_segmentSpheres;             // 0xDC
    math::sphere<float> m_pathBoundingSphere;                       // 0xEC
    bool m_dirtyBoundingSpheres;                                    // 0xFC
    unsigned m_lastCollidedGroup;                                   // 0x100
    jet::video::Color m_color;                                      // 0x168
};

#endif
