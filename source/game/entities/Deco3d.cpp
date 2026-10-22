// source/game/entities/Deco3d.cpp
#include "Deco3d.h"
#include "MaterialDef.h"
#include "game/CGameConfig.h"
#include "scene/model.h"
#include "scene/modelloader.h"
#include "scene/multiresmesh.h"
#include "scene/scenemgr.h"
#include "scene/camera.h"
#include "anim/animationloader.h"
#include "anim/animation.h"
#include "video/shaderuniform.h"
#include "video/material.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#ifdef _WIN32
#include <string.h>
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif
#include <cstring>

jet::String Deco3d::k_tmplName("Deco3d");
const jet::String Deco3d::k_model("Model");
const jet::String Deco3d::k_lowModel("LowModel");
const jet::String Deco3d::k_animation("Animation");
const jet::String Deco3d::k_texAnimation("TexAnimation");
const jet::String Deco3d::k_useTexAnimationLoop("UseTexAnimationLoop");
const jet::String Deco3d::k_animSpeed("AnimSpeed");
const jet::String Deco3d::k_useLoop("UseLoop");
const jet::String Deco3d::k_useMultiMaterials("UseMultiMaterials");
const jet::String Deco3d::k_lowUseMultiMaterials("LowUseMultiMaterials");
const jet::String Deco3d::k_ChooseRandAnimOnAnimEnd("ChooseRandAnimOnAnimEnd");
const jet::String Deco3d::k_useMovementDummy("UseMovementDummy");
const jet::String Deco3d::k_materials("Materials");
const jet::String Deco3d::k_lowMaterials("LowMaterials");
const jet::String Deco3d::k_trailData("TrailData");
const jet::String Deco3d::k_alphaScale("AlphaScale");
const jet::String Deco3d::k_shadowModel("ShadowModel");
const jet::String Deco3d::k_shadowMaterial("ShadowMaterial");
const jet::String Deco3d::k_updateAnimOnUpdate("UpdateAnimOnUpdate");
const jet::String Deco3d::k_generationGroup("DecoGenerationGroup");
const jet::String Deco3d::k_optional("Optional");

// the one model that is always loaded, even when optional decos are hidden
static const char *k_alwaysLoadedModel = "macho_cages_staticmesh_b.pig";

Deco3d::Deco3d(const clara::Template *t)
    : GameEntity(t), m_shadowModel(0), m_shadowEnabled(true), m_model(0), m_rotationToReachDelayer(math::quat<float>(0, 0, 0, 1), math::quat<float>(0, 0, 0, 1), 0),
      m_scaleToReachDelayer(math::vec3<float>(1, 1, 1), math::vec3<float>(1, 1, 1), 0), m_animationTimeToReach(-1), m_useMovementDummy(false),
      m_useMultiMaterial(false), m_lowUseMultiMaterial(false), m_currentRandomAnimIdx(0), m_alphaDelayer(1.0f, 1.0f, 0), m_triggeredAlphaFade(false),
      m_movieLinkedObject(0), m_sceneMgrId(GameLevel::SceneMgr_World), m_updateAabb(false), m_currentAabb(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1)),
      m_blinkingTimer(0), m_forceAnimation(false), m_ctorFlags(0) {
    InitEntity();
}

// the copy of a template Deco3d (@0x4d4544)
Deco3d::Deco3d(Deco3d *src, unsigned ctorFlags)
    : GameEntity(0), m_shadowModel(0), m_shadowEnabled(true), m_model(0), m_rotationToReachDelayer(math::quat<float>(0, 0, 0, 1), math::quat<float>(0, 0, 0, 1), 0),
      m_scaleToReachDelayer(math::vec3<float>(1, 1, 1), math::vec3<float>(1, 1, 1), 0), m_animationTimeToReach(-1), m_useMovementDummy(false),
      m_useMultiMaterial(false), m_lowUseMultiMaterial(false), m_currentRandomAnimIdx(0), m_alphaDelayer(1.0f, 1.0f, 0), m_triggeredAlphaFade(false),
      m_movieLinkedObject(0), m_sceneMgrId(GameLevel::SceneMgr_World), m_updateAabb(false), m_currentAabb(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1)),
      m_blinkingTimer(0), m_forceAnimation(false), m_ctorFlags(ctorFlags) {
    InitEntity();
    ConstructFrom(static_cast<const GameEntity*>(src));
}

