// core/jet/src/util/clipstack.cpp
#include "util/clipstack.h"
#include <limits>

namespace jet::util {

ClipStack::ClipStack() {}

unsigned ClipStack::GetSize() { return static_cast<unsigned>(m_stack.size()); }

const math::rect<float> &ClipStack::GetTop() const {
    static const math::rect<float> kFull(
        -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    return m_stack.empty() ? kFull : m_stack.top();
}

math::rect<float> &ClipStack::GetTop() {
    if (m_stack.empty()) m_stack.push(static_cast<const ClipStack*>(this)->GetTop());
    return m_stack.top();
}

void ClipStack::Push(const math::rect<float> &r, bool intersect) {
    if (intersect && !m_stack.empty()) {
        math::rect<float> clipped = r;
        clipped.clipAgainst(m_stack.top());
        m_stack.push(clipped);
    } else {
        m_stack.push(r);
    }
}

void ClipStack::Pop() { if (!m_stack.empty()) m_stack.pop(); }

}
