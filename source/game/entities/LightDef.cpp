// source/game/entities/LightDef.cpp
#include "LightDef.h"
#include "ShadowLightPoint.h"
#include "game/Game.h"
#include "game/dynamicobjects/EffectControl.h"
#include "scene/camera.h"
#include "video/driver.h"
#include "video/textureloader.h"
#include <algorithm>

jet::String LightDef::k_tmplName("Light");
const jet::scene::Camera *LightDef::s_camera = 0;

LightMgr *LightMgr::GetInstance() {
    static LightMgr s_instance;
    return &s_instance;
}
void LightMgr::Add(Light *l) {
    if (std::find(m_lights.begin(), m_lights.end(), l) == m_lights.end()) m_lights.push_back(l);
}
void LightMgr::Remove(Light *l) {
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), l), m_lights.end());
}
bool LightMgr::HasActiveGlobalLights() const {
    for (unsigned i = 0; i < m_lights.size(); i++)
        if (m_lights[i]->isGlobal && m_lights[i]->isEnabled) return true;
    return false;
}
void LightMgr::Process() {
    jet::video::Driver *d = jet::video::Driver::GetCurrent();
    if (!d) return;
    d->ResetLights();
    for (unsigned i = 0; i < m_lights.size(); i++)
        if (m_lights[i]->isGlobal && m_lights[i]->isEnabled) d->AddLight(m_lights[i]->data);
}

LightDef::LightDef(const clara::Template *t)
    : GameEntity(t), m_isTemplate(false), m_isInstance(false), m_attachToCamera(false), m_cameraOffset(0, 0, 0), m_transitionTime(0), m_transitionElapsed(0), m_colorOn(0, 0, 0),
      m_radiusOn(0), m_colorOff(0, 0, 0), m_radiusOff(0), m_shown(false), m_switchedOn(false), m_effectControl(0) {}

LightDef *LightDef::Instantiate(const LightDef *tmpl) {
    LightDef *l = new LightDef(tmpl->GetTemplate());
    l->m_isInstance = true;
    l->ConstructFrom(static_cast<const clara::Entity*>(tmpl));
    return l;
}
LightDef::~LightDef() {
    if (m_light) LightMgr::GetInstance()->Remove(m_light.get());
}

// Init @0x5719ec: a point light at the entity, FrontColor diffuse, BackColor ambient, white
// specular, radius half the entity scale; the palette texture becomes the "LightPalette" global
void LightDef::Init() {
    GameEntity::Init();
    bool global = false;
    GetParam(jet::String("GlobalLight"), global);
    GetParam(jet::String("isTemplate"), m_isTemplate);
    math::vec3<float> front(0, 0, 0), back(0, 0, 0);
    GetParam(jet::String("FrontColor"), front);
    GetParam(jet::String("BackColor"), back);
    jet::String palette;
    GetParam(jet::String("PaletteTexture"), palette);
    if (!palette.empty()) m_paletteTexture = jet::video::TextureLoader::GetInstance()->Load(palette);
    m_light.reset(new Light());
    m_light->isGlobal = global;
    jet::video::LightData &ld = m_light->data;
    ld.SetType(jet::video::LightData::TypeDef::Point);
    ld.SetDirection(math::vec3<float>(0, 0, 1));
    ld.SetAmbientColor(back);
    ld.SetDiffuseColor(front);
    ld.SetSpecularColor(math::vec3<float>(1, 1, 1));
    ld.SetPosition(GetPosition());
    ld.SetRadius(GetScale().x * 0.5f);
    bool enableTransition = false;
    GetParam(jet::String("EnableTransition"), enableTransition);
    m_transitionTime = 0;
    if (enableTransition) {
        m_colorOn = ld.GetDiffuseColor();
        m_radiusOn = ld.GetRadius();
        GetParam(jet::String("TransitionTime"), m_transitionTime);
        GetParam(jet::String("FrontColorTransition"), m_colorOff);
        GetParam(jet::String("RadiusTransition"), m_radiusOff);
        if (m_radiusOff < 0.01f) m_radiusOff = 0.01f;
    }
    m_transitionElapsed = m_transitionTime;
    m_shown = IsVisible();
    m_switchedOn = IsEnabled();
    GetParam(jet::String("AttachToCamera"), m_attachToCamera);
    if (m_attachToCamera) GetParam(jet::String("CameraOffset"), m_cameraOffset);
    RegisterForRender(true);
    // template lights (e.g. MenuUI's) are copied into the level's light when used
    if (!m_isTemplate || m_isInstance) LightMgr::GetInstance()->Add(m_light.get());
}

