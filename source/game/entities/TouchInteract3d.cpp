// source/game/entities/TouchInteract3d.cpp
#include "TouchInteract3d.h"
#include "Deco3d.h"
#include "Minion.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_MainMenu.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "game/dynamicobjects/EffectDef.h"
#include "sound/soundmgr.h"
#include "scene/model.h"
#include "anim/animation.h"
#include <cstdio>
#include <cstdlib>

jet::String TouchInteract3d::k_tmplName("touchInteract3d");

TouchInteract3d::TouchInteract3d(const clara::Template *t)
    : GameEntity(t), m_deco(0), m_onTouchEffect(0), m_despicablePoints(0), m_decoScale(0, 0, 0), m_touchEvent(0), m_effectLayerId(0), m_ctorFlags(0) {
    m_type = EntityTypeDef::TouchInteract3d;
}

TouchInteract3d::TouchInteract3d(TouchInteract3d *src, unsigned ctorFlags)
    : GameEntity(0), m_deco(0), m_onTouchEffect(0), m_despicablePoints(0), m_decoScale(0, 0, 0), m_touchEvent(0), m_effectLayerId(0), m_ctorFlags(ctorFlags) {
    m_type = EntityTypeDef::TouchInteract3d;
    ConstructFrom(static_cast<const GameEntity*>(src));
}

TouchInteract3d::~TouchInteract3d() {
    delete m_deco;
}

// LoadEnumStringArray @0x4ba4e0
void TouchInteract3d::LoadEnumStringArray(const jet::String &param, std::vector<jet::String> &out) const {
    out.clear();
    if (const clara::Param *p = FindParamByName(param))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) out.push_back(p->GetAsEnumName(i));
}

// Init @0x4b95f0
void TouchInteract3d::Init() {
    GameEntity::Init();
    BPO_LoadBodyPartsArray(BPO_GetDefinitions(), FindParamByName(jet::String("bodyParts")));
    LoadEnumStringArray(jet::String("IntroAnimation"), m_introAnims);
    LoadEnumStringArray(jet::String("IdleAnimation"), m_idleAnims);
    LoadEnumStringArray(jet::String("AutoHideAnimation"), m_autoHideAnims);
    GetParam(jet::String("IntroAnimationSound"), m_introSound);
    clara::Path effect;
    if (const clara::Param *p = FindParamByName(jet::String("OnTouchEffect"))) effect = p->GetAsPath(0);
    m_onTouchEffect = clara::Project::GetInstance()->FindEntityByPath(effect);
    GetParam(jet::String("despicablePoints"), m_despicablePoints);
    RandomizeAnims();
}

// PostInit @0x4b97dc
void TouchInteract3d::PostInit() {
    GameEntity::PostInit();
    InitDeco3d();
    BPO_Init(m_deco->GetModel());
    BPO_Enable(true);
    BPO_AddBodyPartsArray(BPO_GetDefinitions());
    bool reg = !(m_ctorFlags & EGEConstructorFlagsDef::Interface) && !(m_ctorFlags & EGEConstructorFlagsDef::DontRegister);
    RegisterForUpdate(reg);
    RegisterForRender(reg);
    m_effectLayerId = (m_ctorFlags & EGEConstructorFlagsDef::Interface) ? ++s_nextEffectLayer : 0;
}

unsigned TouchInteract3d::s_nextEffectLayer = 0;

// InitDeco3d @0x4b986c: a copy of the objectDeco linked to this entity, scaled by both scales
void TouchInteract3d::InitDeco3d() {
    const clara::Param *p = FindParamByName(jet::String("objectDeco"));
    clara::Entity *tmpl = clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)) ? dynamic_cast<clara::Entity*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0))) : 0;
    Deco3d *tdeco = dynamic_cast<Deco3d*>(tmpl);
    m_deco = new Deco3d(tdeco, m_ctorFlags);
    m_deco->LinkTo(this, jet::String(), 0);
    m_deco->GetModel()->AnimFlags() = 6;
    UpdateTransform(false);
    m_deco->SetPosition(tmpl->GetPosition());
    m_deco->SetRotation(tmpl->GetRotation());
    const math::vec3<float> &a = GetScale(), &b = tmpl->GetScale();
    m_deco->SetScale(math::vec3<float>(a.x * b.x, a.y * b.y, a.z * b.z));
    m_decoScale = m_deco->GetScale();
    SetDecoAnimation(m_introAnim, false, false);
    m_deco->RegisterForUpdate(false);
    m_deco->RegisterForRender(false);
    m_deco->SetVisible(false);
    m_deco->SetEnabled(false);
}

