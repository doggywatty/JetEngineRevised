// core/jet/src/video/atlaser.cpp
#include "video/atlaser.h"
#include <climits>

namespace jet::video {

Atlaser::Atlaser() : mSize(0, 0) {}

// @0x660ba8
Atlaser::Atlaser(const math::vec2<unsigned int> &size) : mSize(size) { Reset(size); }

const math::vec2<unsigned int> &Atlaser::GetSize() const { return mSize; }

void Atlaser::Reset() { Reset(mSize); }

// @0x660bf8: one node spanning the width minus a 1 pixel border on each side
void Atlaser::Reset(const math::vec2<unsigned int> &size) {
    mSize = size;
    mNodes.clear();
    mNodes.push_back(math::vec3<int>(1, 1, (int)size.x - 2));
}

// @0x660c70: the y a rectangle would rest at when placed on node idx, or -1
int Atlaser::Fit(unsigned idx, const math::vec2<unsigned int> &size) {
    int x = mNodes[idx].x, y = mNodes[idx].y;
    int widthLeft = (int)size.x;
    if ((unsigned)(x + size.x) > mSize.x - 1) return -1;
    for (unsigned i = idx; widthLeft > 0; i++) {
        if (mNodes[i].y > y) y = mNodes[i].y;
        if ((unsigned)(y + size.y) > mSize.y - 1) return -1;
        widthLeft -= mNodes[i].z;
    }
    return y;
}

// @0x660cf0: joins neighbouring nodes at the same height
void Atlaser::Merge() {
    for (unsigned i = 0; i + 1 < mNodes.size(); i++) {
        if (mNodes[i].y == mNodes[i + 1].y) {
            mNodes[i].z += mNodes[i + 1].z;
            mNodes.erase(mNodes.begin() + i + 1);
            i--;
        }
    }
}

// @0x660d80: lowest, then narrowest, fit
bool Atlaser::GetRegion(math::vec2<unsigned int> &out, const math::vec2<unsigned int> &size) {
    int bestHeight = INT_MAX, bestWidth = INT_MAX, bestIdx = -1;
    int rx = 0, ry = 0;
    for (unsigned i = 0; i < mNodes.size(); i++) {
        int y = Fit(i, size);
        if (y < 0) continue;
        int top = y + (int)size.y;
        if (top < bestHeight || (top == bestHeight && mNodes[i].z < bestWidth)) {
            bestHeight = top; bestIdx = (int)i; bestWidth = mNodes[i].z;
            rx = mNodes[i].x; ry = y;
        }
    }
    if (bestIdx == -1) return false;
    mNodes.insert(mNodes.begin() + bestIdx, math::vec3<int>(rx, ry + (int)size.y, (int)size.x));
    for (unsigned i = bestIdx + 1; i < mNodes.size(); ) {
        const math::vec3<int> &prev = mNodes[i - 1];
        int end = prev.x + prev.z;
        if (mNodes[i].x >= end) break;
        int shrink = end - mNodes[i].x;
        mNodes[i].x += shrink;
        mNodes[i].z -= shrink;
        if (mNodes[i].z > 0) break;
        mNodes.erase(mNodes.begin() + i);
    }
    Merge();
    out.x = (unsigned)rx; out.y = (unsigned)ry;
    return true;
}

}
