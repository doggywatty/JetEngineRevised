// framework/include/lights/light.h
#ifndef FRAMEWORK_INCLUDE_LIGHTS_LIGHT_H
#define FRAMEWORK_INCLUDE_LIGHTS_LIGHT_H

class Light {  // sizeof 0x54
public:
    Light(bool);
    ~Light();
    unsigned GetMask();
    void SetMask(unsigned);
    jet::video::LightData &GetData();
    const jet::video::LightData &GetData();
    bool IsVisible();
    void SetVisible(bool);
    bool IsGlobal();
protected:
    unsigned GetVersion();
    void Touch();
private:
    unsigned m_mask;  // 0x0
    jet::video::LightData m_data;  // 0x4
    bool m_isVisible;  // 0x4C
    bool m_isGlobal;  // 0x4D
    unsigned m_version;  // 0x50
};

#endif
