// source/game/entities/GameEntity.cpp
#include "GameEntity.h"
#include "TemplateData.h"
#include "Minion.h"
#include "BackgroundData.h"
#include "MaterialDef.h"
#include "game/leveltemplate/LevelTemplateInstance.h"
#include "game/GameLevel.h"
#include "scene/camera.h"
#include <cctype>
#include <cstdint>
#include <cstdio>
#include "MaterialDef.h"
#include "scene/model.h"
#include "scene/modelbase.h"
#include "video/material.h"
#include "video/textureloader.h"
#include "video/driver.h"
#include "video/shaderuniform.h"
#include "scene/dummy.h"
#include "core/util.h"
#include <cmath>
#include <string>

int GameEntity::s_frameDt = 0;

// @0x3f8060
jet::String GameEntity::GetScriptPrefix() const {
    char buf[512];
    snprintf(buf, sizeof(buf), "X_%s_%d", GetName().empty() ? "" : GetName().c_str(), (int)(intptr_t)this);
    for (char *c = buf; *c; c++)
        if (!isalnum((unsigned char)*c)) *c = '_';
    return jet::String(buf);
}

// @0x5640a8
GameEntity::GameEntity(const clara::Template *t)
    : clara::Entity(t), m_type(EntityTypeDef::Unknown), m_persistenceType(PersistenceTypeDef::Default), m_destroyed(false), m_appearProbability(100),
      m_isRegisteredForUpdate(false), m_isRegisteredForRender(false), m_distanceInRun(0), m_originalEntity(0), m_pendingToDelete(false), m_distanceToDelete(0) {}

// @0x56411c
GameEntity::~GameEntity() {
    if (m_isRegisteredForUpdate) _RegisterForUpdate(false);
    if (m_isRegisteredForRender) _RegisterForRender(false);
    if (m_persistenceType == PersistenceTypeDef::Volatile && GameLevel::GetInstance()) GameLevel::GetInstance()->RegisterVolatileEntity(this, false);
}

// @0x564208
void GameEntity::Init() {
    if (m_flags & F_INIT) return;
    clara::Entity::Init();
}

// @0x564218
void GameEntity::PostInit() {
    if (m_flags & F_POSTINIT) return;
    clara::Entity::PostInit();
}

// @0x56529c: the attached sounds and particle systems that follow the entity are not ported yet
void GameEntity::Update() { clara::Entity::Update(); }

// @0x564cc8
void GameEntity::_RegisterForUpdate(bool on) {
    if (GameLevel::GetInstance()) GameLevel::GetInstance()->RegisterForUpdate(this, on);
}

// @0x564cfc
void GameEntity::_RegisterForRender(bool on) {
    if (GameLevel::GetInstance()) GameLevel::GetInstance()->RegisterForRender(this, on);
}

// @0x564c88: the level lists only hold an entity while it is also enabled (update) or visible (render)
void GameEntity::RegisterForUpdate(bool on) {
    if (m_isRegisteredForUpdate == on) return;
    m_isRegisteredForUpdate = on;
    _RegisterForUpdate(on && IsEnabled());
}

// @0x564ca8
void GameEntity::RegisterForRender(bool on) {
    if (m_isRegisteredForRender == on) return;
    m_isRegisteredForRender = on;
    _RegisterForRender(on && IsVisible());
}

// @0x51806c
void GameEntity::SetEnabled(bool on) {
    if (on == IsEnabled()) return;
    clara::Entity::SetEnabled(on);
    if (m_isRegisteredForUpdate) _RegisterForUpdate(on);
}

// @0x565630
void GameEntity::SetVisible(bool on) {
    if (on == IsVisible()) return;
    clara::Entity::SetVisible(on);
    if (m_isRegisteredForRender) _RegisterForRender(on);
}

// @0x5180a8
void GameEntity::Translate(const math::vec3<float> &d) { SetPosition(GetPosition() + d); }

// @0x564d30
void GameEntity::SetPersistenceType(int t) {
    if (m_persistenceType == t) return;
    GameLevel *l = GameLevel::GetInstance();
    if (l && m_persistenceType == PersistenceTypeDef::Volatile) l->RegisterVolatileEntity(this, false);
    m_persistenceType = t;
    if (l && t == PersistenceTypeDef::Volatile) l->RegisterVolatileEntity(this, true);
}

// @0x5655c8
void GameEntity::ConstructFrom(const clara::Entity *o) {
    clara::Entity::ConstructFrom(o);
    m_originalEntity = const_cast<clara::Entity*>(o);
}

// @0x5655c8: a copy of a copy remembers the first original
void GameEntity::ConstructFrom(const GameEntity *o) {
    clara::Entity::ConstructFrom(static_cast<const clara::Entity*>(o));
    m_originalEntity = o->m_originalEntity ? o->m_originalEntity : const_cast<GameEntity*>(o);
}

// @0x565fb4
void GameEntity::SetPendingToDelete(unsigned distance) {
    m_pendingToDelete = true;
    m_distanceToDelete = distance;
    for (unsigned i = 0; i < GetLinkedEntitiesCount(); i++)
        if (GameEntity *g = dynamic_cast<GameEntity*>(GetLinkedEntity(i))) g->SetPendingToDelete(distance);
}

