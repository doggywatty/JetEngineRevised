// source/game/entities/InteractiveObjectTemplate.cpp
#include "InteractiveObjectTemplate.h"
#include "Deco3d.h"
#include "MaterialDef.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../Statistics.h"
#include "../gamemanagers/BackgroundMgr.h"
#include "core/util.h"
#include "game/dynamicobjects/EffectControl.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "param.h"
#include "project.h"
#include "scene/model.h"

jet::String InteractiveObjectTemplate::k_tmplName("InteractiveObjectTemplate"),
    InteractiveObjectTemplate::k_superflousInteractive_tmplName("superfluousInteractiveObjectTemplate");
const jet::String InteractiveObjectTemplate::k_animOnContact("animOnContact"), InteractiveObjectTemplate::k_useEngineEventOnContact("useEngineEventOnContact"),
    InteractiveObjectTemplate::k_engineEventOnContact("engineEventOnContact"), InteractiveObjectTemplate::k_despicablePoints("despicablePoints"),
    InteractiveObjectTemplate::k_checkLaneOnContact("checkLaneOnContact"), InteractiveObjectTemplate::k_stateOnContact("stateOnContact"),
    InteractiveObjectTemplate::k_hideOnContact("hideOnContact"), InteractiveObjectTemplate::k_forceSpawnIt("forceSpawnIt"),
    InteractiveObjectTemplate::k_enablePreContactAnimations("enablePreContactAnimations"), InteractiveObjectTemplate::k_stateOnPreContact("stateOnPreContact"),
    InteractiveObjectTemplate::k_preContactTime("preContactTime"), InteractiveObjectTemplate::k_preContactEffectTime("preContactEffectTime"),
    InteractiveObjectTemplate::k_preContactProjectileTime("preContactProjectileTime"),
    InteractiveObjectTemplate::k_preContactProjectileEffect("effectPreContactProjectile"),
    InteractiveObjectTemplate::k_PreContactImpactEffect("effectPreContactImpact"), InteractiveObjectTemplate::k_preContactObstacleEffect("effectPreContactObstacle"),
    InteractiveObjectTemplate::k_animOnFreeze("animOnFreeze"), InteractiveObjectTemplate::k_effectOnFreeze("effectOnFreeze"),
    InteractiveObjectTemplate::k_hideOnFreeze("hideOnFreeze"), InteractiveObjectTemplate::k_useDistanceTrigger("useDistanceTrigger"),
    InteractiveObjectTemplate::k_distanceTrigger("distanceTrigger"), InteractiveObjectTemplate::k_effectOnDistanceTrigger("effectOnDistanceTrigger"),
    InteractiveObjectTemplate::k_hideOnDistanceTrigger("hideOnDistanceTrigger"),
    InteractiveObjectTemplate::k_despicablePointsOnDistanceTrigger("despicablePointsOnDistanceTrigger"),
    InteractiveObjectTemplate::k_usePostFxDistortion("usePostFxDistortion"), InteractiveObjectTemplate::k_postFxMaterial("PostFxMaterial"),
    InteractiveObjectTemplate::k_postFxMaterialLow("PostFxMaterialLow"), InteractiveObjectTemplate::k_postFxTimeIntro("PostFxTimeIntro"),
    InteractiveObjectTemplate::k_postFxTime("PostFxTime"), InteractiveObjectTemplate::k_postFxTimeOutro("PostFxTimeOutro"),
    InteractiveObjectTemplate::k_postFxUniformMaxValue("PostFxUniformMaxValue"), InteractiveObjectTemplate::k_playIdleSoundParam("playIdleSound"),
    InteractiveObjectTemplate::k_playIdleSoundDistanceParam("playDistance"), InteractiveObjectTemplate::k_idleSoundLabelParam("idleSound");

// the entity a path parameter of the entity names (null when it has none)
static clara::DataEntity *EntityOfParam(const clara::DataEntity *e, const jet::String &name) {
    const clara::Param *p = e->FindParamByName(name);
    return p ? clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)) : 0;
}

