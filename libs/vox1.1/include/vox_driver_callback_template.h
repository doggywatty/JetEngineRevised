// libs/vox1.1/include/vox_driver_callback_template.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DRIVER_CALLBACK_TEMPLATE_H
#define LIBS_VOX1_1_INCLUDE_VOX_DRIVER_CALLBACK_TEMPLATE_H

namespace vox {
class DriverCallbackInterface : public vox::DriverInterface {  // sizeof 0xB0
public:
    DriverCallbackInterface();
    virtual ~DriverCallbackInterface();
    virtual void Init(void*);
    virtual void Shutdown() = 0;
    virtual void Suspend() = 0;
    virtual void Resume() = 0;
    virtual void Update(float);
    virtual void Set3DParameter(int, void*);
    virtual void SetDSPParameter(int, void*);
    virtual void PrintDebug() = 0;
    virtual vox::DriverSourceInterface *CreateDriverSource();
    virtual void DestroyDriverSource(vox::DriverSourceInterface*);
    static vox::DriverInternalBuffer *GetWorkBuffer(int);
protected:
    void SetDefaultParameter();
    virtual void FillBuffer(short*, int);
    void _FillBuffer(short*, int);
    void _Update(float);
private:
    void _Set3DParameter(int, void*);
protected:
    static vox::DriverInternalBuffer m_sWorkBuffer;
    static vox::DriverMixingBuffer m_sMixingBuffer;
    vox::Mutex m_mutex;  // 0x4
    bool m_audioUnitActive;  // 0x8
    int m_nextSourceId;  // 0xC
    std::list<vox::DriverCallbackSourceInterface *,vox::SAllocator<vox::DriverCallbackSourceInterface *,0> > m_activeSources;  // 0x10
private:
    vox::Vox3DGeneralParameters m_3DGeneralParameters;  // 0x28
    vox::ListenerParameters m_listener;  // 0x80
};
}

namespace vox {
class DriverCallbackSourceInterface : public vox::DriverSourceInterface {  // sizeof 0x278
public:
    DriverCallbackSourceInterface(unsigned);
    virtual ~DriverCallbackSourceInterface();
    virtual void Init(void*, const vox::DriverSourceParam&);
    virtual void Play();
    virtual void Stop();
    virtual void Pause();
    virtual void Reset();
    virtual int GetState();
    virtual long GetByteOffset();
    virtual void SetByteOffset(int);
    virtual bool NeedData();
    virtual void UploadData(void*, int);
    virtual void SetGain(float);
    virtual void SetPitch(float);
    virtual float GetGain();
    virtual float GetPitch();
    virtual void Set3DParameter(int, void*);
    virtual void SetDSPParameter(int, void*);
    virtual void Update(float);
    virtual bool AllowBufferReference();
    virtual int GetBufferCount();
    virtual void FreeDisposableData(int, int&, int&);
    virtual void PrintDebug();
    virtual void FillBuffer(int*, int);
    static void Set3DParameters(vox::ListenerParameters, vox::Vox3DGeneralParameters);
    static void SetDriverSampleRate(int);
    static void SetDriverCallbackPeriod(float);
protected:
    virtual void Cleanup();
    void FreeAllBuffer();
    void FillBufferMono8(int*, int);
    void FillBufferStereo8(int*, int);
    void FillBufferMono16(int*, int);
    void FillBufferStereo16(int*, int);
    void FillBufferMono8NoInter(int*, int);
    void FillBufferStereo8NoInter(int*, int);
    void FillBufferMono16NoInter(int*, int);
    void FillBufferStereo16NoInter(int*, int);
    void FillBufferMono16Biquad(int*, int);
    int GetDirectionalGain();
    int GetDistanceGain();
    float GetDopplerPitch();
    void GetStereoPanning(int*, int*);
    void GetNormalizedPosition(float*, float*, float*);
    int GetNbAvailableSamples(int);
    int GetWorkData(unsigned char*, int, int);
public:
    static unsigned s_enable3dSimulation;
    bool m_enable3dSimulation;  // 0x4
    vox::SourceDataGenerator m_dataGenerator;  // 0x8
protected:
    static int s_driverCallbackPeriod;
    static int s_driverSampleRate;
    int m_averageDt;  // 0x10
    vox::Mutex m_mutex;  // 0x14
    float m_dataPitch;  // 0x18
    vox::TrackParams m_trackParams;  // 0x1C
    int m_nominalRampLength;  // 0x2C
    bool m_isFirstBufferFilled;  // 0x30
    int m_gain;  // 0x34
    int m_previousLeftGain;  // 0x38
    int m_previousRightGain;  // 0x3C
    int m_userPitch;  // 0x40
    int m_effectiveUserPitch;  // 0x44
    int m_deltaPitch;  // 0x48
    int m_effectivePitch;  // 0x4C
    float m_effectivePitchUpperLimit;  // 0x50
    float m_userPitchUpperLimit;  // 0x54
    int m_numBuffer;  // 0x58
    int m_currentWriteBuffer;  // 0x5C
    int m_currentReadBuffer;  // 0x60
    int m_state;  // 0x64
    int m_sourceId;  // 0x68
    int m_processedBytes;  // 0x6C
    int m_bytesPerSample;  // 0x70
    std::vector<vox::DriverBuffer,vox::SAllocator<vox::DriverBuffer,0> > m_bufferList;  // 0x74
    static vox::ListenerParameters s_listenerParameters;
    static unsigned s_distanceModel;
    static float s_dopplerFactor;
    static float s_alteredSpeedOfSound;
    vox::VoxVector3f m_position;  // 0x88
    vox::VoxVector3f m_velocity;  // 0x94
    vox::VoxVector3f m_direction;  // 0xA0
    int m_relativeToListener;  // 0xAC
    float m_maxDistance;  // 0xB0
    float m_referenceDistance;  // 0xB4
    float m_rolloffFactor;  // 0xB8
    float m_innerConeAngle;  // 0xBC
    float m_outerConeAngle;  // 0xC0
    float m_outerConeGain;  // 0xC4
    float m_cullingDistance;  // 0xC8
    static vox::Enhanced3dTweakParameters s_tweakParameters;
    vox::VoxFilter m_filter[4];  // 0xCC
    int m_leftSideDelay;  // 0x16C
    short m_delayBuffer[128];  // 0x170
    int m_delayWriteCounter;  // 0x270
    int m_delayReadCounter;  // 0x274
};
}

namespace vox {
class SourceDataGenerator : public vox::MinibusDataGeneratorInterface {  // sizeof 0x8
public:
    SourceDataGenerator();
    virtual ~SourceDataGenerator();
    virtual void GetData(int*, int, int);
    void setParent(vox::DriverCallbackSourceInterface*);
private:
    vox::DriverCallbackSourceInterface *m_parent;  // 0x4
};
}

namespace vox {
struct VoxFilter {  // sizeof 0x28
    VoxFilter();
    void setDistanceShelf(float, float);
    void setDistanceBandpass(float, float, float);
    void setNotch(float, float, float, float);
    static int clip_s16(float);
    static int clip_s32(float);
    float a0;  // 0x0
    float a1;  // 0x4
    float a2;  // 0x8
    float b1;  // 0xC
    float b2;  // 0x10
    float x1;  // 0x14
    float x2;  // 0x18
    float y1;  // 0x1C
    float y2;  // 0x20
    int err;  // 0x24
};
}

#endif
