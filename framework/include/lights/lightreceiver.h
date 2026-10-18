// framework/include/lights/lightreceiver.h
#ifndef FRAMEWORK_INCLUDE_LIGHTS_LIGHTRECEIVER_H
#define FRAMEWORK_INCLUDE_LIGHTS_LIGHTRECEIVER_H

class LightReceiver {  // sizeof 0x2C
public:
    LightReceiver();
    ~LightReceiver();
    unsigned GetLightCount();
    const Light *GetLight(unsigned);
    const math::vec3<float> &GetPosition();
    void SetPosition(const math::vec3<float>&);
    float GetRadius();
    void SetRadius(float);
    bool IsActive();
    void SetActive(bool);
    void UpdateUniform(jet::video::Material&);
    void UpdateGlobalUniform();
    static void InitGlobalUniform();
protected:
    void Clear();
    void Resize(unsigned);
    void SetLights(const std::vector<Light const *,std::allocator<Light const *> >&);
    void AddLight(const Light*);
    unsigned GetVersion();
    void Touch();
public:
    static const int k_uniform_array_size;
    static const jet::String k_global_uniform_positions_name;
    static const jet::String k_global_uniform_diffuse_name;
    static const jet::String k_global_uniform_radius_name;
private:
    std::vector<Light const *,std::allocator<Light const *> > m_lights;  // 0x0
    math::vec3<float> m_position;  // 0x14
    float m_radius;  // 0x20
    unsigned m_version;  // 0x24
    bool m_isActive;  // 0x28
};

#endif
