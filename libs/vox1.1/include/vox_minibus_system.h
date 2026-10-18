// libs/vox1.1/include/vox_minibus_system.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MINIBUS_SYSTEM_H
#define LIBS_VOX1_1_INCLUDE_VOX_MINIBUS_SYSTEM_H

namespace vox {
class MiniAuxBus : public vox::MiniBus {  // sizeof 0x68
public:
    MiniAuxBus();
    virtual ~MiniAuxBus();
    virtual void Update(float);
    int GetDSPPresence();
    void FillBuffer(int*, int*, int);
    vox::CustomDSP *GetDSP();
    void SetDSP(vox::CustomDSP*);
    void RemoveDSP();
protected:
    vox::CustomDSP *m_pDsp;  // 0x60
private:
    bool m_isDspActive;  // 0x64
    bool m_needRemoveDsp;  // 0x65
};
}

namespace vox {
class MiniBus {  // sizeof 0x60
public:
    MiniBus();
    virtual ~MiniBus();
    static void SetDriverSampleRate(int);
    int GetId();
    float GetVolume(vox::VoxDSPGeneralParameter::BusRoutingType);
    void SetId(int);
    void SetVolume(vox::VoxDSPGeneralParameter::BusRoutingType, float, float);
    void RegisterDataGenerator(vox::MinibusDataGeneratorInterface*);
    bool UnregisterDataGenerator(vox::MinibusDataGeneratorInterface*);
    virtual void Update(float) = 0;
protected:
    static int s_driverSampleRate;
    int m_nbSamples;  // 0x4
    int* m_pWetBuffer;  // 0x8
    int* m_pDryBuffer;  // 0xC
    int m_volumeDry;  // 0x10
    int m_volumeWet;  // 0x14
    vox::Fader m_dryVolumeFader;  // 0x18
    vox::Fader m_wetVolumeFader;  // 0x2C
    int m_id;  // 0x40
    std::list<vox::MinibusDataGeneratorInterface *,vox::SAllocator<vox::MinibusDataGeneratorInterface *,0> > m_dataGenerators;  // 0x44
    vox::Mutex m_mutex;  // 0x5C
};
}

namespace vox {
class MiniBusManager {  // sizeof 0x20
public:
    virtual ~MiniBusManager();
    static vox::MiniBusManager *GetInstance();
    static void ReleaseInstance();
    vox::MiniMasterBus *GetMasterBus();
    void AttachDataGeneratorToBus(int, vox::MinibusDataGeneratorInterface*);
    void DetachDataGeneratorFromBus(vox::MinibusDataGeneratorInterface*);
    bool AttachDSP(const char*, vox::CustomDSP*);
    void SetBusRoutingVolumeChange(vox::BusRoutingChange*);
    void SetDriverSampleRate(int);
private:
    void _DetachDataGeneratorFromBus(vox::MinibusDataGeneratorInterface*);
    MiniBusManager();
    static vox::MiniBusManager *s_pInstance;
    static bool s_isActive;
    vox::MiniMasterBus *m_pMasterBus;  // 0x4
    std::vector<vox::MiniAuxBus *,vox::SAllocator<vox::MiniAuxBus *,0> > m_auxBuses;  // 0x8
    vox::Mutex m_busManagerMutex;  // 0x1C
};
}

namespace vox {
class MiniMasterBus : public vox::MiniBus {  // sizeof 0x78
public:
    MiniMasterBus();
    virtual ~MiniMasterBus();
    virtual void Update(float);
    void FillBuffer(int*, int);
    void RegisterInputBus(vox::MiniAuxBus*);
    void UnregisterInputBus(vox::MiniAuxBus*);
protected:
    std::list<vox::MiniAuxBus *,vox::SAllocator<vox::MiniAuxBus *,0> > m_inputBuses;  // 0x60
};
}

#endif
