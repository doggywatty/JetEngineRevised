// source/game/entities/SynchronizedObject.cpp
#include "SynchronizedObject.h"
#include <cstdio>
#include "Deco3d.h"
#include "Object.h"
#include "Minion.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "anim/animation.h"
#include "anim/animationloader.h"
#include "core/util.h"
#include "project.h"
#include "scene/model.h"

const jet::String SynchronizedObject::k_prePositionAnimation("PreAnchorPositionAnimation"), SynchronizedObject::k_postPositionAnimation("PostAnchorPositionAnimation"),
    SynchronizedObject::k_timeOffset("timeOffset"), SynchronizedObject::k_useSyncMovement("useSyncMovement"), SynchronizedObject::k_syncDistance("syncDistance"),
    SynchronizedObject::k_syncTime("syncTime"), SynchronizedObject::k_preContactTimeMaxParam("preContactTimeMax"),
    SynchronizedObject::k_preContactTimeMinParam("preContactTimeMin"), SynchronizedObject::k_preContactEngineEventParam("preContactEngineEvent");

// the position / rotation an entity has relative to what it is linked to (its own when it is not linked) (clara::Entity::GetLocalPosition @0x517f7c, GetLocalRotation @0x517f90)
static math::vec3<float> LocalPosition(const clara::Entity *e) { return e->IsLinked() ? e->GetLinkData()->position : e->GetPosition(); }
static math::quat<float> LocalRotation(const clara::Entity *e) { return e->IsLinked() ? e->GetLinkData()->rotation : e->GetRotation(); }

// the pacesetter the minion runs on (the level's: the minion keeps a pointer to it at 0x218 of its own)
static Pacesetter *MinionPacesetter() { return GameLevel::GetInstance()->GetPacesetter(); }

// @0x5222f8
SynchronizedObject::SynchronizedObject()
    : m_animType(0), m_timeOffset(0), m_prePositionAnimationTime(0), m_lastTime(0), m_skipCount(0), m_pacesetterInstance(0), m_syncTime(0), m_syncDistance(0),
      m_pacesetterCourseInit(false), m_pacesetterCourseFinished(false), m_pacesetterCourseLocked(false), m_timeToGetReached(3600000.0f),
      m_distanceToGetReached(9999999.0f), m_lastTimeComputeFrame(-1), m_spawnLane(Lane_Count), m_owner(0), m_localPosition(0, 0, 0),
      m_localRotation(0, 0, 0, 1), m_preContactTimeMax(0), m_preContactTimeMin(0), m_preContactEngineEvent(0) {}

// @0x5223ac: the own pacesetter goes with it
SynchronizedObject::~SynchronizedObject() {
    StopPacesetter(false);
    delete m_pacesetterInstance;
}

// @0x52247c
void SynchronizedObject::Init(Object *owner) {
    m_owner = owner;
    if (owner->HasParam(k_prePositionAnimation)) {
        owner->GetParam(k_prePositionAnimation, m_prePositionAnimation);
        owner->GetParam(k_postPositionAnimation, m_postPositionAnimation);
        owner->GetParam(k_timeOffset, m_timeOffset);
        if (!m_prePositionAnimation.empty()) {
            if (jet::anim::Animation *a = jet::anim::AnimationLoader::GetInstance()->Load(m_prePositionAnimation))
                m_prePositionAnimationTime = (int)a->GetDuration();
            else
                fprintf(stderr, "[Jet] Failed to load animation: %s\n", m_prePositionAnimation.c_str());
        }
    }
    if (owner->HasParam(k_useSyncMovement)) {
        bool use = false;
        owner->GetParam(k_useSyncMovement, use);
        if (use) {
            owner->GetParam(k_syncTime, m_syncTime);
            owner->GetParam(k_syncDistance, m_syncDistance);
            InitPacesetter();
            m_skipCount = (int)((int)jet::core::Rand() % 5);
            owner->GetDeco()->SetForceAnimation(true);
        }
    }
    m_animType = SynchronizationAnimType::None;
    if (!m_prePositionAnimation.empty()) {
        m_animType = SynchronizationAnimType::PrePosition;
        owner->GetDeco()->SetForceAnimation(true);
        m_lastTime = 0x540be3ff;
    } else if (m_pacesetterInstance) {
        m_animType = SynchronizationAnimType::SyncMovement;
    }
    m_preContactEngineEvent = -1;
    if (owner->HasParam(k_preContactEngineEventParam)) {
        owner->GetParam(k_preContactTimeMaxParam, m_preContactTimeMax);
        owner->GetParam(k_preContactTimeMinParam, m_preContactTimeMin);
        owner->GetParam(k_preContactEngineEventParam, m_preContactEngineEvent);
    }
}

