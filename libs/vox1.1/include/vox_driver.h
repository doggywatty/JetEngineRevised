// libs/vox1.1/include/vox_driver.h
// The driver is what plays: it mixes the sources of the emitters and gives the result to the sound device of the platform.
// A source is the part of one emitter that the driver knows: a queue of buffers of PCM, a gain, a pitch and a 3D position.
//
// DriverCallbackInterface (@vox_driver_callback_template.cpp in the .so) is the mixing core every driver shares: its
// FillBuffer mixes the active sources into a 16 bit stereo buffer, and a platform driver (the .so's vox_driver_android.cpp is an
// AudioTrack; here a WAV file writer, ALSA, the Windows waveOut and the Apple AudioQueue) calls it to get what to play.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DRIVER_H
#define LIBS_VOX1_1_INCLUDE_VOX_DRIVER_H

#include <list>
#include <string>
#include <vector>
#include "vox_mutex.h"
#include "vox_types.h"

namespace vox {

// the buffers of a source: how many, and how many bytes each one holds
struct DriverSourceParam {  // sizeof 0xC
    DriverSourceParam() : numBuffer(4), bufferSize(16384) {}
    int numBuffer;  // 0x4
    int bufferSize;  // 0x8
};

// the listener of the 3D sounds
struct ListenerParameters {
    VoxVector3f position;
    VoxVector3f velocity;
    VoxVector3f lookAt;
    VoxVector3f up;
    ListenerParameters() : lookAt(0, 0, -1), up(0, 1, 0) {}
};

// Vox3DGeneralParameters: how the 3D sounds are computed (the values SoundMgr::Initialize sets @0x63bcec)
struct Vox3DGeneralParameters {  // sizeof 0x58
    Vox3DGeneralParameters() : dopplerFactor(1.0f), speedOfSound(343.3f), distanceModel(2), enhanced3d(0) {}
    float dopplerFactor;  // 0x0
    float speedOfSound;  // 0x4
    unsigned distanceModel;  // 0x8: 0 none, 1 inverse, 2 inverse clamped, 3 linear, 4 linear clamped, 5 exponent, 6 exponent clamped
    unsigned enhanced3d;  // 0xC
};

// Vox3DEmitterParameters: the 3D parameters of a sound
struct Vox3DEmitterParameters {  // sizeof 0x1C
    Vox3DEmitterParameters()
        : relativeToListener(0), maxDistance(1000000.0f), referenceDistance(1.0f), rolloffFactor(1.0f), innerConeAngle(360.0f), outerConeAngle(360.0f), outerConeGain(0.0f) {}
    int relativeToListener;  // 0x0
    float maxDistance;  // 0x4
    float referenceDistance;  // 0x8
    float rolloffFactor;  // 0xC
    float innerConeAngle;  // 0x10
    float outerConeAngle;  // 0x14
    float outerConeGain;  // 0x18
};

class DriverSourceInterface {
public:
    virtual ~DriverSourceInterface() {}
    // the format of what will be uploaded: the track's channels and rate, 16 bit
    virtual void Init(const TrackParams &track, const DriverSourceParam &param) = 0;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Reset() = 0;
    virtual int GetState() = 0;
    // the bytes of what was uploaded that were played
    virtual long GetByteOffset() = 0;
    // true when a buffer is free
    virtual bool NeedData() = 0;
    virtual void UploadData(const void *data, int bytes) = 0;
    // the bytes that were uploaded and are not played yet
    virtual int GetQueuedBytes() = 0;
    virtual void SetGain(float gain) = 0;
    virtual void SetPitch(float pitch) = 0;
    virtual float GetGain() = 0;
    virtual float GetPitch() = 0;
    virtual void Set3DParameter(int id, const void *value) = 0;
    virtual void Update(float dt) = 0;
};

class DriverInterface {
public:
    virtual ~DriverInterface() {}
    virtual void Init(void *param) = 0;
    virtual void Shutdown() = 0;
    virtual void Suspend() = 0;
    virtual void Resume() = 0;
    virtual void Update(float dt) = 0;
    virtual void Set3DParameter(int id, const void *value) = 0;
    virtual DriverSourceInterface *CreateDriverSource() = 0;
    virtual void DestroyDriverSource(DriverSourceInterface *source) = 0;
    virtual void SetMasterGain(float gain) = 0;
    virtual int GetSampleRate() const = 0;
    virtual void PrintDebug() = 0;
};

class DriverCallbackSourceInterface;

// the mixing core
class DriverCallbackInterface : public DriverInterface {  // sizeof 0xB0
public:
    DriverCallbackInterface();
    virtual ~DriverCallbackInterface();
    virtual void Init(void *param);
    virtual void Update(float dt);
    virtual void Set3DParameter(int id, const void *value);
    virtual DriverSourceInterface *CreateDriverSource();
    virtual void DestroyDriverSource(DriverSourceInterface *source);
    virtual void SetMasterGain(float gain);
    virtual int GetSampleRate() const { return m_sampleRate; }
    // mixes `frames` frames of the active sources into out (stereo, 16 bit)
    void FillBuffer(short *out, int frames);
protected:
    void SetDefaultParameter();
    Mutex m_mutex;  // 0x4
    int m_sampleRate;
    int m_nextSourceId;  // 0xC
    std::list<DriverCallbackSourceInterface*> m_activeSources;  // 0x10
    Vox3DGeneralParameters m_3DGeneralParameters;  // 0x28
    ListenerParameters m_listener;  // 0x80
    float m_masterGain;
    float m_masterGainStep;
    std::vector<int> m_mixBuffer;
};

class DriverCallbackSourceInterface : public DriverSourceInterface {  // sizeof 0x278
public:
    DriverCallbackSourceInterface(int sampleRate);
    virtual ~DriverCallbackSourceInterface();
    virtual void Init(const TrackParams &track, const DriverSourceParam &param);
    virtual void Play();
    virtual void Stop();
    virtual void Pause();
    virtual void Reset();
    virtual int GetState();
    virtual long GetByteOffset();
    virtual bool NeedData();
    virtual void UploadData(const void *data, int bytes);
    virtual int GetQueuedBytes();
    virtual void SetGain(float gain);
    virtual void SetPitch(float pitch);
    virtual float GetGain();
    virtual float GetPitch();
    virtual void Set3DParameter(int id, const void *value);
    virtual void Update(float dt);
    // adds `frames` frames of the source to a stereo mix of 16.16 fixed samples
    void FillBuffer(int *mix, int frames, const ListenerParameters &listener, const Vox3DGeneralParameters &general);
    void SetDriverSampleRate(int rate) { m_driverSampleRate = rate; }
protected:
    struct Buffer {
        std::vector<short> data;
        int frames;  // the frames it holds
    };
    float GetDistanceGain(const VoxVector3f &relativePosition, const Vox3DGeneralParameters &general) const;
    void GetStereoPanning(const VoxVector3f &relativePosition, float &left, float &right) const;
    float GetDopplerPitch(const ListenerParameters &listener, const Vox3DGeneralParameters &general) const;
    Mutex m_mutex;  // 0x14
    TrackParams m_trackParams;  // 0x1C
    int m_driverSampleRate;
    int m_state;  // 0x64
    float m_userGain;
    float m_previousLeftGain;
    float m_previousRightGain;
    float m_userPitch;
    std::list<Buffer> m_buffers;  // the queued data; the first one is being read
    int m_numBuffer;  // 0x58
    int m_bufferSize;
    double m_readPosition;  // in frames of the first buffer
    long m_processedBytes;  // 0x6C
    int m_bytesPerFrame;
    short m_lastFrame[2];  // the last frame played, which the interpolation continues from at a buffer boundary
    // 3D
    bool m_is3D;
    Vox3DEmitterParameters m_3DParameters;
    VoxVector3f m_position;  // 0x88
    VoxVector3f m_velocity;  // 0x94
    VoxVector3f m_direction;  // 0xA0
};

// the driver of the platform that this build has: a WAV file when JET_AUDIO_WAV=<path> is set in the environment, else the sound
// device (ALSA on Linux, waveOut on Windows, the AudioQueue on Apple platforms). A device that cannot be opened (no sound card in a
// container, say) gives a driver that mixes in time and drops the samples.
DriverInterface *CreateDefaultDriver();

// the same drivers, by name: "wav", "alsa", "mmsystem", "coreaudio" or "null"
DriverInterface *CreateDriver(const char *name, const char *param);

}  // namespace vox

#endif
