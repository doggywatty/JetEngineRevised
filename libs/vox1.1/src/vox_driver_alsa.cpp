// libs/vox1.1/src/vox_driver_alsa.cpp
// The sound device of Linux through ALSA. The library (libasound.so.2) is loaded when the driver is opened, so the build has no
// need of its headers or of the library; a machine that does not have it, or a sound card, gets the null driver (see
// CreateDefaultDriver). The prototypes are the ones of <alsa/asoundlib.h>.
#if defined(__linux__) && !defined(__ANDROID__)
#include <dlfcn.h>
#include "vox_driver_pumped.h"

namespace vox {

namespace {
typedef long snd_pcm_sframes_t;
typedef unsigned long snd_pcm_uframes_t;
struct Alsa {
    int (*open)(void **pcm, const char *name, int stream, int mode);
    int (*close)(void *pcm);
    int (*set_params)(void *pcm, int format, int access, unsigned channels, unsigned rate, int softResample, unsigned latencyUs);
    snd_pcm_sframes_t (*writei)(void *pcm, const void *buffer, snd_pcm_uframes_t size);
    int (*recover)(void *pcm, int err, int silent);
    int (*drain)(void *pcm);
};
Alsa s_alsa;
enum { kPcmStreamPlayback = 0, kPcmFormatS16LE = 2, kPcmAccessRwInterleaved = 3 };
}

DriverAlsa::DriverAlsa() : m_lib(0), m_pcm(0) {}

DriverAlsa::~DriverAlsa() { Shutdown(); }

bool DriverAlsa::Open(const char *device) {
    m_lib = dlopen("libasound.so.2", RTLD_NOW);
    if (!m_lib) return false;
    *(void**)&s_alsa.open = dlsym(m_lib, "snd_pcm_open");
    *(void**)&s_alsa.close = dlsym(m_lib, "snd_pcm_close");
    *(void**)&s_alsa.set_params = dlsym(m_lib, "snd_pcm_set_params");
    *(void**)&s_alsa.writei = dlsym(m_lib, "snd_pcm_writei");
    *(void**)&s_alsa.recover = dlsym(m_lib, "snd_pcm_recover");
    *(void**)&s_alsa.drain = dlsym(m_lib, "snd_pcm_drain");
    if (!s_alsa.open || !s_alsa.close || !s_alsa.set_params || !s_alsa.writei || !s_alsa.recover || !s_alsa.drain) return false;
    if (s_alsa.open(&m_pcm, device, kPcmStreamPlayback, 0) < 0 || !m_pcm) { m_pcm = 0; return false; }
    // 16 bit stereo at the rate of the driver, resampled by ALSA when the device wants another one, 100 ms of latency
    if (s_alsa.set_params(m_pcm, kPcmFormatS16LE, kPcmAccessRwInterleaved, 2, (unsigned)m_sampleRate, 1, 100000) < 0) {
        s_alsa.close(m_pcm);
        m_pcm = 0;
        return false;
    }
    return true;
}

void DriverAlsa::Init(void *) {
    DriverCallbackInterface::Init(0);
    StartThread();
}

void DriverAlsa::Shutdown() {
    StopThread();
    if (m_pcm) {
        s_alsa.drain(m_pcm);
        s_alsa.close(m_pcm);
        m_pcm = 0;
    }
    if (m_lib) {
        dlclose(m_lib);
        m_lib = 0;
    }
}

// the write blocks while the device has no room, which paces the thread
bool DriverAlsa::Output(const short *samples, int frames) {
    while (frames > 0 && m_running) {
        const snd_pcm_sframes_t n = s_alsa.writei(m_pcm, samples, (snd_pcm_uframes_t)frames);
        if (n < 0) {
            if (s_alsa.recover(m_pcm, (int)n, 1) < 0) return false;  // an underrun is recovered from, a lost device is not
            continue;
        }
        samples += n * 2;
        frames -= (int)n;
    }
    return true;
}

}  // namespace vox
#endif