// @0x563c40
void SynchronizedObject::Update() {
    if (m_animType != SynchronizationAnimType::None) {
        if (m_skipCount > 0) m_skipCount--;
        else _Update();
    }
    if (m_preContactEngineEvent >= 0) _UpdatePreContact();
}

// @0x5226d4: how long (ms) until the minion reaches the object, looked at once a frame
float SynchronizedObject::ComputeTimeToGetReached() {
    const int frame = Game::GetInstance()->GetUpdateCounter();
    if (frame != m_lastTimeComputeFrame) {
        m_timeToGetReached = 3600000.0f;
        Pacesetter *pace = MinionPacesetter();
        if (pace && pace->GetSpeed() > 0)
            m_timeToGetReached = pace->GetTimeToReachPlane(m_owner->GetPosition(), m_owner->GetRotation(), m_cache0, &m_distanceToGetReached);
    }
    m_lastTimeComputeFrame = frame;
    return m_timeToGetReached;
}

// @0x52279c: a few ms before the minion reaches the object (on its lane, on foot), the engine event is sent
void SynchronizedObject::_UpdatePreContact() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    Pacesetter *pace = MinionPacesetter();
    if (!minion || !pace) return;
    if ((int)pace->GetCurrentLane() != (int)m_spawnLane) return;
    // the minion takes no pre contact while it is in the rocket (+0x389), the fluffy (+0x38A), fast forward (+0x38B), frozen (+0x38C), riding
    // (+0x38E) or falling out (+0x390) state
    if (minion->IsInRocketMode() || minion->IsInFluffyMode() || minion->IsInFastForward() || minion->IsFrozenRun() || minion->IsRiding() ||
        minion->IsFallingOut())
        return;
    const float reach = (float)m_preContactTimeMax * pace->GetTargetSpeed() * 0.001f;
    if (math::distanceSq(minion->GetPosition(), m_owner->GetPosition()) > reach * reach) return;
    const int t = (int)ComputeTimeToGetReached();
    if (t > m_preContactTimeMax || t < m_preContactTimeMin) return;
    // the minion is told the pre contact engine event (its SM_OnEngineEvent, vtbl +0xCC); the event is sent once: when the minion takes it
    // (returns true) it is forgotten (m_preContactEngineEvent = -1)
    if (minion->SM_OnEngineEvent(m_preContactEngineEvent)) m_preContactEngineEvent = -1;
}

