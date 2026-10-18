// source/game/leveltemplate/LevelTemplateDef.cpp
#include <algorithm>
#include "game/leveltemplate/LevelTemplateDef.h"
#include "game/entities/TemplateData.h"
#include "game/entities/BackgroundData.h"
#include "game/entities/Deco3d.h"
#include "game/entities/PlatformInstance.h"
#include "game/CGameConfig.h"
#include "param.h"

const jet::String LevelTemplateDef::k_DecoGenerationGroupDefinition_tmplName("DecoGenerationGroupDefinition"),
    LevelTemplateDef::k_paramDecoGroupDefinitions("DecoGroupDefinitions"), LevelTemplateDef::k_paramDecoGenerationGroup("DecoGenerationGroup"),
    LevelTemplateDef::k_paramMinDecos("MinDecos"), LevelTemplateDef::k_paramMaxDecos("MaxDecos");

// @0x3cec2c
void LevelTemplateDef::EntityGenerationGroup::AddEntity(const GenerationGroupEntity *e) {
    if (std::find(entities.begin(), entities.end(), e) == entities.end()) entities.push_back(e);
}

// @0x3e8108
LevelTemplateDef::LevelTemplateDef() : m_templateId(0), m_position(0, 0), m_size(0, 0), m_processed(false), m_templateData(0), m_backgroundData(0) {}

LevelTemplateDef::~LevelTemplateDef() {}

// @0x3e81e4
void LevelTemplateDef::AddEntity(clara::DataEntity *e) {
    GameEntity *g = dynamic_cast<GameEntity*>(e);
    if (!g) {
        if (!e) return;
        if (e->GetTemplateName().Equals(BackgroundData::k_tmplName)) {
            LoadEntityGenerationGroupsData(e);
            SetBackgroundData(static_cast<BackgroundData*>(e));
        } else if (e->GetTemplateName().Equals(TemplateData::k_tmplName)) {
            SetTemplateData(static_cast<TemplateData*>(e));
        }
        return;
    }
    float x = g->GetPositionOnTemplate().x;
    std::vector<GameEntity*>::iterator at = m_entities.begin();
    while (at != m_entities.end() && (*at)->GetPositionOnTemplate().x <= x) ++at;
    if (Deco3d *d = dynamic_cast<Deco3d*>(g))
        if (d->GetGenerationGroup().group != 0) GetEntityGenerationGroup(d->GetGenerationGroup().group).AddEntity(&d->GetGenerationGroup());
    m_entities.insert(at, g);
}

// @0x3e8418: the position and size of the template, the randomizer of its ground platform; the ground platform goes first
void LevelTemplateDef::ProcessTemplateData() {
    if (m_processed) return;
    m_processed = true;
    math::vec3<float> p = m_templateData->GetPosition();
    m_position = math::vec2<float>(p.x, p.y);
    math::vec3<float> s = m_templateData->GetScale();
    m_size = math::vec2<float>(s.x, s.y);
    PlatformInstance *ground = m_templateData->GetGroundPlatform(true);
    if (ground) ground->ProcessRandomizerData(this);
    std::vector<GameEntity*>::iterator it = std::find(m_entities.begin(), m_entities.end(), static_cast<GameEntity*>(ground));
    if (it == m_entities.begin()) return;
    if (it != m_entities.end()) std::swap(*m_entities.begin(), *it);
}

// @0x3e8598
void LevelTemplateDef::SetScaleMult(float m) {
    float old = m_size.x;
    m_size.x = m * old;
    m_position.x += (m_size.x - old) * 0.5f;
}

PlatformInstance *LevelTemplateDef::GetGroundPlatform() const { return m_templateData->GetGroundPlatform(false); }
bool LevelTemplateDef::IsBonusTemplate() const { return m_templateData->IsBonusSection(); }
bool LevelTemplateDef::IsShootingArea() const { return m_templateData->IsShootingSection(); }
bool LevelTemplateDef::IsPauseTemplate() const { return m_templateData->IsLaunchSection() || m_templateData->IsTransitionSection(); }
bool LevelTemplateDef::IsTransitionTemplate() const { return m_templateData->IsTransitionSection(); }
bool LevelTemplateDef::IsGameplayTemplate() const { return m_templateData->IsGameplayTemplate(); }
bool LevelTemplateDef::UsePostFx(MaterialDef **fx) const { return m_backgroundData->UsePostFx(fx); }
bool LevelTemplateDef::UseFog(jet::video::Color *c, float *s, float *e) const { return m_backgroundData->UseFog(c, s, e); }
bool LevelTemplateDef::UseSound(jet::String *l) const { return m_backgroundData->UseSound(l); }
bool LevelTemplateDef::UseMusic(jet::String *l) const { return m_backgroundData->UseMusic(l); }
bool LevelTemplateDef::UseSoftMusic() const { return m_backgroundData->UseSoftMusic(); }
float LevelTemplateDef::GetGameplaySoundGain() const { return m_backgroundData->GetGameplaySoundGain(); }
math::vec3<float> LevelTemplateDef::GetTemplateDataPosition() const { return m_templateData->GetPosition(); }

// @0x3e8570: the template data's name, else the background's
const jet::String &LevelTemplateDef::GetName() const { return m_templateData ? m_templateData->GetName() : m_backgroundData->GetName(); }

// @0x3e868c: an empty instance for every group
void LevelTemplateDef::InitEntityGenerationMap(GenerationMap &map) const {
    map.clear();
    for (std::map<unsigned, EntityGenerationGroup>::const_iterator i = m_groups.begin(); i != m_groups.end(); ++i) map[i->first];
}

// @0x3e86fc: how many decos each group allows, between its min and max by the device's lerp
void LevelTemplateDef::LoadEntityGenerationGroupsData(clara::DataEntity *background) {
    const clara::Param *p = background->FindParamByName(k_paramDecoGroupDefinitions);
    if (!p) return;
    double lerp = CGameConfig::Current().decoGenerationGroupAllowedDecosLerp;
    for (unsigned i = 0; i < p->GetComponentCount(); i++) {
        clara::DataEntity *d = p->GetAsEntity(i);
        int group = 0, mn = 0, mx = 0;
        d->GetParam(k_paramDecoGenerationGroup, group);
        d->GetParam(k_paramMinDecos, mn);
        d->GetParam(k_paramMaxDecos, mx);
        double v = (double)(unsigned)mx * lerp + (1.0 - lerp) * (double)(unsigned)mn;
        GetEntityGenerationGroup((unsigned)group).maxDecos = v > 0.0 ? (unsigned)(long long)v : 0;
    }
}