void Deco3d::InitEntity() {
    m_type = EntityTypeDef::Deco3d;
    m_useMovementDummy = false;
    m_model = 0;
    m_useMultiMaterial = false;
    m_animationTimeToReach = -1;
    m_updateAabb = false;
    m_sceneMgrId = (m_ctorFlags & EGEConstructorFlagsDef::Interface) ? GameLevel::SceneMgr_Menu : GameLevel::SceneMgr_World;
}

Deco3d::~Deco3d() {
    if (m_model) jet::scene::ModelLoader::GetInstance()->Free(m_model);
    if (m_shadowModel) jet::scene::ModelLoader::GetInstance()->Free(m_shadowModel);
}

// CanLoadModel @0x4d4910
bool Deco3d::CanLoadModel() const {
    jet::String name = m_modelName;
    if (name.empty()) GetParam(k_model, name);
    if (CGameConfig::Current().useLowModels && HasParam(k_lowModel)) {
        jet::String low;
        GetParam(k_lowModel, low);
        if (!low.empty()) name = low;
    }
    return jet::scene::ModelLoader::GetInstance()->CanLoad(name);
}

jet::anim::Animation *Deco3d::GetAnimation() const { return m_model ? m_model->GetAnimation() : 0; }

// Init @0x4d4a18: pick the model (the low one on weak devices), load it, place it, start its animations
void Deco3d::Init() {
    GameEntity::Init();
    const PlatformToggles &tg = CGameConfig::Current();
    GetParam(k_model, m_modelName);
    bool lowModel = false;
    if (tg.useLowModels && HasParam(k_lowModel)) {
        jet::String low;
        GetParam(k_lowModel, low);
        if (!low.empty()) m_modelName = low;
    }
    if (tg.hideOptionalDecos && !tg.useLowModels && HasParam(k_lowModel)) {
        jet::String low;
        GetParam(k_lowModel, low);
        if (!low.empty() && strstr(low.c_str(), "iphone4") && strstr(low.c_str(), "concrete")) {
            m_modelName = low;
            lowModel = true;
        }
    }
    bool optionalHidden = false;
    if (tg.hideOptionalDecos && !m_modelName.Equals(k_alwaysLoadedModel) && HasParam(k_optional)) GetParam(k_optional, optionalHidden);
    if (m_modelName.empty()) {
        fprintf(stderr, "ERROR: invalid model name\n");
    } else if (!optionalHidden) {
        m_model = jet::scene::ModelLoader::GetInstance()->Load(m_modelName);
    }
    if (!m_model) {
        if (!optionalHidden) fprintf(stderr, "ERROR: could not load model, loading replacement model: %s\n", m_modelName.c_str());
        else fprintf(stderr, "WARNING: not load optional model %s\n", m_modelName.c_str());
        return;
    }
    m_currentAabb = m_model->ComputeAbsoluteAccurateBoundingBox();
    m_model->SetUpdateMode(2);
    m_model->GetRootNode().SetTransform(GetPosition(), GetRotation(), GetScale());

    if (const clara::Param *p = FindParamByName(k_animation)) {
        jet::String anim = p->GetAsEnumName(p->GetComponentCount() ? (unsigned)rand() % p->GetComponentCount() : 0);
        if (!anim.empty()) {
            m_updateAabb = true;
            SetAnimation(anim, false, AnimInterpolation::None);
        }
    }
    if (HasParam(k_texAnimation)) {
        const clara::Param *p = FindParamByName(k_texAnimation);
        jet::String tex = p->GetAsEnumName(p->GetComponentCount() ? (unsigned)rand() % p->GetComponentCount() : 0);
        if (!tex.empty()) {
            bool loop = true;
            if (HasParam(k_useTexAnimationLoop)) GetParam(k_useTexAnimationLoop, loop);
            SetTexAnimation(tex, loop);
        }
    }
    float speed = 100.0f;
    if (HasParam(k_animSpeed)) GetParam(k_animSpeed, speed);
    m_model->CC().SetSpeedFixed((int)std::floor(speed * 655.36f + 0.5f));
    bool loop = true;
    GetParam(k_useLoop, loop);
    m_model->CC().SetLooping(loop);
    m_useMultiMaterial = false;
    GetParam(k_useMultiMaterials, m_useMultiMaterial);
    if (tg.useLowModels && HasParam(k_lowModel)) {
        jet::String low;
        GetParam(k_lowModel, low);
        if (!low.empty()) lowModel = true;
    }
    if (lowModel) {
        if (HasParam(k_lowUseMultiMaterials)) GetParam(k_lowUseMultiMaterials, m_lowUseMultiMaterial);
        LoadMaterials(m_materialDefs, k_lowMaterials, m_lowUseMultiMaterial);
    } else {
        LoadMaterials(m_materialDefs, k_materials, m_useMultiMaterial);
    }
    if (HasParam(k_useMovementDummy)) GetParam(k_useMovementDummy, m_useMovementDummy);
    else m_useMovementDummy = false;
    if (tg.useShadows > 0) {
        jet::String sm;
        if (HasParam(k_shadowModel) && GetParam(k_shadowModel, sm) && !sm.empty()) {
            m_shadowModel = jet::scene::ModelLoader::GetInstance()->Load(sm);
            LoadMaterials(m_shadowMaterialDefs, k_shadowMaterial, false);
        }
    }
    m_forceAnimation = false;
}