// @0x5228f8
void SynchronizedObject::_Update() {
    Pacesetter *pace = MinionPacesetter();
    const bool running = pace && pace->GetSpeed() > 0;
    if (!running) {
        if (m_animType != SynchronizationAnimType::SyncMovement) return;
        // @0x522c3c: the minion that is dying or dead lets go of an object that moves with a pacesetter (Minion::IsDying @0x577284 and
        // Minion::IsDead @0x577230 test the run state ids 0x1d 0x2e 0x24 0x25 0x26 and 0x1e: Minion has no run states here, so
        // neither is ever true)
        return;
    }
    if (m_animType == SynchronizationAnimType::PrePosition) {
        Deco3d *deco = m_owner->GetDeco();
        const float t = ComputeTimeToGetReached();
        int limit = m_lastTime < m_prePositionAnimationTime ? m_lastTime : m_prePositionAnimationTime;
        int r5 = (int)(t - (float)m_timeOffset);
        if (limit < r5) r5 = limit;
        m_lastTime = r5;
        jet::scene::Model *model = deco->GetModel();
        jet::anim::Animation *playing = model->GetAnimation();
        if (r5 <= 0) {
            if (r5 == 0) return;
            // the object is past: its post position animation, its time kept
            jet::scene::Model::Controller &cc = model->CC();
            if (!cc.IsActive()) cc.SetActive(true);
            if (!playing || playing->GetResourceName() != m_postPositionAnimation)
                deco->SetAnimation(m_postPositionAnimation, false, Deco3d::AnimInterpolation::IfPlaying);
            return;
        }
        if (r5 > m_prePositionAnimationTime) return;
        if (!playing || playing->GetResourceName() != m_prePositionAnimation)
            deco->SetAnimation(m_prePositionAnimation, false, Deco3d::AnimInterpolation::IfPlaying);
        // the animation is never ahead of what the time left says, nor goes back
        model = deco->GetModel();
        jet::scene::Model::Controller &cc = model->CC();
        int want = m_prePositionAnimationTime - r5;
        if (want < 0) want = 0;
        if (want > m_prePositionAnimationTime) want = m_prePositionAnimationTime;
        int time = cc.GetTime();
        if (want >= time) time = want;
        cc.SetTime(time);
        const bool active = time != 0;
        if (cc.IsActive() != active) cc.SetActive(active);
        if (active && m_prePositionAnimation.Equals("obstacle_door_anim.anim")) {
            // "le.huathi - hard code play sound: ev_sfx_door_opens": SoundMgr::Play("ev_sfx_door_opens") when it is not playing (no SoundMgr)
        }
        if (cc.IsFinished()) m_lastTime = -1;
        return;
    }
    if (m_animType != SynchronizationAnimType::SyncMovement) return;
    // @0x522ae8: the object moves along its pacesetter, for the time it has to meet the minion
    if (!m_pacesetterInstance || m_pacesetterCourseFinished) return;
    if (!m_pacesetterCourseInit || !m_pacesetterInstance->GetCurrentGameplayTemplateInstance()) {
        InitPacesetterCourse();
        m_skipCount = 5;
        return;
    }
    const float t = ComputeTimeToGetReached();
    if (t > 0 && (int)t <= m_syncTime) {
        float distance = t * (m_syncDistance / (float)m_syncTime);
        bool ok = true;
        if (m_pacesetterInstance->IsPaused()) {
            ok = StartPacestterCourse(t, distance);
        } else {
            float direction = 0;
            m_pacesetterInstance->GetDistanceInLane(m_owner->GetPosition(), distance, m_cache1, direction, (LaneIndex)3);
        }
        if (ok && distance > 0.9f && t > 300.0f) m_pacesetterInstance->SetSpeed(-(distance / (t * 0.001f)), 0);
    } else if (t <= -300.0f && GetPacesetter()) {
        OnSyncFinished();
        StopPacesetter(false);
        m_pacesetterCourseLocked = true;
    } else if (t > (float)m_syncTime && !m_pacesetterInstance->IsPaused()) {
        ResetPacestterCourse();
    }
    int skip = (int)(t * 0.001f);
    m_skipCount = skip < 1 ? 1 : (skip > 5 ? 5 : skip);
}

// @0x522e00 is GoTo (nothing); @0x522e00
void SynchronizedObject::OnGameplayTemplateInstanceDestroyed() {
    StopPacesetter(true);
    SetPacesetter(0);
}

// @0x522e1c: the own pacesetter is put on the lane at the object's place, when the object is near the minion (40 units) and the level has a template
bool SynchronizedObject::InitPacesetterCourse() {
    if (!m_pacesetterInstance || m_pacesetterCourseInit) return false;
    GameLevel *level = GameLevel::GetInstance();
    GameplayTemplateInstance *gti = level->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance();
    if (!gti) return false;
    if (math::distanceSq(level->GetMinion()->GetPosition(), m_owner->GetPosition()) > 1600.0f) return false;
    m_pacesetterInstance->SetCurrentGameplayTemplateInstance(gti);
    m_pacesetterCourseInit = m_pacesetterInstance->SetLanePosition(m_owner->GetPosition(), m_syncDistance, m_spawnLane, 0);
    if (!m_pacesetterCourseInit) return false;
    m_pacesetterInstance->ApplyCurrentLaneTransform();
    return true;
}

