// source/game/leveltemplate/BackgroundTemplateInstance.cpp
#include "game/leveltemplate/BackgroundTemplateInstance.h"
#include "game/GameLevel.h"
#include "game/entities/BackgroundData.h"
#include "game/entities/Minion.h"
#include "game/entities/Pacesetter.h"
#include "game/entities/RocketPath.h"
#include "game/entities/Deco3d.h"
#include "game/entities/LensFlareEntity.h"
#include "game/gamemanagers/FlareMgr.h"
#include "game/gamemanagers/SkyDomeMgr.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/GameSettings.h"
#include "param.h"

const jet::String BackgroundTemplateInstance::k_skyDomeDeco("SkyDome"), BackgroundTemplateInstance::k_skyDomeObjectDeco("objectDeco");

// @0x3e03f0: the rocket path is the one the pacesetter follows in the rocket and fluffy modes
GameEntity *BackgroundTemplateInstance::ProcessCreatedEntity(const GameEntity*, GameEntity *copy, GameplayTemplateInstance*) {
    if (copy && copy->GetTemplateName().Equals(RocketPath::k_tmplName)) m_rocketPath = copy;
    return copy;
}

// @0x3e0434
void BackgroundTemplateInstance::ApplyPostFxValue() const {
    MaterialDef *fx = 0;
    if (m_def->UsePostFx(&fx))
        GameLevel::GetInstance()->SetCurrentPostEffect(fx, true, GameLevel::k_fadeFx_defaultTechnique, 300, 0, 300, jet::core::InterpolationDef::LINEAR,
                                                       jet::core::InterpolationDef::LINEAR, 0);
}

// @0x3e04b0
void BackgroundTemplateInstance::ApplyFogValue() const {
    jet::video::Color c;
    float start = 0, end = 0;
    if (!m_def->UseFog(&c, &start, &end)) return;
    GameLevel *l = GameLevel::GetInstance();
    l->SetFogDepth(start, end, 500);
    l->SetFogColor(c, 500);
}

// @0x3e0514
void BackgroundTemplateInstance::ApplySoundSettings() const {
    jet::String sound, music;
    bool hasSound = m_def->UseSound(&sound);
    bool hasMusic = m_def->UseMusic(&music);
    GameLevel *l = GameLevel::GetInstance();
    l->SetGameplaySoundGain(m_def->GetGameplaySoundGain());
    l->SetSound(hasSound ? &sound : 0, hasMusic ? &music : 0);
    if (hasMusic && music.size() && m_def->UseSoftMusic()) l->SetCurrentMusicGain(GameSettings::GetInstance()->GetSavedSoundVolume() * 0.5f);
}

// @0x3e060c
void BackgroundTemplateInstance::ApplyRocketPath() const {
    GameLevel *l = GameLevel::GetInstance();
    Minion *m = l->GetMinion();
    if (!l->IsGameplayStateCreated() || (!m->IsInFluffyMode() && !m->IsInRocketMode())) return;
    l->GetPacesetter()->SetController(m_rocketPath ? static_cast<RocketPath*>(m_rocketPath) : 0);
}

// @0x454618: the flare of the background is put at the start anchor of its template, turned with its angle
void BackgroundTemplateInstance::ApplyLensFlare() const {
    BackgroundData *bd = m_def->GetBackgroundData();
    if (!bd) return;
    FlareMgr *fm = FlareMgr::GetInstance();
    fm->CleanFlares();
    if (clara::DataEntity *flare = bd->GetLensFlare()) {
        math::vec3<float> pos(0, 0, 0);
        float angle = 0;
        GetStartAnchor(pos, angle);
        fm->AddFlare(static_cast<LensFlareEntity*>(flare), pos, math::quat<float>(math::quat<float>::axisZ, angle * 0.017453292f));
    }
}

// @0x3e0720: the sky dome the background's SkyDome parameter points to
void BackgroundTemplateInstance::ApplySkyDome() const {
    BackgroundData *bd = m_def->GetBackgroundData();
    SkyDomeMgr *mgr = SkyDomeMgr::GetInstance();
    if (bd) {
        const clara::Param *p = bd->FindParamByName(k_skyDomeDeco);
        const clara::Path &path = p->GetAsPath(0);
        if (!path.IsEmpty()) {
            clara::Project *prj = clara::Project::GetInstance();
            if (clara::DataEntity *e = prj->FindEntityByPath(path)) {
                const clara::Param *o = e->FindParamByName(k_skyDomeObjectDeco);
                mgr->AddSkyDome(*static_cast<Deco3d*>(prj->FindEntityByPath(o->GetAsPath(0))));
                return;
            }
        }
    }
    mgr->CleanOldSkyDome();
}
