// source/game/entities/LightDef.h
#ifndef SOURCE_GAME_ENTITIES_LIGHTDEF_H
#define SOURCE_GAME_ENTITIES_LIGHTDEF_H

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include "GameEntity.h"
#include "video/lightdata.h"
#include "video/texture.h"

namespace jet::scene { class Camera; }
class EffectControl;

// The game's light (Light, 0x54): driver light data plus its global flag
struct Light {
    jet::video::LightData data;  // 0x4
    bool isGlobal;               // 0x4c
    bool isEnabled;              // 0x4d
    Light() : isGlobal(true), isEnabled(true) {}
};

// LightMgr: global lights go to the driver for every object (Process @0x64e6f0)
class LightMgr {
public:
    static LightMgr *GetInstance();
    void Add(Light *l);
    void Remove(Light *l);
    bool HasActiveGlobalLights() const;
    // the driver's light list for the frame
    void Process();
private:
    std::vector<Light*> m_lights;
};

// "Light" template (LightDef, 0xdc; Init @0x5719ec, Render @0x571fe4)
class LightDef : public GameEntity {
public:
    explicit LightDef(const clara::Template*);
    virtual ~LightDef();
    virtual void Init();
    virtual void Render();
    virtual void NotifyTransformChanged(bool);
    // @0x571f24, @0x571f84: with a transition the light fades on and off (Render goes on with it) instead of switching
    virtual void SetEnabled(bool);
    virtual void SetVisible(bool);
    // @0x5723a8, @0x57240c: the light of an effect is on while the EffectMgr plays the effect
    void StartEffectControl(EffectControl*);
    void StopEffectControl();
    void AttachToCamera(const jet::scene::Camera *camera, const math::vec3<float> &offset);
    // LightDef(LightDef*) as MenuMgr::SetDefault3dLightFrom @0x4df8cc uses it: a live light
    // built from a template light
    static LightDef *Instantiate(const LightDef *tmpl);
    const Light *GetLightData() const { return m_light.get(); }
    const math::vec3<float> &GetCameraOffset() const { return m_cameraOffset; }

    static jet::String k_tmplName;
    // The camera lights attached to the camera follow (the level's current camera)
    static const jet::scene::Camera *s_camera;
private:
    boost::scoped_ptr<Light> m_light;                        // 0x8c
    boost::shared_ptr<jet::video::Texture> m_paletteTexture;  // 0x90
    bool m_isTemplate;
    bool m_isInstance;
    bool m_attachToCamera;                                   // 0xc8
    math::vec3<float> m_cameraOffset;                        // 0xcc
    // EnableTransition: the colour and the radius the light has when it is on, and those it fades from and to
    int m_transitionTime;                                    // 0x98: ms, 0 for none
    int m_transitionElapsed;                                 // 0x9c
    math::vec3<float> m_colorOn;                             // 0xa0
    float m_radiusOn;                                        // 0xac
    math::vec3<float> m_colorOff;                            // 0xb0
    float m_radiusOff;                                       // 0xbc
    bool m_shown, m_switchedOn;                              // 0xc0, 0xc1: where the transition goes
    EffectControl *m_effectControl;                          // 0xc4
};

#endif
