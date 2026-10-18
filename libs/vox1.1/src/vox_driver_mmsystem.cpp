// libs/vox1.1/src/vox_driver_mmsystem.cpp
#ifdef _WIN32
#include "vox_driver_mmsystem.h"

namespace vox {

DriverMMSystem::DriverMMSystem() : m_waveOut(0), m_currentBuffer(0) { memset(m_blocks, 0, sizeof(m_blocks)); }

DriverMMSystem::~DriverMMSystem() { Shutdown(); }

bool DriverMMSystem::Open() {
    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.nSamplesPerSec = (DWORD)m_sampleRate;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = (WORD)(fmt.nChannels * fmt.wBitsPerSample / 8);
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
    if (waveOutOpen(&m_waveOut, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        m_waveOut = 0;
        return false;
    }
    for (int i = 0; i < kBlocks; i++) {
        m_blockData[i].assign((size_t)GetBlockFrames() * 2, 0);
        m_blocks[i].lpData = reinterpret_cast<LPSTR>(&m_blockData[i][0]);
        m_blocks[i].dwBufferLength = (DWORD)(m_blockData[i].size() * 2);
        waveOutPrepareHeader(m_waveOut, &m_blocks[i], sizeof(WAVEHDR));
        m_blocks[i].dwFlags |= WHDR_DONE;
    }
    return true;
}

void DriverMMSystem::Init(void *) {
    DriverCallbackInterface::Init(0);
    StartThread();
}

void DriverMMSystem::Shutdown() {
    StopThread();
    if (!m_waveOut) return;
    waveOutReset(m_waveOut);
    for (int i = 0; i < kBlocks; i++) waveOutUnprepareHeader(m_waveOut, &m_blocks[i], sizeof(WAVEHDR));
    waveOutClose(m_waveOut);
    m_waveOut = 0;
}

// the block of the ring that the device has played is filled and queued; the thread waits while all of them are queued
bool DriverMMSystem::Output(const short *samples, int frames) {
    while (m_running && !(m_blocks[m_currentBuffer].dwFlags & WHDR_DONE)) Sleep(1);
    if (!m_running) return true;
    memcpy(&m_blockData[m_currentBuffer][0], samples, (size_t)frames * 4);
    m_blocks[m_currentBuffer].dwFlags &= ~WHDR_DONE;
    if (waveOutWrite(m_waveOut, &m_blocks[m_currentBuffer], sizeof(WAVEHDR)) != MMSYSERR_NOERROR) return false;
    m_currentBuffer = (m_currentBuffer + 1) % kBlocks;
    return true;
}

DriverInterface *CreateMMSystemDriver() {
    DriverMMSystem *d = new DriverMMSystem();
    if (d->Open()) return d;
    delete d;
    return 0;
}

}  // namespace vox
#endif