// @0x565f58: pending to delete and the minion has run past the distance
bool GameEntity::IsOutOfScene() const {
    return m_pendingToDelete && m_distanceToDelete <= GameLevel::GetInstance()->GetMinion()->GetTraveledDistance();
}

// @0x565734
bool GameEntity::CheckVisibility(float rangeSq, float, const math::vec3<float> &pos, float nearSq) {
    if (rangeSq < nearSq) return true;
    return !(rangeSq <= math::distanceSq(pos, GameLevel::GetInstance()->GetCamera()->GetPosition()));
}

// Texture placeholders of RenderPass "Textures", in the model's map slot order
static int PlaceholderSlot(const jet::String &t) {
    static const char *names[] = { "<Diffuse>", "<Ambient>", "<Bump>", "<SpecularCol>", "<SpecularLvl>", "<Glossiness>",
                                   "<SelfIllum>", "<Opacity>", "<FilterCol>", "<Reflection>", "<Refraction>", "<Displacement>" };
    for (int i = 0; i < 12; i++)
        if (t.Equals(names[i])) return i;
    return -1;
}

// ApplyMaterials @0x5642f4: the definition is built into the model's own material; texture layers
// named by a placeholder take the model's texture from that map slot.
void GameEntity::ApplyMaterials(jet::scene::Model *model, const jet::String &name, MaterialDef *def) const {
    if (!model || !def) return;
    int idx = model->FindMaterialIdxByName(name);
    if (idx < 0) return;
    jet::video::Material &mat = model->GetMutableMaterial((unsigned)idx);
    std::vector<std::vector<jet::String> > names;
    def->LoadMaterial(mat, names);
    const std::vector<jet::String> &slots = model->GetBase()->GetMaterialData((unsigned)idx).textureNames;
    jet::video::TextureLoader *tl = jet::video::TextureLoader::GetInstance();
    const bool separateAlpha = jet::video::Driver::GetCurrent() && jet::video::Driver::GetCurrent()->GetSeparateAlphaTextures();
    for (unsigned p = 0; p < names.size() && p < mat.GetRenderPassCount(); p++)
        for (unsigned l = 0; l < names[p].size() && l < mat.GetTextureLayerCount(p); l++) {
            int slot = PlaceholderSlot(names[p][l]);
            if (slot < 0) continue;
            if ((unsigned)slot >= slots.size() || slots[slot].empty()) continue;
            jet::video::TextureLayer &layer = mat.GetTextureLayer(p, l);
            layer.SetTexture(tl->Load(slots[slot]));
            if (separateAlpha) {
                jet::String alpha = slots[slot];
                alpha.append("__alpha");
                if (tl->CanLoad(alpha)) layer.SetAlphaTexture(tl->Load(alpha));
            }
        }
}

// @0x56470c: every material of the model
void GameEntity::ApplyMaterials(jet::scene::Model *model, MaterialDef *def) const {
    if (!model) return;
    for (unsigned i = 0; i < model->GetMaterialCount(); i++) ApplyMaterials(model, model->GetMaterialName(i), def);
}

// @0x564228: an unnamed entry applies to all materials
void GameEntity::ApplyMaterials(jet::scene::Model *model, const MaterialDefs &defs) const {
    for (unsigned i = 0; i < defs.size(); i++) {
        if (defs[i].first.empty()) ApplyMaterials(model, defs[i].second);
        else ApplyMaterials(model, defs[i].first, defs[i].second);
    }
}

// LoadMaterials @0x56477c: a multi-material parameter gives (component name, material) pairs, a
// single one ("", material)
void GameEntity::LoadMaterials(MaterialDefs &out, const jet::String &param, bool multi) const {
    out.clear();
    clara::Project *project = clara::Project::GetInstance();
    const clara::Param *p = FindParamByName(param);
    if (!p || !project) return;
    if (multi) {
        out.reserve(p->GetComponentCount());
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            MaterialDef *def = dynamic_cast<MaterialDef*>(project->FindEntityByPath(p->GetAsPath(i)));
            out.push_back(std::make_pair(p->GetComponentName(i), def));
        }
    } else {
        MaterialDef *def = dynamic_cast<MaterialDef*>(project->FindEntityByPath(p->GetAsPath(0)));
        if (def) out.push_back(std::make_pair(jet::String(), def));
    }
}

// jet::core::Tokenize<jet::String, jet::String> @0x5845ec: the pieces between the characters of the separator set (no empty piece)
static std::vector<std::string> TokenizeBySet(const std::string &s, const char *set) {
    std::vector<std::string> pieces;
    size_t begin = s.find_first_not_of(set);
    while (begin != std::string::npos) {
        size_t end = s.find_first_of(set, begin);
        if (end == std::string::npos) end = s.size();
        pieces.push_back(s.substr(begin, end - begin));
        begin = s.find_first_not_of(set, end);
    }
    return pieces;
}