// @0x56c9fc
InteractiveObjectTemplate::InteractiveObjectTemplate(const clara::Template *t)
    : Object(t), m_animOnContact(""), m_useEngineEvent(false), m_engineEvent(-1), m_despicablePoints(0), m_checkLaneOnContact(false), m_hideOnContact(false),
      m_useStateMachine(false), m_preContactState(PreContactAnimState::None), m_preContactTime(0), m_preContactProjectileEffect(0), m_preContactImpactEffect(0),
      m_preContactObstacleEffect(0), m_preContactEffectTime(0), m_preContactProjectileTime(0), m_preContactProjectileTimeCount(0), m_projectileEffectControl(0),
      m_useDistanceTrigger(false), m_distanceTriggerSq(0), m_hideOnDistanceTrigger(false), m_despicablePointsOnDistanceTrigger(0), m_postFxMaterial(0),
      m_postFxTimeIntro(0), m_postFxTime(0), m_postFxTimeOutro(0), m_postFxMaxValue(0), m_postFxState(PostFxAnimState::None), m_playIdleSound(false),
      m_playIdleSoundDistanceSq(0), m_idleSoundHandle(-1), m_hideOnFreeze(false), m_frozen(false), m_forzeSpawnIt(false) {
    m_type = 0xf;
    for (EffectControl *&c : m_effectControls) c = 0;
    if (GameLevel::GetInstance()) GameLevel::GetInstance()->AddInteractiveObject(this);
}

// @0x56cb94: the sound stops, the level forgets it and what played goes
InteractiveObjectTemplate::~InteractiveObjectTemplate() {
    StopIdleSound();
    if (GameLevel::GetInstance()) GameLevel::GetInstance()->RemoveInteractiveObject(this);
    ReleaseEffects();
}

// @0x56cd0c: the effects started for the object stop: the projectile's and the three at 0x210 are released by the EffectMgr
void InteractiveObjectTemplate::ReleaseEffects() {
    if (EffectMgr *effects = EffectMgr::GetInstance()) {
        effects->Release(m_projectileEffectControl);
        for (EffectControl *c : m_effectControls) effects->Release(c);
    }
    m_projectileEffectControl = 0;
    for (EffectControl *&c : m_effectControls) c = 0;
}

// @0x56cd80
void InteractiveObjectTemplate::Init() {
    if (IsLibraryEntity()) return;
    Object::Init();
}