// PostInit @0x4d52d4: materials, registering with the level, the random animation list
void Deco3d::PostInit() {
    GameEntity::PostInit();
    int group = 0;
    GetParam(k_generationGroup, group);
    m_generation.group = (unsigned)group;
    const PlatformToggles &tg = CGameConfig::Current();
    ApplyMaterials(m_model, m_materialDefs);
    CheckAlphaFadeability();
    bool hidden = false;
    if (tg.hideOptionalDecos && !m_modelName.Equals(k_alwaysLoadedModel) && HasParam(k_optional)) GetParam(k_optional, hidden);
    if (!hidden) {
        bool reg = !(m_ctorFlags & EGEConstructorFlagsDef::Interface) && !(m_ctorFlags & EGEConstructorFlagsDef::DontRegister);
        RegisterForUpdate(reg);
        RegisterForRender(reg);
    } else {
        SetEnabled(false);
        SetVisible(false);
    }
    if (m_model) m_model->Animate(true);
    if (HasParam(k_ChooseRandAnimOnAnimEnd)) {
        bool choose = false;
        GetParam(k_ChooseRandAnimOnAnimEnd, choose);
        const clara::Param *p = FindParamByName(k_animation);
        if (choose && p && p->GetComponentCount() > 1) {
            std::vector<jet::String> names;
            for (unsigned i = 0; i < p->GetComponentCount(); i++) names.push_back(p->GetAsEnumName(i));
            PlayRandomAnimations(names);
        }
    }
    if (m_shadowModel) {
        ApplyMaterials(m_shadowModel, m_shadowMaterialDefs);
        InitShadowNodes(m_model, m_shadowModel);
    }
}

void Deco3d::SetEnabled(bool on) {
    GameEntity::SetEnabled(on);
    if (on || m_animationTimeToReach < 0 || !m_model) return;
    if (m_animationTimeToReach != m_model->CC().GetTime()) m_model->CC().SetTimeFull(m_animationTimeToReach);
    m_model->Animate(false);
    m_animationTimeToReach = -1;
}

void Deco3d::SetVisible(bool on) {
    GameEntity::SetVisible(on);
    if (on) UpdateTransform(false);
}

bool Deco3d::GetLinkPointTransform(const jet::String &name, math::vec3<float> &pos, math::quat<float> &rot) const {
    if (name.empty()) return GameEntity::GetLinkPointTransform(name, pos, rot);
    if (!m_model) {
        pos = GetPosition();
        rot = GetRotation();
        return true;
    }
    if (const jet::scene::Node *n = m_model->FindNodeByName(name)) {
        pos = n->GetAbsolutePosition();
        rot = n->GetAbsoluteRotation();
        return true;
    }
    if (jet::scene::Dummy *d = m_model->FindDummyByName(name, 0)) {
        pos = d->GetAbsolutePosition();
        rot = d->GetAbsoluteRotation();
        return true;
    }
    return false;
}

void Deco3d::SetMaterial(const jet::String &name, MaterialDef *def) {
    for (size_t i = 0; i < m_materialDefs.size(); i++)
        if (m_materialDefs[i].first.Equals(name)) {
            m_materialDefs[i].second = def;
            ApplyMaterials(m_model, m_materialDefs);
            CheckAlphaFadeability();
            return;
        }
}

void Deco3d::SetAllMaterials(MaterialDef *def) {
    for (size_t i = 0; i < m_materialDefs.size(); i++) m_materialDefs[i].second = def;
    ApplyMaterials(m_model, m_materialDefs);
    CheckAlphaFadeability();
}

