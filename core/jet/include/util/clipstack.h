// core/jet/include/util/clipstack.h
#ifndef CORE_JET_INCLUDE_UTIL_CLIPSTACK_H
#define CORE_JET_INCLUDE_UTIL_CLIPSTACK_H

#include <stack>
#include <deque>
#include "smath.h"

namespace jet::util {
// Push()'s bool intersects the new rect with the current top instead of pushing it raw
// (standard nested-clip-region semantics; not independently verified against the .so).
class ClipStack {  // sizeof 0x1C
public:
    ClipStack();
    unsigned GetSize();
    const math::rect<float> &GetTop() const;
    math::rect<float> &GetTop();
    void Push(const math::rect<float>&, bool);
    void Pop();
private:
    std::stack<math::rect<float>,std::deque<math::rect<float>,std::allocator<math::rect<float> > > > m_stack;  // 0x0
};
}

#endif
