// framework/include/utils/painterclipscope.h
#ifndef FRAMEWORK_INCLUDE_UTILS_PAINTERCLIPSCOPE_H
#define FRAMEWORK_INCLUDE_UTILS_PAINTERCLIPSCOPE_H

#include "smath.h"
#include "video/painter.h"

// Clips what the painter draws to a rectangle until it is popped or goes out of scope. An empty
// rectangle (no area) pushes nothing; clipping stays on while any rectangle is on the stack.
class PainterClipScope {  // sizeof 0x8
public:
    PainterClipScope(jet::video::Painter *painter, const math::rect<float> &r, bool intersect = true) : m_painter(painter), m_pushCount(0) { Push(r, intersect); }
    ~PainterClipScope();
    void Push(const math::rect<float>&, bool intersect = true);
    void Pop();
private:
    jet::video::Painter *m_painter;  // 0x0
    int m_pushCount;  // 0x4
};

#endif