void Deco3d::CheckAlphaFadeability() {
    m_alphaScaleShaderUniforms.clear();
    GetUniformsByName(m_model, k_alphaScale, m_alphaScaleShaderUniforms);
}

void Deco3d::ApplyAlpha() {
    for (size_t i = 0; i < m_alphaScaleShaderUniforms.size(); i++) {
        const ShaderUniformRef &r = m_alphaScaleShaderUniforms[i];
        r.material->SetUniform(r.pass, (unsigned)r.uniform, jet::video::ShaderUniform(m_alphaDelayer.GetCurrentValue()));
    }
}

// Update @0x4d5910
void Deco3d::Update() {
    const int dt = s_frameDt;
    GameEntity::Update();
    if (m_blinkingTimer > 0) m_blinkingTimer -= dt;
    if (m_rotationToReachDelayer.IsUpdating()) {
        m_rotationToReachDelayer.Update(dt);
        SetRotation(m_rotationToReachDelayer.GetCurrentValue());
    }
    if (m_scaleToReachDelayer.IsUpdating()) {
        m_scaleToReachDelayer.Update(dt);
        SetScale(m_scaleToReachDelayer.GetCurrentValue());
    }
    // moves the animation toward a time, at most as fast as it would play
    if (m_animationTimeToReach >= 0 && m_model) {
        jet::scene::Model::Controller &cc = m_model->CC();
        int step = (int)(((float)cc.GetSpeedFixed() / 65536.0f) * (float)dt);
        int diff = m_animationTimeToReach - cc.GetTime();
        if (diff > step) diff = step;
        if (diff < -step) diff = -step;
        int t = cc.GetTime() + diff;
        cc.SetTimeFull(t);
        m_model->Animate(false);
        if (t == m_animationTimeToReach) m_animationTimeToReach = -1;
    }
    if (m_triggeredAlphaFade && m_alphaDelayer.IsUpdating()) {
        m_alphaDelayer.Update(dt);
        ApplyAlpha();
    }
    if (IsVisible() && m_model) {
        if (m_model->GetAnimation() || m_model->GetTextureAnimation()) m_model->Update(dt);
        if (!m_randomAnimations.empty() && m_model->CC().IsFinished()) {
            unsigned idx = (unsigned)rand() % (unsigned)m_randomAnimations.size();
            if (m_currentRandomAnimIdx == idx) idx = (idx ? idx : (unsigned)m_randomAnimations.size()) - 1;
            m_currentRandomAnimIdx = idx;
            SetAnimation(m_randomAnimations[idx], false, AnimInterpolation::IfPlaying);
        }
        if (m_model->GetAnimation()) UpdateLinkedEntities(LinkedToDef::LinkPoint);
        if (m_useMovementDummy) {
            math::vec3<float> delta = m_model->GetMovementDelta();
            m_model->ClearMovementDelta();
            const math::quat<float> &rot = IsLinked() ? GetLinkData()->rotation : GetRotation();
            math::vec3<float> d = rot * delta;
            if (d.getLength() > 0.0f) SetPosition((IsLinked() ? GetLinkData()->position : GetPosition()) + d);
        }
    }
}

// Render @0x4d5ff0: animate when needed, hand the model (and its shadow) to the level's scene manager
void Deco3d::Render() {
    if (m_blinkingTimer > 0 && !(m_blinkingTimer & 0x80)) return;
    if (!IsVisible() || !m_model) return;
    jet::scene::SceneMgr *sm = GameLevel::GetInstance()->GetSceneMgr(m_sceneMgrId);
    if (m_model->GetUpdateMode() == 2) {
        int last = m_model->GetLastAnimTime();
        if (m_model->GetAnimation() && m_model->CC().GetTime() != last) {
            bool skip = false;
            if (m_sceneMgrId != GameLevel::SceneMgr_Menu) {
                const math::vec3<float> &cam = GameLevel::GetInstance()->GetCamera()->GetPosition();
                float r = CGameConfig::Current().decosAnimationRadius;
                skip = r * r < math::distanceSq(GetPosition(), cam) && !m_forceAnimation;
            }
            if (!skip) {
                m_model->Animate(false);
                GameLevel::GetInstance()->AddAnimatedEntity(this, m_model->GetAnimation() ? m_animationName : jet::String(), (int)m_model->GetNodeCount());
            }
        }
        if (m_model->GetAnimation()) UpdateLinkedEntities(LinkedToDef::LinkPoint);
    }
    sm->Add(m_model);
    if (m_shadowEnabled && m_shadowModel) {
        if (m_shadowModel != m_model) UpdateShadowNodes();
        sm->Add(GameLevel::GetShadowTagName(), m_shadowModel);
    }
}

