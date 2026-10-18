// framework/src/utils/painterclipscope.cpp
#include "utils/painterclipscope.h"
#include <cmath>

static bool Same(float a, float b) { return std::fabs(a - b) <= 1.0f / 8388608.0f; }

// @0x473118
void PainterClipScope::Push(const math::rect<float> &r, bool intersect) {
    if (!m_painter) return;
    if (!Same(r.upperLeftCorner.x, r.lowerRightCorner.x) || !Same(r.upperLeftCorner.y, r.lowerRightCorner.y)) {
        m_pushCount++;
        m_painter->GetClipStack().Push(r, intersect);
    }
    m_painter->SetClippingEnabled(m_painter->GetClipStack().GetSize() != 0);
}

// @0x44a4b8
void PainterClipScope::Pop() {
    if (!m_painter) return;
    if (m_pushCount > 0) {
        m_pushCount--;
        m_painter->GetClipStack().Pop();
    }
    m_painter->SetClippingEnabled(m_painter->GetClipStack().GetSize() != 0);
}

// @0x416078
PainterClipScope::~PainterClipScope() {
    while (m_pushCount > 0) {
        m_pushCount--;
        m_painter->GetClipStack().Pop();
        m_painter->SetClippingEnabled(m_painter->GetClipStack().GetSize() != 0);
    }
}
