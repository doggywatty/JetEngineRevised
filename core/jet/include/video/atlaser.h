// core/jet/include/video/atlaser.h
#ifndef CORE_JET_INCLUDE_VIDEO_ATLASER_H
#define CORE_JET_INCLUDE_VIDEO_ATLASER_H

#include <vector>
#include "smath.h"

namespace jet::video {
// skyline rectangle packer (nodes are x, y, width)
class Atlaser {  // sizeof 0x1C
public:
    Atlaser(const math::vec2<unsigned int>&);
    Atlaser();
    const math::vec2<unsigned int> &GetSize() const;
    void Reset();
    void Reset(const math::vec2<unsigned int>&);
    bool GetRegion(math::vec2<unsigned int>&, const math::vec2<unsigned int>&);
private:
    int Fit(unsigned, const math::vec2<unsigned int>&);
    void Merge();
    math::vec2<unsigned int> mSize;  // 0x0
    std::vector<math::vec3<int> > mNodes;  // 0x8
};
}

#endif