float Deco3d::GetFocusRadius() const { return m_model ? m_model->GetBoundingSphereRadius() : 0.0f; }

// SetAnimation @0x4d620c: the interpolation says whether the new animation blends from the old pose
void Deco3d::SetAnimation(const jet::String &name, bool loop, AnimInterpolation::Enum interp) {
    if (!m_model) {
        fprintf(stderr, "SetAnimation m_model NULL\n");
        return;
    }
    jet::scene::Model::Controller &cc = m_model->CC();
    cc.SetLooping(loop);
    if (m_animationName.Equals(name)) {
        cc.SetTime(0);
        return;
    }
    unsigned char saved = m_model->AnimFlags();
    m_animationName = name;
    unsigned char &f = m_model->AnimFlags();
    if (interp == AnimInterpolation::Always) f |= 0x80;
    else if (interp == AnimInterpolation::IfPlaying) f = cc.IsActive() ? (f | 0x80) : (f & 0x7f);
    else f &= 0x7f;
    jet::anim::Animation *a = jet::anim::AnimationLoader::GetInstance()->Load(name);
    m_model->SetAnimation(a, 100);
    m_model->AnimFlags() = (unsigned char)((m_model->AnimFlags() & 0x7f) | (saved & 0x80));
}

void Deco3d::SetTexAnimation(const jet::String &name, bool loop) {
    if (!m_texAnimationName.Equals(name)) {
        m_texAnimationName = name;
        m_model->SetTextureAnimation(jet::anim::AnimationLoader::GetInstance()->Load(name));
        m_model->CTC().SetLooping(loop);
        return;
    }
    m_model->CTC().SetTime(0);
}

void Deco3d::PlayRandomAnimations(const std::vector<jet::String> &anims) {
    if (anims.empty() || m_randomAnimations.empty() == false) {
        if (anims.empty()) fprintf(stderr, "PlayRandomAnimations anims NULL\n");
    }
    if (anims.empty()) return;
    m_randomAnimations = anims;
    SetAnimation(m_randomAnimations[0], false, AnimInterpolation::IfPlaying);
}

void Deco3d::SetAnimationSpeed(float s) {
    s = s < 0.1f ? 0.1f : (s > 10.0f ? 10.0f : s);
    m_model->CC().SetSpeedFixed((int)std::floor(s * 65536.0f + 0.5f));
}

// SetAnimationTime @0x4d6480: jump to a time; the pose is applied at once without blending
void Deco3d::SetAnimationTime(unsigned ms) {
    jet::scene::Model::Controller &cc = m_model->CC();
    unsigned end = (unsigned)cc.GetEnd();
    if (ms > end) ms = end;
    if (m_updateAabb) cc.SetTimeFull((int)ms);
    else cc.SetTime((int)ms);
    unsigned char saved = m_model->AnimFlags();
    m_model->InterpolationTotal() = 0;
    m_model->AnimFlags() = 0;
    m_model->Animate(true);
    m_model->AnimFlags() = saved & 0x7f;
    if (!(saved & 0x7f)) m_model->InterpolationTotal() = 0;
}

int Deco3d::GetAnimTime() const { return m_model->CC().GetTime(); }
int Deco3d::GetAnimOldTime() const { return m_model->CC().GetPrevTime(); }

void Deco3d::SetRotationToReach(const math::quat<float> &q, unsigned ms) {
    m_rotationToReachDelayer.SetDelay((int)ms);
    m_rotationToReachDelayer.ResetValues(GetRotation(), q);
}

void Deco3d::SetScaleToReach(const math::vec3<float> &s, unsigned ms) {
    m_scaleToReachDelayer.SetDelay((int)ms);
    m_scaleToReachDelayer.ResetValues(GetScale(), s);
}

// TriggerAlphaFade @0x4d6518
void Deco3d::TriggerAlphaFade(float alpha, int ms) {
    m_alphaDelayer.SetDelay(ms);
    m_alphaDelayer.SetTargetValue(alpha);
    if (ms < 1) {
        m_alphaDelayer.ResetValues(alpha);
        ApplyAlpha();
    }
    m_triggeredAlphaFade = true;
}

