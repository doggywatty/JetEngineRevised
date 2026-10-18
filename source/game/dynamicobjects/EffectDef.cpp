// source/game/dynamicobjects/EffectDef.cpp
#include "EffectDef.h"
#include "EffectMgr.h"
#include "camera/cameracontroller.h"
#include "camera/cameramgr.h"
#include "project.h"
#include "game/CGameConfig.h"
#include "game/GameLevel.h"
#include "game/entities/LightDef.h"

jet::String EffectDef::k_tmplName("EffectDef");

// @0x59a8d0
EffectDef::EffectDef(const clara::Template *t)
    : GameEntity(t), m_isLooped(false), m_rotateModel(false), m_isImpostor(false), m_lightDef(0), m_lightTime(0), m_useFade(false), m_fadeInTime(-1), m_fadePeakTime(-1),
      m_fadeOutTime(-1), m_shakeCameraTime(0), m_useBounceScreenAnim(false), m_bounceSide(0), m_trailDef(0) {}

// @0x59a96c: the effects playing this definition stop with it
EffectDef::~EffectDef() {
    if (EffectMgr *mgr = EffectMgr::GetInstance()) mgr->OnEffectDefDestroyed(this);
}

// @0x59aa94
void EffectDef::Init() {
    GameEntity::Init();
    GetParam(jet::String("Model"), m_modelName);
    if (!m_modelName.empty()) {
        GetParam(jet::String("Anim"), m_animName);
        GetParam(jet::String("UseLoop"), m_isLooped);
        GetParam(jet::String("RotateModel"), m_rotateModel);
        bool multiMaterials = false;
        GetParam(jet::String("UseMultiMaterials"), multiMaterials);
        LoadMaterials(m_materials, jet::String("Materials"), multiMaterials);
    }
    GetParam(jet::String("ImpactSoundEvent"), m_soundLabel);
    GetParam(jet::String("Emmiter"), m_particles);
    const PlatformToggles &toggles = CGameConfig::Current();
    if (toggles.useLowParticles && HasParam(jet::String("EmmiterLow"))) {
        jet::String low;
        GetParam(jet::String("EmmiterLow"), low);
        if (!low.empty()) m_particles = low;
    }
    GetParam(jet::String("ShakeCameraTime"), m_shakeCameraTime);
    if (const clara::Param *p = FindParamByName(jet::String("Light")))
        m_lightDef = dynamic_cast<LightDef*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)));
    GetParam(jet::String("LightTime"), m_lightTime);
    GetParam(jet::String("isImpostor"), m_isImpostor);
    if (const clara::Param *p = FindParamByName(jet::String("TrailData")))
        m_trailDef = reinterpret_cast<TrailDefEntity*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)));
    GetParam(jet::String("UseFade"), m_useFade);
    if (m_useFade) {
        GetParam(jet::String("Shader"), m_fadeTechnique);
        GetParam(jet::String("timeIn"), m_fadeInTime);
        GetParam(jet::String("timePeak"), m_fadePeakTime);
        GetParam(jet::String("timeOut"), m_fadeOutTime);
    }
    if (toggles.useShadows > 0) {
        GetParam(jet::String("ShadowModel"), m_shadowModelName);
        LoadMaterials(m_shadowMaterials, jet::String("ShadowMaterial"), false);
    }
    GetParam(jet::String("useBounceScreenAnim"), m_useBounceScreenAnim);
    if (m_useBounceScreenAnim) {
        // the animations are the names of the enumeration (the empty ones are left out)
        if (const clara::Param *p = FindParamByName(jet::String("flyingScreenAnim")))
            for (unsigned i = 0; i < p->GetComponentCount(); i++) {
                jet::String n = p->GetAsEnumName(i);
                if (!n.empty()) m_flyingScreenAnim.push_back(n);
            }
        if (const clara::Param *p = FindParamByName(jet::String("bounceScreenAnim")))
            for (unsigned i = 0; i < p->GetComponentCount(); i++) {
                jet::String n = p->GetAsEnumName(i);
                if (!n.empty()) m_bounceScreenAnim.push_back(n);
            }
        int side = 0;
        GetParam(jet::String("bounceScreenPosition"), side);
        m_bounceSide = side;
    }
}

// @0x59b214
void EffectDef::FlashScreen() const {
    GameLevel *level = GameLevel::GetInstance();
    level->SetCurrentPostEffect(level->GetPostFxMaterial(), true, m_fadeTechnique, m_fadeInTime, m_fadePeakTime, m_fadeOutTime, jet::core::InterpolationDef::LINEAR,
                                jet::core::InterpolationDef::LINEAR, 0);
}

// @0x59b2a0
void EffectDef::ShakeCamera() const {
    if (m_shakeCameraTime <= 0.0f) return;
    if (CameraController *c = CameraMgr::GetInstance()->GetCurrentController()) c->SetShakeTime(m_shakeCameraTime);
}
