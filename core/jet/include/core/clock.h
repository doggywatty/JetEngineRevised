// core/jet/include/core/clock.h
#ifndef CORE_JET_INCLUDE_CORE_CLOCK_H
#define CORE_JET_INCLUDE_CORE_CLOCK_H

#include "jtypes.h"

namespace jet {
namespace stream {
class IStream;
class IStreamW;
}

namespace core {

// Millisecond stopwatch on the system clock or on the application's game time.
class Clock {
public:
    explicit Clock(bool useSystemTime = true);
    void Reset(s64 elapsed = 0);
    void Pause();
    void Resume();
    bool IsPaused() const { return m_isPaused; }
    s64 GetElapsed() const;
    s64 GetElapsedAndReset();
    void Write(stream::IStreamW& s) const;
    void Read(stream::IStream& s);

    // Game time source for clocks created with useSystemTime = false (the application sets it).
    static void SetGameTimeSource(const s64* ms);

private:
    s64 Now() const;

    bool m_useSystemTime;
    s64 m_startTimestamp;
    bool m_isPaused;
    s64 m_pausedTimestamp;
};

}
}

#endif