// @0x571f24: the light is on when the entity is enabled and visible
void LightDef::SetEnabled(bool on) {
    if (m_transitionTime > 0) {
        m_transitionElapsed = 0;
        m_switchedOn = on;
        GameEntity::SetEnabled(true);
        return;
    }
    GameEntity::SetEnabled(on);
    if (m_light) m_light->isEnabled = IsEnabled() && IsVisible();
}

// @0x571f84
void LightDef::SetVisible(bool on) {
    if (m_transitionTime > 0) {
        m_transitionElapsed = 0;
        m_shown = on;
        GameEntity::SetVisible(true);
        return;
    }
    GameEntity::SetVisible(on);
    if (m_light) m_light->isEnabled = IsEnabled() && IsVisible();
}

// @0x5723a8: the effect switches the light on; the light is not the level's any more once the effect stops
void LightDef::StartEffectControl(EffectControl *ec) {
    StopEffectControl();
    m_effectControl = ec;
    SetVisible(true);
    SetEnabled(true);
    LightMgr::GetInstance()->Add(m_light.get());
}

// @0x57240c: the effect lets go of the light, which goes off (and out of the manager, unless it fades off in Render)
void LightDef::StopEffectControl() {
    if (m_effectControl) {
        m_effectControl->light = 0;
        m_effectControl = 0;
    }
    SetVisible(false);
    SetEnabled(false);
    if (m_transitionTime == 0) LightMgr::GetInstance()->Remove(m_light.get());
}


// AttachToCamera @0x572498
void LightDef::AttachToCamera(const jet::scene::Camera *camera, const math::vec3<float> &offset) {
    if (!camera || !m_light) return;
    jet::scene::Camera &c = const_cast<jet::scene::Camera&>(*camera);
    math::vec3<float> o = c.GetRotation() * offset;
    const math::vec3<float> &p = c.GetPosition();
    m_light->data.SetPosition(math::vec3<float>(p.x + o.x, p.y + o.y, p.z + o.z));
}

// Render @0x571fe4: follow the camera or the entity, publish the palette
void LightDef::Render() {
    if (!m_light) return;
    // @0x57200c: the light fades on or off over the transition time, the entity staying on until the fade off is done
    if (m_transitionTime > 0 && m_transitionElapsed != m_transitionTime) {
        const bool fadingOn = m_shown ? m_switchedOn : false;
        m_transitionElapsed = std::min(m_transitionElapsed + (int)Game::GetInstance()->GetRenderTime(), m_transitionTime);
        float t = (float)m_transitionElapsed / (float)m_transitionTime;
        const bool done = t >= 1.0f;
        if (fadingOn || done) {
            GameEntity::SetVisible(m_shown);
            GameEntity::SetEnabled(m_switchedOn);
            m_light->isEnabled = IsEnabled() && IsVisible();
            if (!m_light->isEnabled && done) LightMgr::GetInstance()->Remove(m_light.get());
            if (fadingOn) t = 1.0f - t;
        }
        m_light->data.SetRadius(m_radiusOn + t * (m_radiusOff - m_radiusOn));
        m_light->data.SetDiffuseColor(math::lerp(m_colorOn, m_colorOff, t));
    }
    // @0x572214: a light that is on aims the shadow camera itself when no ShadowLightPoint entity does
    if (m_light->isEnabled && m_light->isGlobal && CGameConfig::Current().useShadows > 0 && !ShadowLightPoint::GetInstance())
        ShadowLightPoint::ApplyRefPoint(GetPosition());
    if (m_attachToCamera && s_camera) AttachToCamera(s_camera, m_cameraOffset);
    else m_light->data.SetPosition(GetPosition());
    if (m_paletteTexture && jet::video::Driver::GetCurrent())
        jet::video::Driver::GetCurrent()->SetGlobalTexture(jet::String("LightPalette"), m_paletteTexture);
}

void LightDef::NotifyTransformChanged(bool) {
    if (m_light && !m_attachToCamera) m_light->data.SetPosition(GetPosition());
}
