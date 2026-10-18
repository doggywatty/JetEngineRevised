// core/jet/src/core/fps.cpp
#include "util/fps.h"
#include <cmath>
#include "system.h"
#include "thread/taskmgr.h"

namespace jet {
namespace util {

static void SleepMs(unsigned ms)
{
    if (thread::TaskMgr* m = thread::TaskMgr::GetInstance())
        m->Sleep(ms);
}

// Long waits sleep; the last 2 ms yield.
void Fps::Tick(bool limit)
{
    s64 now = System::GetUTime();
    if (m_targetFrameTime && limit) {
        s64 spent = now - m_lastTime;
        if (spent < 0)
            spent = 0;
        s64 remaining = m_targetFrameTime - spent;
        while (remaining > 0) {
            s64 t0 = System::GetUTime();
            SleepMs(remaining > 2000 ? unsigned(remaining / 1000) : 0);
            remaining -= System::GetUTime() - t0;
        }
    }
    now = System::GetUTime();
    s64 dt = now - m_lastTime;
    if (dt <= 0) {
        m_fps = 0;
        m_frameTime = 0;
    } else {
        m_fps = 1000.0f / (float(dt) * 0.001f);
        m_frameTime = unsigned(dt / 1000);
    }
    if (m_history.empty())
        m_history.assign(64, std::make_pair(s64(0), 0.0f));
    m_history[m_historyIdx] = std::make_pair(now, m_fps);
    m_lastTime = now;
    m_historyIdx = (m_historyIdx + 1) & 63;
}

float Fps::GetAverageFps() const
{
    if (m_history.empty())
        return 0;
    s64 newest = m_history[(m_historyIdx - 1) & 63].first;
    float n = 0, sum = 0;
    for (unsigned i = 0; i < 64; ++i) {
        const auto& h = m_history[(m_historyIdx - i) & 63];
        if (newest - h.first <= 999999) {
            n += 1.0f;
            sum += h.second;
        }
    }
    return std::fabs(n) > 1.1920929e-07f ? sum / n : 0.0f;
}

}
}
