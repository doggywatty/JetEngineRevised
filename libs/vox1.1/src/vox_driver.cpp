// libs/vox1.1/src/vox_driver.cpp
// The thread that pulls the mix, the WAV file and the null outputs, and the choice of the driver of the platform
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "vox_driver_pumped.h"

namespace vox {

DriverPumped::DriverPumped() : m_running(false), m_suspended(false), m_framesOutput(0), m_framesPaced(0) {}

DriverPumped::~DriverPumped() { StopThread(); }

void DriverPumped::StartThread() {
    if (m_running) return;
    m_running = true;
    m_framesPaced = 0;
    m_start = std::chrono::steady_clock::now();
    m_thread = std::thread(&DriverPumped::Run, this);
}

void DriverPumped::StopThread() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void DriverPumped::Shutdown() { StopThread(); }

void DriverPumped::Suspend() { m_suspended = true; }

void DriverPumped::Resume() {
    // the time spent suspended is not owed to the output
    m_start = std::chrono::steady_clock::now();
    m_framesPaced = 0;
    m_suspended = false;
}

void DriverPumped::PrintDebug() { fprintf(stderr, "[vox] driver: %d Hz stereo 16 bit, %lld frames output\n", m_sampleRate, m_framesOutput); }

void DriverPumped::PaceToRealTime(int frames) {
    m_framesPaced += frames;
    const std::chrono::steady_clock::time_point due = m_start + std::chrono::microseconds(m_framesPaced * 1000000LL / m_sampleRate);
    std::this_thread::sleep_until(due);
}

void DriverPumped::Run() {
    const int block = GetBlockFrames();
    std::vector<short> buf((size_t)block * 2);
    bool outputLost = false;
    while (m_running) {
        if (m_suspended) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        FillBuffer(&buf[0], block);
        // an output that is lost goes on as the null one, so that the sources are still consumed in time
        if (outputLost || !Output(&buf[0], block)) {
            outputLost = true;
            PaceToRealTime(block);
        }
        m_framesOutput += block;
    }
}

DriverWav::DriverWav(const char *path) : m_path(path), m_file(0), m_sumSquares(0), m_peak(0), m_framesWritten(0), m_lastHeaderFrames(0) {}

DriverWav::~DriverWav() { Shutdown(); }

// the header of a 16 bit stereo PCM file; the sizes are those of the frames written so far
void DriverWav::WriteHeader(bool) {
    if (!m_file) return;
    const unsigned dataBytes = (unsigned)(m_framesWritten * 4), riff = 36 + dataBytes, fmtLen = 16, rate = (unsigned)m_sampleRate, byteRate = rate * 4;
    const unsigned short fmt = 1, ch = 2, ba = 4, bits = 16;
    fseek(m_file, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, m_file); fwrite(&riff, 4, 1, m_file); fwrite("WAVEfmt ", 1, 8, m_file); fwrite(&fmtLen, 4, 1, m_file);
    fwrite(&fmt, 2, 1, m_file); fwrite(&ch, 2, 1, m_file); fwrite(&rate, 4, 1, m_file); fwrite(&byteRate, 4, 1, m_file);
    fwrite(&ba, 2, 1, m_file); fwrite(&bits, 2, 1, m_file);
    fwrite("data", 1, 4, m_file); fwrite(&dataBytes, 4, 1, m_file);
    fseek(m_file, 0, SEEK_END);
    fflush(m_file);
    m_lastHeaderFrames = m_framesWritten;
}

void DriverWav::Init(void *) {
    DriverCallbackInterface::Init(0);
    m_file = fopen(m_path.c_str(), "wb");
    if (!m_file) {
        fprintf(stderr, "[vox] cannot open %s for the WAV output\n", m_path.c_str());
        return;
    }
    WriteHeader(false);
    StartThread();
}

bool DriverWav::Output(const short *samples, int frames) {
    if (!m_file) return false;
    fwrite(samples, 4, (size_t)frames, m_file);
    for (int i = 0; i < frames * 2; i++) {
        m_sumSquares += (double)samples[i] * samples[i];
        const int a = samples[i] < 0 ? -samples[i] : samples[i];
        if (a > m_peak) m_peak = a;
    }
    m_framesWritten += frames;
    // the header is kept right, so that the file can be read while the game runs
    if (m_framesWritten - m_lastHeaderFrames >= m_sampleRate) WriteHeader(false);
    PaceToRealTime(frames);
    return true;
}

void DriverWav::Shutdown() {
    StopThread();
    if (!m_file) return;
    WriteHeader(true);
    fclose(m_file);
    m_file = 0;
    const double rms = m_framesWritten ? sqrt(m_sumSquares / ((double)m_framesWritten * 2)) : 0.0;
    fprintf(stderr, "[vox] wav %s: %lld frames at %d Hz (%.2f s), rms %.1f, peak %d\n", m_path.c_str(), m_framesWritten, m_sampleRate,
            (double)m_framesWritten / m_sampleRate, rms, m_peak);
}

void DriverNull::Init(void *) {
    DriverCallbackInterface::Init(0);
    StartThread();
}

bool DriverNull::Output(const short *, int frames) {
    PaceToRealTime(frames);
    return true;
}

DriverInterface *CreateDriver(const char *name, const char *param) {
    if (!strcmp(name, "wav")) return new DriverWav(param ? param : "vox.wav");
    if (!strcmp(name, "null")) return new DriverNull();
#if defined(__linux__) && !defined(__ANDROID__)
    if (!strcmp(name, "alsa")) {
        DriverAlsa *d = new DriverAlsa();
        if (d->Open(param ? param : "default")) return d;
        delete d;
        return 0;
    }
#endif
    return 0;
}

DriverInterface *CreateMMSystemDriver();
DriverInterface *CreateCoreAudioDriver();

DriverInterface *CreateDefaultDriver() {
    const char *wav = getenv("JET_AUDIO_WAV");
    if (wav && *wav) return CreateDriver("wav", wav);
    DriverInterface *d = 0;
#if defined(_WIN32)
    d = CreateMMSystemDriver();
#elif defined(__APPLE__)
    d = CreateCoreAudioDriver();
#elif defined(__linux__) && !defined(__ANDROID__)
    d = CreateDriver("alsa", getenv("JET_AUDIO_DEVICE"));
#endif
    return d ? d : new DriverNull();
}

}  // namespace vox