// @0x56cd90: what the data says; a copy then makes its deco (Object::PostInit), the state machines and the synchronization
void InteractiveObjectTemplate::PostInit() {
    GetParam(k_despicablePoints, m_despicablePoints);
    GetParam(k_forceSpawnIt, m_forzeSpawnIt);
    if (FindParamByName(k_useDistanceTrigger)) {
        GetParam(k_useDistanceTrigger, m_useDistanceTrigger);
        if (m_useDistanceTrigger) GetParam(k_despicablePointsOnDistanceTrigger, m_despicablePointsOnDistanceTrigger);
    }
    if (IsLibraryEntity()) return;
    Object::PostInit();
    GetParam(k_animOnContact, m_animOnContact);
    GetParam(k_useEngineEventOnContact, m_useEngineEvent);
    if (HasParam(k_checkLaneOnContact)) GetParam(k_checkLaneOnContact, m_checkLaneOnContact);
    if (m_useEngineEvent) GetParam(k_engineEventOnContact, m_engineEvent);
    GetParam(k_hideOnContact, m_hideOnContact);
    // the post effect that distorts the screen: its material (the low one on a device that does without post effects) and its times
    if (FindParamByName(k_usePostFxDistortion)) {
        bool usePostFx = false;
        GetParam(k_usePostFxDistortion, usePostFx);
        if (usePostFx) {
            const jet::String &materialParam = MaterialDef::s_postFxEnabled ? k_postFxMaterial : k_postFxMaterialLow;
            if (const clara::Param *p = FindParamByName(materialParam)) {
                m_postFxMaterial = static_cast<MaterialDef*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)));
                GetParam(k_postFxTimeIntro, m_postFxTimeIntro);
                GetParam(k_postFxTime, m_postFxTime);
                GetParam(k_postFxTimeOutro, m_postFxTimeOutro);
                GetParam(k_postFxUniformMaxValue, m_postFxMaxValue);
                SetPostFxState(PostFxAnimState::None);
            }
        }
    }
    // what happens when the minion comes near (the distance is kept squared)
    m_useDistanceTrigger = false;
    if (FindParamByName(k_useDistanceTrigger)) {
        GetParam(k_useDistanceTrigger, m_useDistanceTrigger);
        if (m_useDistanceTrigger) {
            GetParam(k_distanceTrigger, m_distanceTriggerSq);
            m_distanceTriggerSq = m_distanceTriggerSq * m_distanceTriggerSq;
            GetParam(k_hideOnDistanceTrigger, m_hideOnDistanceTrigger);
            if (const clara::Param *p = FindParamByName(k_effectOnDistanceTrigger))
                for (unsigned i = 0; i < p->GetComponentCount(); i++)
                    m_effectOnDistanceTrigger.push_back(reinterpret_cast<EffectDef*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(i))));
        }
    }
    // what happens when it is frozen: one of the animations, the effects and whether it hides
    if (HasParam(k_animOnFreeze)) {
        if (const clara::Param *p = FindParamByName(k_animOnFreeze))
            for (unsigned i = 0; i < p->GetComponentCount(); i++) {
                const jet::String &name = p->GetAsEnumName(i);
                if (!name.empty()) m_animOnFreeze.push_back(name);
            }
        if (const clara::Param *p = FindParamByName(k_effectOnFreeze))
            for (unsigned i = 0; i < p->GetComponentCount(); i++)
                if (clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(i)))
                    m_effectOnFreeze.push_back(reinterpret_cast<const EffectDef*>(e));
        GetParam(k_hideOnFreeze, m_hideOnFreeze);
    }
    if (FindParamByName(k_playIdleSoundParam)) {
        GetParam(k_playIdleSoundParam, m_playIdleSound);
        if (m_playIdleSound) {
            GetParam(k_playIdleSoundDistanceParam, m_playIdleSoundDistanceSq);
            m_playIdleSoundDistanceSq = m_playIdleSoundDistanceSq * m_playIdleSoundDistanceSq;
            GetParam(k_idleSoundLabelParam, m_idleSoundLabel);
        }
    }
    InitStateMachine();
    InitPreContactAnims();
    SynchronizedObject::Init(this);
    RegisterForUpdate(true);
}

// @0x56d2f4: the pre contact animations: the object waits for the minion to be near enough in time, then plays its state, then the
// projectile flies from the deco's place to the obstacle (its effect follows the deco, or the link point when the object is linked to a
// deco, as the time goes); when it has arrived the impact starts and the obstacle's effect is kept until the object is released
void InteractiveObjectTemplate::UpdatePreContactAnims(unsigned dt) {
    EffectMgr *effects = EffectMgr::GetInstance();
    const int layer = Object::GetEffectLayerId();
    switch (m_preContactState) {
    case PreContactAnimState::WaitMinion: {
        Pacesetter *pace = GameLevel::GetInstance()->GetPacesetter();
        if (!pace) return;
        Minion *minion = GameLevel::GetInstance()->GetMinion();
        const float reach = pace->GetTargetSpeed() * 0.001f * (float)(m_preContactTime + 200);
        if (!minion || math::distance(minion->GetPosition(), GetPosition()) > reach) return;
        int t = (int)ComputeTimeToGetReached();
        if (t < 0) t = -t;
        if (t == 0 || t > m_preContactTime) return;
        SM_SetState(m_stateOnPreContact);
        m_preContactState = PreContactAnimState::WaitEffect;
        return;
    }
    case PreContactAnimState::WaitEffect:
        m_preContactEffectTime -= (int)dt;
        if (m_preContactEffectTime > 0) return;
        m_preContactState = PreContactAnimState::Projectile;
        // the projectile starts at the deco's place and rotation; the object releases it (autoRemove off)
        if (m_deco && effects && effects->Add(layer, m_preContactProjectileEffect, m_deco->GetPosition(), m_deco->GetRotation(), math::vec3<float>(1, 1, 1), false, 0, -1, true, 0))
            m_projectileEffectControl = effects->GetLastAdded();
        return;
    case PreContactAnimState::Projectile: {
        if (!m_deco || !effects) return;
        // where the projectile comes from: the deco's place, or the link point when the object is linked to a deco
        math::vec3<float> start = m_deco->GetPosition();
        math::quat<float> startRotation = m_deco->GetRotation();
        if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) GetLinkee()->GetLinkPointTransform(GetLinkPointName(), start, startRotation);
        m_preContactProjectileTimeCount += (int)dt;
        if (m_preContactProjectileTimeCount < m_preContactProjectileTime) {
            if (!m_projectileEffectControl) return;
            const float t = (float)m_preContactProjectileTimeCount / (float)m_preContactProjectileTime;
            effects->SetPosition(m_projectileEffectControl, math::lerp(m_deco->GetPosition(), start, t));
            effects->SetRotation(m_projectileEffectControl, math::lerp(m_deco->GetRotation(), startRotation, t));
            return;
        }
        effects->Release(m_projectileEffectControl);
        m_projectileEffectControl = 0;
        effects->Add(layer, m_preContactImpactEffect, start, startRotation, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
        if (effects->Add(layer, m_preContactObstacleEffect, start, startRotation, math::vec3<float>(1, 1, 1), false, 0, -1, true, 0))
            m_effectControls[2] = effects->GetLastAdded();
        m_preContactState = PreContactAnimState::Done;
        return;
    }
    default:
        return;
    }
}