// @0x522f40: a pacesetter of its own (paused) takes the object's deco
void SynchronizedObject::InitPacesetter() {
    clara::Project *project = clara::Project::GetInstance();
    m_pacesetterInstance = new Pacesetter(project->FindTemplateByName(Pacesetter::k_tmplName));
    m_pacesetterInstance->Init();
    m_pacesetterInstance->PostInit();
    m_pacesetterInstance->SetPaused(true);
    m_pacesetterInstance->RegisterFollower(this);
    Deco3d *deco = m_owner->GetDeco();
    m_localPosition = LocalPosition(deco);
    m_localRotation = LocalRotation(deco);
    const math::vec3<float> position = LocalPosition(m_owner) + m_localPosition;
    const math::quat<float> rotation = LocalRotation(m_owner) * m_localRotation;
    deco->LinkTo(0, jet::String::null, 0);
    deco->LinkTo(m_pacesetterInstance, jet::String::null, 0);
    deco->SetPosition(position);
    deco->SetRotation(rotation);
    if (Deco3d *linked = m_owner->GetLinkedDeco()) linked->LinkTo(deco, jet::String::null, 0);
}

// @0x5230ec
void SynchronizedObject::AddSyncMovementDistance(float d) {
    if (m_pacesetterInstance) m_syncDistance += d;
}

// @0x523108: the pacesetter starts to run so that it is at the object's place when the time is up
bool SynchronizedObject::StartPacestterCourse(float timeToReach, float distance) {
    if (!m_pacesetterInstance) return false;
    GameLevel *level = GameLevel::GetInstance();
    m_pacesetterInstance->SetCurrentGameplayTemplateInstance(level->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance());
    if (!m_pacesetterInstance->SetLanePosition(m_owner->GetPosition(), distance, m_spawnLane, 0)) return false;
    m_pacesetterInstance->RegisterFollower(this);
    m_pacesetterInstance->SetSpeed(-distance / (timeToReach * 0.001f), 0);
    m_pacesetterInstance->SetPaused(false);
    return true;
}

// @0x5231f8: after a revive the course starts again, unless the object is too near for it
bool SynchronizedObject::OnMinionRevive() {
    float distance = m_distanceToGetReached;
    if (distance < 60.0f)
        MinionPacesetter()->GetTimeToReachPlane(m_owner->GetPosition(), m_owner->GetRotation(), m_cache0, &distance);
    if (m_animType != SynchronizationAnimType::None && (distance <= 15.0f || m_distanceToGetReached * 2.5f <= m_syncDistance)) {
        StopPacesetter(true);
        return false;
    }
    ResetPacestterCourse();
    return true;
}

// @0x5232ec
void SynchronizedObject::ResetPacestterCourse() {
    if (m_pacesetterCourseLocked) return;
    m_pacesetterCourseInit = false;
    m_pacesetterCourseFinished = false;
    if (m_pacesetterInstance) m_pacesetterInstance->SetPaused(true);
}

// @0x523314: the deco goes back to its object (when asked), the pacesetter stops, and the places looked at are forgotten
void SynchronizedObject::StopPacesetter(bool relink) {
    if (!m_pacesetterInstance) return;
    Deco3d *deco = m_owner->GetDeco();
    if (!deco->IsLinkedTo(m_pacesetterInstance)) return;
    if (relink) {
        deco->LinkTo(m_owner, jet::String::null, 0);
        deco->SetPosition(m_localPosition);
        deco->SetRotation(m_localRotation);
    }
    if (GetPacesetter() == m_pacesetterInstance) m_pacesetterInstance->UnregisterFollower(this);
    m_pacesetterInstance->SetPaused(true);
    m_pacesetterCourseFinished = true;
    m_cache0.clear();
    m_cache1.clear();
}