// the number of an "Animated_Dummy<n>" name: that of its only piece after the prefix, else 0
static int AnimatedDummyIndex(const jet::String &name, bool *hasNumber) {
    std::vector<std::string> pieces = TokenizeBySet(name.c_str(), "Animated_Dummy");
    if (hasNumber) *hasNumber = pieces.size() == 1;
    return pieces.size() == 1 ? atoi(pieces[0].c_str()) : 0;
}

// @0x5657a0
void GameEntity::BuildAnimatedDummyList(jet::scene::Model *model, std::vector<jet::scene::Dummy*> &out) const {
    out.clear();
    if (!model) return;
    const char *prefix = "Animated_Dummy";
    std::vector<jet::scene::Dummy*> named;
    int count = 0;
    for (unsigned i = 0; i < model->GetDummyCount(); i++) {
        jet::scene::Dummy *d = model->GetDummy(i);
        const char *name = d->GetName().c_str();
        if (jet::core::Stristr(name, prefix) != name) continue;
        named.push_back(d);
        bool hasNumber;
        int n = AnimatedDummyIndex(d->GetName(), &hasNumber);
        if (hasNumber && count < n + 1) count = n + 1;
    }
    for (int n = 0; n < count; n++) {
        out.push_back(0);
        for (size_t j = 0; j < named.size(); j++)
            if (AnimatedDummyIndex(named[j]->GetName(), 0) == n) {
                out.back() = named[j];
                break;
            }
    }
}

// @0x565ab8: every material of the model
bool GameEntity::UpdateShaderUniformByAnimatedDummy(std::vector<jet::scene::Dummy*> &dummies, jet::scene::Model *model) const {
    if (dummies.empty()) return false;
    bool changed = false;
    for (unsigned i = 0; i < model->GetMaterialCount(); i++) changed |= UpdateShaderUniformByAnimatedDummy(dummies, model->GetMutableMaterial(i));
    return changed;
}

// @0x565b28
bool GameEntity::UpdateShaderUniformByAnimatedDummy(std::vector<jet::scene::Dummy*> &dummies, jet::video::Material &mat) const {
    bool changed = false;
    for (size_t i = 0; i < dummies.size(); i++) {
        if (!dummies[i]) continue;
        const jet::scene::Node *node = dummies[i]->GetNode();
        for (unsigned pass = 0; pass < mat.GetRenderPassCount(); pass++) {
            int idx = mat.FindUniformIdxByName(pass, jet::String::Format("animdummyscale%d", (int)i));
            if (idx >= 0) {
                const math::vec3<float> value = node->GetScale() - math::vec3<float>(1, 1, 1);
                math::vec3<float> current(0, 0, 0);
                mat.GetUniform(pass, (unsigned)idx).Get(0, &current, 1);
                if (current.x != value.x || current.y != value.y || current.z != value.z) {
                    mat.SetUniform(pass, (unsigned)idx, jet::video::ShaderUniform(value));
                    changed = true;
                }
            }
            idx = mat.FindUniformIdxByName(pass, jet::String::Format("animdummyrot%d", (int)i));
            if (idx >= 0) {
                // the rotation as angles (about x, y and z) in turns
                const math::quat<float> &q = node->GetRotation();
                const float s = 2.0f * (q.x * q.z + q.y * q.w);
                float roll, pitch, yaw = 0.0f;
                if (s >= 0.9999f) {
                    pitch = 1.5707963705062866f;
                    roll = atan2f(2.0f * (q.w * q.z + q.y * q.x), 1.0f - 2.0f * (q.z * q.z + q.x * q.x));
                } else if (s <= -0.9999f) {
                    pitch = -1.5707963705062866f;
                    roll = -atan2f(2.0f * (q.w * q.z + q.y * q.x), 1.0f - 2.0f * (q.z * q.z + q.x * q.x));
                } else {
                    pitch = asinf(s);
                    roll = atan2f(-2.0f * (-(q.w * q.x) + q.y * q.z), 1.0f - 2.0f * (q.y * q.y + q.x * q.x));
                    yaw = atan2f(-2.0f * (-(q.w * q.z) + q.y * q.x), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
                }
                const float turn = 0.15915493667125702f;
                const math::vec3<float> value(roll * turn, pitch * turn, yaw * turn);
                math::vec3<float> current(0, 0, 0);
                mat.GetUniform(pass, (unsigned)idx).Get(0, &current, 1);
                if (current.x != value.x || current.y != value.y || current.z != value.z) {
                    mat.SetUniform(pass, (unsigned)idx, jet::video::ShaderUniform(value));
                    changed = true;
                }
            }
        }
    }
    return changed;
}

// @0x566038
bool GameEntity::CanSpawnNewEntity(LevelTemplateInstance *inst) const {
    if (inst && !inst->IsEntityAllowed(&m_generation)) return false;
    const jet::String &n = GetTemplateName();
    return !(n.Equals(TemplateData::k_tmplName) || n.Equals(BackgroundData::k_tmplName) || n.Equals(MaterialDef::k_tmplName));
}
