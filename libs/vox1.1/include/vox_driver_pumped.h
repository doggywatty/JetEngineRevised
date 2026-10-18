// libs/vox1.1/include/vox_driver_pumped.h
// The drivers of this build: a thread asks the mixing core (DriverCallbackInterface::FillBuffer) for a block of the sound and
// gives it to an output; the output is what makes a driver (a WAV file, the sound device of a platform, nothing).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DRIVER_PUMPED_H
#define LIBS_VOX1_1_INCLUDE_VOX_DRIVER_PUMPED_H

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include "vox_driver.h"

namespace vox {

class DriverPumped : public DriverCallbackInterface {
public:
    DriverPumped();
    virtual ~DriverPumped();
    virtual void Shutdown();
    virtual void Suspend();
    virtual void Resume();
    virtual void PrintDebug();
protected:
    // starts the thread, once the output is ready
    void StartThread();
    void StopThread();
    // gives `frames` frames (stereo, 16 bit) to the output; it returns when the output can take more, so that it paces the thread
    virtual bool Output(const short *samples, int frames) = 0;
    virtual int GetBlockFrames() const { return 512; }
    // waits until the time of the blocks given so far has passed (for the outputs that do not block themselves)
    void PaceToRealTime(int frames);
    std::atomic<bool> m_running;
    std::atomic<bool> m_suspended;
    long long m_framesOutput;
private:
    void Run();
    std::thread m_thread;
    std::chrono::steady_clock::time_point m_start;
    long long m_framesPaced;
};

// the sound goes to a file (JET_AUDIO_WAV=<path>), paced as if it was played, with its level printed when it is closed
class DriverWav : public DriverPumped {
public:
    explicit DriverWav(const char *path);
    virtual ~DriverWav();
    virtual void Init(void *param);
    virtual void Shutdown();
protected:
    virtual bool Output(const short *samples, int frames);
    virtual int GetBlockFrames() const { return 441; }
private:
    void WriteHeader(bool final);
    std::string m_path;
    FILE *m_file;
    double m_sumSquares;
    int m_peak;
    long long m_framesWritten;
    long long m_lastHeaderFrames;
};

// the mix goes nowhere, but in time (a machine without a sound device)
class DriverNull : public DriverPumped {
public:
    DriverNull() {}
    virtual void Init(void *param);
protected:
    virtual bool Output(const short *samples, int frames);
};

#if defined(__linux__) && !defined(__ANDROID__)
// ALSA, loaded at run time (libasound.so.2), so that the build does not need its headers
class DriverAlsa : public DriverPumped {
public:
    DriverAlsa();
    virtual ~DriverAlsa();
    // false when there is no sound device
    bool Open(const char *device);
    virtual void Init(void *param);
    virtual void Shutdown();
protected:
    virtual bool Output(const short *samples, int frames);
    virtual int GetBlockFrames() const { return 441; }
private:
    void *m_lib;
    void *m_pcm;
};
#endif

}  // namespace vox

#endif