// Reset @0x4b9a8c
void TouchInteract3d::Reset(bool on) {
    m_deco->SetVisible(on);
    m_deco->SetEnabled(on);
    m_touchedAnim = jet::String();
    if (m_introAnim.empty() && !m_idleAnim.empty()) {
        SetDecoAnimation(m_idleAnim, true, true);
        return;
    }
    SetDecoAnimation(m_introAnim, false, true);
}

void TouchInteract3d::AutoHide() {
    if (!m_touchedAnim.empty()) return;
    SetDecoAnimation(m_autoHideAnim, false, false);
    m_touchedAnim = m_autoHideAnim;
    m_touchEvent = TouchInteractEvents::Hide;
}

// Update @0x4b9bb0
void TouchInteract3d::Update() {
    if (m_deco->IsEnabled()) m_deco->Update();
    GameEntity::Update();
    BPO_Update(s_frameDt);
    if (IsDecoAnimationFinished(m_introAnim)) SetDecoAnimation(m_idleAnim, true, false);
    if (IsDecoAnimationFinished(m_touchedAnim)) {
        ExecuteTouchEvent(m_touchEvent, false);
        m_touchedAnim = jet::String();
    }
}

// Render @0x4b9c7c
void TouchInteract3d::Render() {
    if (m_deco->IsVisible()) m_deco->Render();
    GameEntity::Render();
    BPO_Render();
    jet::scene::Model *m = m_deco->GetModel();
    if (m->GetAnimation()) m->AnimateNodes(false);
}

bool TouchInteract3d::IsDecoAnimationFinished(const jet::String &anim) {
    if (anim.empty()) return false;
    jet::scene::Model *m = m_deco->GetModel();
    if (m->GetAnimation() && !anim.Equals(m->GetAnimation()->GetResourceName())) return false;
    return m->CC().IsFinished();
}

// SetDecoAnimation @0x4b9d24: the menu animations are windows of the same files: the loop runs between the
// start and end time of the one named
void TouchInteract3d::SetDecoAnimation(const jet::String &anim, bool loop, bool force) {
    int start = 0, end = 0;
    if (anim.Equals(m_introAnim)) { start = 0; end = 500; }
    else if (anim.Equals(m_idleAnim)) { start = 0x535; end = 0x11d6; }
    else if (anim.Equals("ui_mm_tapplay_tap.anim")) { start = 5000; end = 0x16a8; }
    else if (anim.Equals("ui_mm_tapplay_out.anim")) { start = 0x1964; end = 0x1af4; }
    jet::scene::Model *m = m_deco->GetModel();
    if (!force) {
        if (anim.empty()) return;
        if (m->GetAnimation() && anim.Equals(m->GetAnimation()->GetResourceName())) return;
    }
    m_deco->SetAnimation(anim, loop, Deco3d::AnimInterpolation::IfPlaying);
    m_deco->SetTexAnimation(anim, loop);
    if (start || end) {
        m->CC().SetLoop(start, end);
        m->CTC().SetLoop(start, end);
        m->CC().SetTime(start);
        m->CTC().SetTime(start);
    }
    m->CC().SetFinished(false);
    m->CTC().SetFinished(false);
    RandomizeAnims();
}

// ExecuteTouchEvent @0x4b9ee4: events 1..3 are the main menu's to handle, 5 hides the element
void TouchInteract3d::ExecuteTouchEvent(int event, bool outro) {
    if (event < 1) return;
    if (event < 4) {
        MenuMgr *mm = MenuMgr::GetInstance();
        Menu_Base *menu = mm->GetMenu(Menu_MainMenu::k_menuName);
        if (mm->GetCurrentPlayingMenu() == menu) {
            if (!outro) static_cast<Menu_MainMenu*>(menu)->ExecuteTouchInteractEvent(event);
            else static_cast<Menu_MainMenu*>(menu)->TouchInteract_StartingOutroAnim(event);
        }
        return;
    }
    if (event != TouchInteractEvents::Hide) return;
    if (outro) {
        SetVisible(false);
        SetEnabled(false);
    }
}