// @0x56d6f8: when the minion is near enough the effects start and the object gives its points (once), and its idle sound starts (once)
void InteractiveObjectTemplate::UpateDistanceTriggers() {
    if (!m_useDistanceTrigger && !m_playIdleSound) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion || !GameLevel::GetInstance()->GetPacesetter()) return;
    const math::vec3<float> minionPosition = minion->GetPosition();
    const math::vec3<float> position = m_deco ? m_deco->GetPosition() : GetPosition();
    const float distanceSq = math::distanceSq(position, minionPosition);
    if (m_useDistanceTrigger && distanceSq <= m_distanceTriggerSq) {
        // EffectMgr::Add of every effect of m_effectOnDistanceTrigger at the deco's place and rotation (the object's when it has no deco)
        const math::quat<float> rotation = m_deco ? m_deco->GetRotation() : GetRotation();
        for (size_t i = 0; i < m_effectOnDistanceTrigger.size(); i++)
            EffectMgr::GetInstance()->Add(Object::GetEffectLayerId(), m_effectOnDistanceTrigger[i], position, rotation, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
        m_useDistanceTrigger = false;
        if (m_hideOnDistanceTrigger && m_deco) {
            m_deco->SetVisible(false);
            ReleaseEffects();
        }
        minion->IncreaseDespicablePoints(m_despicablePointsOnDistanceTrigger, minionPosition);
    }
    if (m_playIdleSound && distanceSq <= m_playIdleSoundDistanceSq) {
        m_playIdleSound = false;
        // m_idleSoundHandle = GameSoundMgr::Play(m_idleSoundLabel, 0, GetPosition(), 0): no sound manager
    }
}

// @0x56d960
void InteractiveObjectTemplate::Update() {
    const unsigned dt = (unsigned)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    Object::Update();
    if (m_preContactState != PreContactAnimState::None) UpdatePreContactAnims(dt);
    if (m_useStateMachine) SM_Update();
    if (m_postFxMaterial) UpdatePostFxState(dt);
    UpateDistanceTriggers();
    SynchronizedObject::Update();
}

// @0x56d9f8
void InteractiveObjectTemplate::OnSyncFinished() { StopPacesetter(true); }

