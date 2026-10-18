// core/jet/include/util/rectclipper.h
#ifndef CORE_JET_INCLUDE_UTIL_RECTCLIPPER_H
#define CORE_JET_INCLUDE_UTIL_RECTCLIPPER_H

#include <vector>
#include "smath.h"

namespace jet::util {
class RectClipper {  // sizeof 0x134
public:
    RectClipper();
    void SetRect(const math::rect<float>&);
    void SetShape(const math::vec2<float>*, unsigned, bool);
    // count points of stride floats each; the clipped polygon is left in m_output
    void ClipPrimitive(const float *points, unsigned count, unsigned stride);
protected:
    bool Inside(const math::line2<float>&, const float*);
    void Clip(const math::line2<float>&, const float*, const float*, const unsigned, unsigned&, float*);
    void Intersect(const math::line2<float>&, const float*, const float*, const unsigned, float*);
public:
    unsigned m_numberOfOutputPoints;  // 0x0
    unsigned m_numberOfClips;  // 0x4
    std::vector<float,std::allocator<float> > m_output;  // 0x8
protected:
    std::vector<float,std::allocator<float> > m_input;  // 0x1C
    unsigned m_nLines;  // 0x30
    math::line2<float> m_lines[16];  // 0x34
};
}

#endif