// BPO_OnBodyPartTouched @0x524c24: the first touch plays the click and the part's sound, starts its animation and
// fires its event; every touch starts the effect of the touch at the part (EffectMgr::Add, its place and rotation are
// the node's, the part's offset and rotation and, for a part with a height, the middle of its extent). The despicable
// points of the touch go to the minion at the place of the object (Minion::IncreaseDespicablePoints @0x57a398, the ones of m_despicablePoints).
void TouchInteract3d::BPO_OnBodyPartTouched(BodyPartSection *s) {
    if (m_touchedAnim.empty()) {
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
        if (!s->def->touchedSound.empty()) SoundMgr::GetInstance()->Play(s->def->touchedSound, 0);
        SetDecoAnimation(s->def->touchedAnimation, false, false);
        m_touchedAnim = s->def->touchedAnimation;
        m_touchEvent = s->def->touchEvent;
        ExecuteTouchEvent(m_touchEvent, true);
    }
    if (m_onTouchEffect && s) {
        const BodyPartDefinition *def = s->def;
        jet::scene::Node *node = s->node;
        const math::quat<float> nodeRotation = node->GetAbsoluteRotation();
        const math::vec3<float> nodePosition = node->GetAbsolutePosition();
        const math::quat<float> rotation = nodeRotation * def->rotation;
        math::vec3<float> position = nodePosition + nodeRotation * def->position;
        if (def->height != 0) {
            const math::vec3<float> end = position + rotation * math::vec3<float>(0, 0, def->height);
            position = math::lerp(position, end, 0.5f);
        }
        EffectMgr::GetInstance()->Add(m_effectLayerId, reinterpret_cast<const EffectDef*>(m_onTouchEffect), position, rotation, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
    }
    if (m_despicablePoints > 0)
        if (GameLevel *level = GameLevel::GetInstance())
            if (Minion *minion = level->GetMinion()) minion->IncreaseDespicablePoints(m_despicablePoints, GetPosition());
}

void TouchInteract3d::BPO_OnBodyPartPressed(BodyPartSection *s) {
    if (!m_touchedAnim.empty()) return;
    SetDecoAnimation(s->def->pressedAnimation, true, false);
}

void TouchInteract3d::BPO_OnBodyPartReleased(BodyPartSection*) {
    if (!m_touchedAnim.empty()) return;
    SetDecoAnimation(m_idleAnim, true, false);
}

void TouchInteract3d::SetVisible(bool on) {
    GameEntity::SetVisible(on);
    m_deco->SetVisible(on);
    BPO_Enable(IsEnabled() && IsVisible());
}

void TouchInteract3d::SetEnabled(bool on) {
    GameEntity::SetEnabled(on);
    m_deco->SetEnabled(on);
    BPO_Enable(IsEnabled() && IsVisible());
}

void TouchInteract3d::NotifyTransformChanged(bool b) {
    GameEntity::NotifyTransformChanged(b);
}

// RandomizeAnims @0x4ba538
void TouchInteract3d::RandomizeAnims() {
    if (!m_idleAnims.empty()) m_idleAnim = m_idleAnims[(unsigned)rand() % m_idleAnims.size()];
    if (!m_introAnims.empty()) m_introAnim = m_introAnims[(unsigned)rand() % m_introAnims.size()];
    if (!m_autoHideAnims.empty()) m_autoHideAnim = m_autoHideAnims[(unsigned)rand() % m_autoHideAnims.size()];
}

void TouchInteract3d::ScaleUIDecoAndBP(float f) {
    m_deco->SetScale(math::vec3<float>(f * m_decoScale.x, f * m_decoScale.y, f * m_decoScale.z));
    BPO_Scale(f);
}

void TouchInteract3d::SetSceneMgr(GameLevel::SceneMgrId id) {
    m_deco->SetSceneMgr(id);
    if (id == GameLevel::SceneMgr_Menu) {
        m_deco->RegisterForUpdate(false);
        m_deco->RegisterForRender(false);
    }
    BPO_SetSceneMgr((int)id);
}

void TouchInteract3d::SetModelNodesVisible(const jet::String &prefix, bool visible) { m_deco->SetModelNodesVisible(prefix, visible); }

void TouchInteract3d::SetBPSVisibleIfItHas(int event, bool visible) {
    for (unsigned i = 0; i < BPO_GetBodyPartCount(); i++) {
        BodyPartDefinition *d = BPO_GetBodyPart(i)->def;
        if (d->touchEvent == event) {
            BPO_SetBodyPartEnabled(d, visible);
            BPO_SetBodyPartVisible(d, visible, false, false);
        }
    }
}