// @0x56da0c: what the object does when something touches it: unless it has nothing left to do, it plays its animation or state, tells the
// minion what happened, hides and starts its post effect. What the minion is
// told (its engine event through its state machine, its despicable points) are below.
void InteractiveObjectTemplate::CollidesWith(GameEntity *other) {
    if (m_frozen) return;
    // a contact that has already done its part (its animation or state is the one playing, no post effect or engine event waits, no effect is left)
    // does nothing
    const bool animDone = m_animOnContact.empty() || (m_deco && m_deco->GetAnimationName().Equals(m_animOnContact));
    if (animDone) {
        const bool stateDone = !(m_useStateMachine && !m_stateOnContact.empty()) || (SM_GetState() && SM_GetState()->GetName().Equals(m_stateOnContact));
        if (stateDone && (!m_postFxMaterial || m_postFxState != PostFxAnimState::None) && m_engineEvent < 0 && m_effectsOnContact.empty()) return;
    }
    if (m_deco && !m_deco->IsVisible()) return;
    if (!other) return;
    Minion *minion = dynamic_cast<Minion*>(other);
    if (m_checkLaneOnContact && minion) {
        Pacesetter *pace = GameLevel::GetInstance()->GetPacesetter();
        if (pace && (int)pace->GetCurrentLane() != (int)GetSpawnLane()) return;
    }
    // the effects on contact start for the minion (Object::CollidesWith @0x57faec: SpawnEffects of the listener of type 9)
    if (minion) SpawnEffects(m_effectsOnContact, !m_effectOnContactSpawnAll);
    m_effectsOnContact.clear();
    // a frozen minion takes nothing from the object: the contact ends here (the .so returns before the stop of the pacesetter, the animation and the
    // post effect; the effects above have already started)
    if (minion && minion->IsFrozenRun()) return;
    if (minion) {
        if (m_useEngineEvent) {
            // the minion is told the engine event of the object, then the object's points where it was touched (Minion::IncreaseDespicablePoints @0x57a398)
            minion->SM_OnEngineEvent(m_engineEvent);
            minion->IncreaseDespicablePoints(m_despicablePoints, GetPosition());
            m_engineEvent = -1;
            if (BackgroundMgr::GetInstance()->GetLocation() == 2)
                Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_evilEyesCount, 1);
        }
    }
    StopPacesetter(true);
    if (m_useStateMachine) SM_SetState(m_stateOnContact);
    else m_deco->SetAnimation(m_animOnContact, false, Deco3d::AnimInterpolation::IfPlaying);
    if (m_hideOnContact) {
        m_deco->SetVisible(false);
        ReleaseEffects();
    }
    if (m_postFxMaterial) SetPostFxState(PostFxAnimState::Intro);
    StopIdleSound();
}

// @0x56dccc: the state the object takes when the minion nears, the times of its effects
void InteractiveObjectTemplate::InitPreContactAnims() {
    bool enabled = false;
    GetParam(k_enablePreContactAnimations, enabled);
    if (!enabled) return;
    const clara::Param *stateParam = FindParamByName(k_stateOnPreContact);
    if (!stateParam) return;
    const clara::Param::StateValue state = stateParam->GetAsState(0);
    if (state.a.empty() || state.b.empty()) return;
    m_preContactState = PreContactAnimState::WaitMinion;
    m_stateOnPreContact = state.b;
    GetParam(k_preContactTime, m_preContactTime);
    GetParam(k_preContactEffectTime, m_preContactEffectTime);
    GetParam(k_preContactProjectileTime, m_preContactProjectileTime);
    m_preContactProjectileEffect = reinterpret_cast<EffectDef*>(EntityOfParam(this, k_preContactProjectileEffect));
    m_preContactImpactEffect = reinterpret_cast<EffectDef*>(EntityOfParam(this, k_PreContactImpactEffect));
    m_preContactObstacleEffect = reinterpret_cast<EffectDef*>(EntityOfParam(this, k_preContactObstacleEffect));
    if (!m_useStateMachine) {
        m_useStateMachine = true;
        SM_Init(m_deco ? m_deco->GetModel() : 0, state);
        SM_SetStateLabel(1);
    }
}

// @0x56ded0: the state machine of the contact: the set and the state of "stateOnContact", when it names both
void InteractiveObjectTemplate::InitStateMachine() {
    const clara::Param *p = FindParamByName(k_stateOnContact);
    if (!p) return;
    const clara::Param::StateValue state = p->GetAsState(0);
    m_useStateMachine = !state.a.empty() && !state.b.empty();
    if (!m_useStateMachine) return;
    m_stateOnContact = state.b;
    SM_Init(m_deco ? m_deco->GetModel() : 0, state);
    SM_SetStateLabel(1);
}

