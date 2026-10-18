// core/jet/src/core/clock.cpp
#include "core/clock.h"
#include "stream/istream.h"
#include "system.h"

namespace jet {
namespace core {

static const s64* s_gameTime = nullptr;

void Clock::SetGameTimeSource(const s64* ms) { s_gameTime = ms; }

s64 Clock::Now() const { return m_useSystemTime || !s_gameTime ? System::GetTime() : *s_gameTime; }

Clock::Clock(bool useSystemTime) : m_useSystemTime(useSystemTime), m_isPaused(false), m_pausedTimestamp(0) { m_startTimestamp = Now(); }

void Clock::Reset(s64 elapsed)
{
    if (m_isPaused)
        m_pausedTimestamp = elapsed;
    else
        m_startTimestamp = Now() - elapsed;
}

void Clock::Pause()
{
    if (m_isPaused)
        return;
    m_pausedTimestamp = GetElapsed();
    m_isPaused = true;
}

void Clock::Resume()
{
    if (!m_isPaused)
        return;
    m_isPaused = false;
    Reset(m_pausedTimestamp);
}

s64 Clock::GetElapsed() const
{
    if (m_isPaused)
        return m_pausedTimestamp;
    s64 d = Now() - m_startTimestamp;
    return d < 0 ? 0 : d;
}

s64 Clock::GetElapsedAndReset()
{
    s64 e = GetElapsed();
    Reset();
    return e;
}

void Clock::Write(stream::IStreamW& s) const
{
    s64 e = GetElapsed();
    s << m_isPaused << e;
}

void Clock::Read(stream::IStream& s)
{
    s64 e = 0;
    bool p = false;
    s >> p >> e;
    m_isPaused = false;
    Reset(e);
    if (p)
        Pause();
}

}
}
