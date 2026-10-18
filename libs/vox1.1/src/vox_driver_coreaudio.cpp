// libs/vox1.1/src/vox_driver_coreaudio.cpp
// The sound device of the Apple platforms: an AudioQueue of three buffers whose callback asks the mixing core for the next block
// (the queue has its own thread, so the driver does not run one)
#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#include "vox_driver_pumped.h"

namespace vox {

class DriverCoreAudio : public DriverPumped {
public:
    DriverCoreAudio() : m_queue(0), m_started(false) { memset(m_buffers, 0, sizeof(m_buffers)); }
    virtual ~DriverCoreAudio() { Shutdown(); }

    bool Open() {
        AudioStreamBasicDescription fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.mSampleRate = m_sampleRate;
        fmt.mFormatID = kAudioFormatLinearPCM;
        fmt.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
        fmt.mBitsPerChannel = 16;
        fmt.mChannelsPerFrame = 2;
        fmt.mFramesPerPacket = 1;
        fmt.mBytesPerFrame = 4;
        fmt.mBytesPerPacket = 4;
        if (AudioQueueNewOutput(&fmt, &DriverCoreAudio::Callback, this, 0, 0, 0, &m_queue) != noErr) return false;
        for (int i = 0; i < kBuffers; i++) {
            if (AudioQueueAllocateBuffer(m_queue, kBlockFrames * 4, &m_buffers[i]) != noErr) return false;
            Fill(m_buffers[i]);
            AudioQueueEnqueueBuffer(m_queue, m_buffers[i], 0, 0);
        }
        return true;
    }

    virtual void Init(void *) {
        DriverCallbackInterface::Init(0);
        m_started = AudioQueueStart(m_queue, 0) == noErr;
    }

    virtual void Shutdown() {
        if (m_queue) {
            AudioQueueStop(m_queue, true);
            AudioQueueDispose(m_queue, true);
            m_queue = 0;
        }
    }

    virtual void Suspend() { if (m_queue) AudioQueuePause(m_queue); }
    virtual void Resume() { if (m_queue) AudioQueueStart(m_queue, 0); }

protected:
    // the queue pulls: there is no thread of the driver
    virtual bool Output(const short *, int) { return true; }

private:
    enum { kBuffers = 3, kBlockFrames = 441 };

    void Fill(AudioQueueBufferRef buffer) {
        FillBuffer(static_cast<short*>(buffer->mAudioData), kBlockFrames);
        buffer->mAudioDataByteSize = kBlockFrames * 4;
        m_framesOutput += kBlockFrames;
    }

    static void Callback(void *user, AudioQueueRef queue, AudioQueueBufferRef buffer) {
        DriverCoreAudio *d = static_cast<DriverCoreAudio*>(user);
        d->Fill(buffer);
        AudioQueueEnqueueBuffer(queue, buffer, 0, 0);
    }

    AudioQueueRef m_queue;
    AudioQueueBufferRef m_buffers[kBuffers];
    bool m_started;
};

DriverInterface *CreateCoreAudioDriver() {
    DriverCoreAudio *d = new DriverCoreAudio();
    if (d->Open()) return d;
    delete d;
    return 0;
}

}  // namespace vox
#endif