// @0x56df7c: the post effect goes through its states (intro, hold, outro, gone). It is the level's extra post effect, with the value of
// the delayer moving from 0 to 1 (intro), held a PostFxTime, and back to 0 (outro).
void InteractiveObjectTemplate::SetPostFxState(unsigned state) {
    m_postFxState = state;
    switch (state) {
    case PostFxAnimState::Intro:
        GameLevel::GetInstance()->AddExtraPostEffect(m_postFxMaterial, &m_postFxUniformValue, &m_postFxState);
        m_postFxUniformValue.SetDelay(m_postFxTimeIntro);
        m_postFxUniformValue.SetCurrentValue(0.0f);
        m_postFxUniformValue.SetTargetValue(1.0f);
        break;
    case PostFxAnimState::Hold:
    case PostFxAnimState::Outro:
        m_postFxUniformValue.SetDelay(state == PostFxAnimState::Hold ? m_postFxTime : m_postFxTimeOutro);
        m_postFxUniformValue.SetCurrentValue(1.0f);
        m_postFxUniformValue.SetTargetValue(0.0f);
        break;
    case PostFxAnimState::Removed:
        GameLevel::GetInstance()->RemoveExtraPostEffect(m_postFxMaterial);
        break;
    default:
        break;
    }
}

// @0x56e094: the delayer moves the value; the effect goes on to the next state when it is done. The value goes to the "uvDistortion" uniform
// (times the maximum, m_postFxMaxValue) and to "tintAmount" of the effect's material (jet::video::Material::SetUniform of the material that
// GameLevel::AddExtraPostEffect makes, which the tree's version does not give back): not set here.
void InteractiveObjectTemplate::UpdatePostFxState(unsigned dt) {
    switch (m_postFxState) {
    case PostFxAnimState::Hold:
        if (m_postFxUniformValue.IsUpdating()) m_postFxUniformValue.Update((int)dt);
        else SetPostFxState(PostFxAnimState::Outro);
        return;
    case PostFxAnimState::Intro:
    case PostFxAnimState::Outro:
        if (m_postFxUniformValue.IsUpdating()) m_postFxUniformValue.Update((int)dt);
        else SetPostFxState(m_postFxState + 1);
        return;
    default:
        return;
    }
}

// @0x56e32c: the idle sound stops (SoundMgr::Stop(handle, 100) when it still plays: there is no SoundMgr, so none was started)
void InteractiveObjectTemplate::StopIdleSound() {
    if (m_idleSoundHandle < 0) return;
    m_idleSoundHandle = -1;
}

// @0x56e3a8: when the object is frozen it plays one of its freeze animations, hides, and gives its points
void InteractiveObjectTemplate::OnFreeze() {
    m_frozen = true;
    if (m_hideOnFreeze) {
        m_deco->SetVisible(false);
        ReleaseEffects();
    }
    if (!m_animOnFreeze.empty())
        m_deco->SetAnimation(m_animOnFreeze[jet::core::Rand() % m_animOnFreeze.size()], false, Deco3d::AnimInterpolation::IfPlaying);
    SpawnEffects(m_effectOnFreeze, true);
    // the minion gets the object's despicable points where the object is (Minion::IncreaseDespicablePoints @0x57a398)
    if (GameLevel *level = GameLevel::GetInstance())
        if (Minion *minion = level->GetMinion()) minion->IncreaseDespicablePoints(m_despicablePoints, GetPosition());
    StopPacesetter(true);
    StopIdleSound();
}

// @0x56e4c4: a post effect that is on goes off with the gameplay
void InteractiveObjectTemplate::OnGamePlayChanged() {
    if (m_postFxState == PostFxAnimState::None) return;
    SetPostFxState(PostFxAnimState::Removed);
}

// @0x563d04
void InteractiveObjectTemplate::SetSpawnLane(int lane) { SynchronizedObject::SetSpawnLane((LaneIndex)lane); }

// @0x563d0c (and its thunk @0x563d14): the machine's id is the entity's
unsigned InteractiveObjectTemplate::SM_GetId() { return GetId(); }

// @0x563d1c: the synchronization starts again, and the engine event comes back
void InteractiveObjectTemplate::OnMinionRevive() {
    ResetPacestterCourse();
    if (m_useEngineEvent) GetParam(k_engineEventOnContact, m_engineEvent);
}
