// core/jet/include/util/fps.h
#ifndef CORE_JET_INCLUDE_UTIL_FPS_H
#define CORE_JET_INCLUDE_UTIL_FPS_H

#include <utility>
#include <vector>
#include "jtypes.h"

namespace jet {
namespace util {

// Frame timer with an optional frame rate cap (sleeping out the rest of the frame).
class Fps {
public:
    explicit Fps(unsigned fpsLimit = 0) : m_targetFrameTime(0), m_lastTime(0), m_frameTime(0), m_fpsLimit(0), m_fps(0), m_historyIdx(0)
    {
        SetFpsLimit(fpsLimit);
    }

    void Tick(bool limit = true);
    float GetFps() const { return m_fps; }
    float GetAverageFps() const;  // over the last second (up to 64 frames)
    unsigned GetFrameTime() const { return m_frameTime; }  // ms
    void SetFpsLimit(unsigned fps)
    {
        m_fpsLimit = fps;
        m_targetFrameTime = fps ? 1000000 / fps : 0;
    }
    unsigned GetFpsLimit() const { return m_fpsLimit; }

private:
    s64 m_targetFrameTime;  // us
    s64 m_lastTime;         // us
    unsigned m_frameTime;
    unsigned m_fpsLimit;
    float m_fps;
    std::vector<std::pair<s64, float>> m_history;
    unsigned m_historyIdx;
};

}
}

#endif
