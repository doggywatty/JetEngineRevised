// framework/include/lights/lightmgr.h
#ifndef FRAMEWORK_INCLUDE_LIGHTS_LIGHTMGR_H
#define FRAMEWORK_INCLUDE_LIGHTS_LIGHTMGR_H

class LightMgr : public Singleton<LightMgr> {  // sizeof 0x6C
public:
    void Add(LightReceiver*);
    void Add(Light*);
    void Remove(LightReceiver*);
    void Remove(Light*);
    unsigned HasActiveLights();
    unsigned HasActiveGlobalLights();
    unsigned HasActiveLocalLights();
    void Process();
protected:
    void TouchLights();
    void TouchReceivers();
private:
    std::vector<Light *,std::allocator<Light *> > m_globalLights;  // 0x4
    std::vector<Light *,std::allocator<Light *> > m_lights;  // 0x18
    std::vector<LightReceiver *,std::allocator<LightReceiver *> > m_receivers;  // 0x2C
    std::vector<Light *,std::allocator<Light *> > m_lightsOn;  // 0x40
    std::vector<LightReceiver *,std::allocator<LightReceiver *> > m_receiversOn;  // 0x54
    bool m_lightsChanged;  // 0x68
    bool m_receiversChanged;  // 0x69
};

#endif