// NotifyAnimationChanged @0x4d67f0: the clara entity's animation (set by its movie or the editor) drives the model
void Deco3d::NotifyAnimationChanged() {
    if (!m_model) return;
    const bool movie = IsControlledByMovie();
    jet::scene::Model::Controller &cc = m_model->CC();
    const AnimationParams &params = GetAnimationParams();
    cc.SetLooping(params.loop);
    const jet::String &name = movie ? GetAnimationName() : m_animationName;
    jet::anim::Animation *cur = m_model->GetAnimation();
    unsigned char saved = m_model->AnimFlags();
    if (!cur || !name.Equals(cur->GetResourceName())) {
        if (!movie) m_model->AnimFlags() |= 0x80;
        else m_model->AnimFlags() &= 0x7f;
        m_model->SetAnimation(jet::anim::AnimationLoader::GetInstance()->Load(name), 100);
        m_model->AnimFlags() = (unsigned char)((m_model->AnimFlags() & 0x7f) | (saved & 0x80));
    }
    bool active = !movie;
    if (cc.IsActive() != active) cc.SetActive(active);
    if (movie) cc.SetTime((int)params.time);
}

void Deco3d::NotifyTransformChanged(bool b) {
    GameEntity::NotifyTransformChanged(b);
    if (m_model) m_model->GetRootNode().SetTransform(GetPosition(), GetRotation(), GetScale());
}

// SetModelNodesVisible @0x4d6980: the meshes of the nodes whose name starts with the prefix
void Deco3d::SetModelNodesVisible(const jet::String &prefix, bool visible) {
    if (!m_model) return;
    for (unsigned i = 0; i < m_model->GetNodeCount(); i++) {
        const jet::String &n = m_model->GetNode(i).GetName();
        if (strncasecmp(n.c_str(), prefix.c_str(), prefix.size()) != 0) continue;
        for (unsigned m = 0; m < m_model->GetMultiResMeshCount(); m++)
            if (m_model->GetMultiResMesh(m).GetNode() && m_model->GetMultiResMesh(m).GetNode()->GetName().Equals(n)) m_model->GetMultiResMesh(m).SetVisible(visible);
    }
}

// InitShadowNodes @0x4d5ea0: the shadow model's nodes follow the nodes of the same name of the model
void Deco3d::InitShadowNodes(jet::scene::Model *model, jet::scene::Model *shadow) {
    m_shadowAnimatedNodes.clear();
    if (!model || !shadow || model == shadow) return;
    for (unsigned i = 0; i < shadow->GetNodeCount(); i++) {
        jet::scene::Node &sn = shadow->GetMutableNode(i);
        const jet::scene::Node *mn = model->FindNodeByName(sn.GetName());
        if (mn) {
            ShadowAnimatedNodes a = { &sn, const_cast<jet::scene::Node*>(mn) };
            m_shadowAnimatedNodes.push_back(a);
        }
    }
}

void Deco3d::UpdateShadowNodes() {
    jet::scene::Node &root = m_model->GetRootNode();
    m_shadowModel->GetRootNode().SetTransform(root.GetPosition(), root.GetRotation(), root.GetScale());
    for (size_t i = 0; i < m_shadowAnimatedNodes.size(); i++)
        m_shadowAnimatedNodes[i].shadowNode->SetTransform(m_shadowAnimatedNodes[i].modelNode->GetPosition(), m_shadowAnimatedNodes[i].modelNode->GetRotation());
}

void Deco3d::FillModelData(TModelData &d) {
    GetParam(k_model, d.model);
    if (const clara::Param *p = FindParamByName(k_animation))
        d.animation = p->GetAsEnumName(p->GetComponentCount() ? (unsigned)rand() % p->GetComponentCount() : 0);
    GetParam(k_useMultiMaterials, d.useMultiMaterials);
    LoadMaterials(d.materials, k_materials, d.useMultiMaterials);
    d.loop = true;
    GetParam(k_useLoop, d.loop);
}

// ApplyModelData @0x4d6b80: swap the model, animation and materials for the ones in the data
void Deco3d::ApplyModelData(const TModelData &d) {
    if (!d.model.Equals(m_modelName)) {
        if (m_model) jet::scene::ModelLoader::GetInstance()->Free(m_model);
        m_modelName = d.model;
        m_model = jet::scene::ModelLoader::GetInstance()->Load(m_modelName);
        SetAnimation(d.animation, d.loop, AnimInterpolation::IfPlaying);
        m_useMultiMaterial = d.useMultiMaterials;
        m_materialDefs = d.materials;
        ApplyMaterials(m_model, m_materialDefs);
        CheckAlphaFadeability();
    }
}
