// source/game/entities/InteractiveAnimatedPusher.cpp
#include "InteractiveAnimatedPusher.h"
#include "Actor.h"
#include "Collision3d.h"
#include "Deco3d.h"
#include "JumpDefinition.h"
#include "Minion.h"
#include "../GameLevel.h"
#include "../Statistics.h"
#include "../actors/statemachine/ActorState.h"
#include "param.h"
#include "project.h"

jet::String InteractiveAnimatedPusher::k_tmplName("InteractiveAnimatedPusher");
const jet::String InteractiveAnimatedPusher::k_animOnContact("animOnContact"), InteractiveAnimatedPusher::k_stateOnContact("stateOnContact"),
    InteractiveAnimatedPusher::k_jumpDefinition("jumpDefinition"), InteractiveAnimatedPusher::k_upSpeed("upSpeed"),
    InteractiveAnimatedPusher::k_frontSpeed("frontSpeed"), InteractiveAnimatedPusher::k_frontSpeedRestoreTime("frontSpeedRestoreTime"),
    InteractiveAnimatedPusher::k_minSpeedInfoPair("minSpeedInfoPair"), InteractiveAnimatedPusher::k_maxSpeedInfoPair("maxSpeedInfoPair"),
    InteractiveAnimatedPusher::k_useEngineEventParam("useEngineEventOnPushed"), InteractiveAnimatedPusher::k_engineEventParam("engineEvent"),
    InteractiveAnimatedPusher::k_countForJumperStatistics("countForJumperStatistics"), InteractiveAnimatedPusher::k_exitPointParam("exitPoint");

// @0x56bea4
InteractiveAnimatedPusher::InteractiveAnimatedPusher(const clara::Template *t)
    : Object(t), m_pusherState(Idle), m_jumpDefinition(0), m_frontSpeed(0), m_upSpeed(0), m_frontSpeedRestoreTime(0), m_engineEventOnPushed(-1),
      m_countForJumperStatistics(false), m_minSpeedInfoPair(0, 0), m_maxSpeedInfoPair(0, 0), m_minionPlayingState(0), m_exitPoint(0),
      m_ownedByLauncher(false) {
    m_type = 0x10;
}

// @0x56bf7c
InteractiveAnimatedPusher::~InteractiveAnimatedPusher() {}

// @0x56c028
void InteractiveAnimatedPusher::Init() {
    if (IsLibraryEntity()) return;
    Object::Init();
}

// @0x56c038: the parameters of the pusher; the exit point is the entity the minion goes to when the pusher lets it go (a location transition)
void InteractiveAnimatedPusher::PostInit() {
    if (IsLibraryEntity()) return;
    Object::PostInit();
    GetParam(k_animOnContact, m_animOnContact);
    if (const clara::Param *p = FindParamByName(k_stateOnContact)) m_stateOnContact = p->GetAsState(0).b;
    GetParam(k_upSpeed, m_upSpeed);
    GetParam(k_frontSpeed, m_frontSpeed);
    GetParam(k_frontSpeedRestoreTime, m_frontSpeedRestoreTime);
    m_engineEventOnPushed = -1;
    bool useEngineEvent = false;
    GetParam(k_useEngineEventParam, useEngineEvent);
    if (useEngineEvent) GetParam(k_engineEventParam, m_engineEventOnPushed);
    GetParam(k_minSpeedInfoPair, m_minSpeedInfoPair);
    GetParam(k_maxSpeedInfoPair, m_maxSpeedInfoPair);
    GetParam(k_countForJumperStatistics, m_countForJumperStatistics);
    // the path is kept from the jump definition when the exit point's parameter is missing, as the .so does
    clara::Path path;
    if (const clara::Param *p = FindParamByName(k_jumpDefinition)) path = p->GetAsPath(0);
    m_jumpDefinition = dynamic_cast<JumpDefinition*>(clara::Project::GetInstance()->FindEntityByPath(path));
    if (const clara::Param *p = FindParamByName(k_exitPointParam)) path = p->GetAsPath(0);
    clara::DataEntity *exit = clara::Project::GetInstance()->FindEntityByPath(path);
    m_exitPoint = exit ? dynamic_cast<clara::Entity*>(exit) : 0;
    SynchronizedObject::Init(this);
    GetDeco()->SetForceAnimation(true);
    RegisterForUpdate(true);
}

// @0x56c268: the synchronized movement runs; a pushing pusher lets the minion go once it changes state
void InteractiveAnimatedPusher::Update() {
    Object::Update();
    SynchronizedObject::Update();
    if (m_pusherState != Pushing) return;
    if (m_minionPlayingState == GameLevel::GetInstance()->GetMinion()->SM_GetState()) return;
    SetPusherState(Released);
}

// @0x56be84: the pusher is idle again when the minion revives
void InteractiveAnimatedPusher::OnMinionRevive() {
    SynchronizedObject::ResetPacestterCourse();
    SetPusherState(Idle);
}

// @0x56c2c8: an actor that runs onto the idle pusher starts it. A minion must take the jumpers it runs onto and not be linked to something
// else; the jumper statistic counts the contact when the data asks for it
void InteractiveAnimatedPusher::CollidesWith(Collision3d *c, const CollisionPair &pair) {
    if (m_pusherState != Idle) return;
    Collision3dListener *listener = c->GetCollisionListener();
    if (!listener) return;
    GameEntity *e = listener->GetAsEntity();
    if (!e || !e->GetBaseTemplateName().Equals(Actor::k_tmplName)) return;
    if (e->GetTemplateName().Equals(Minion::k_tmplName)) {
        if (!static_cast<Minion*>(e)->InteractWithJumpers()) return;
        if (e->IsLinked()) return;
    }
    if (m_countForJumperStatistics) Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_usedJumpersCount, 1);
    Object::CollidesWith(c, pair);
    SetPusherState(Pushing);
}

// @0x56c400: Pushing: the minion is pushed (its state on contact, the animation of the pusher). Released: the minion is let go at the
// exit point (the exit point takes the pusher's link for the time of the push, and is placed where it was)
void InteractiveAnimatedPusher::SetPusherState(PusherState s) {
    m_pusherState = s;
    if (s == Pushing) {
        Minion *minion = GameLevel::GetInstance()->GetMinion();
        minion->OnInteractiveAnimatedPusherStart(this, m_stateOnContact, m_minSpeedInfoPair, m_maxSpeedInfoPair, m_exitPoint);
        m_minionPlayingState = minion->SM_GetState();
        GetDeco()->SetAnimation(m_animOnContact, false, Deco3d::AnimInterpolation::IfPlaying);
    } else if (s == Released) {
        m_minionPlayingState = 0;
        Minion *minion = GameLevel::GetInstance()->GetMinion();
        math::vec3<float> pos(0, 0, 0);
        math::quat<float> rot(0, 0, 0, 1);
        if (m_exitPoint) {
            pos = m_exitPoint->GetPosition();
            rot = m_exitPoint->GetRotation();
            m_exitPoint->LinkTo(GetLinkee(), GetLinkPointName(), 0);
            m_exitPoint->SetPosition(pos);
            m_exitPoint->SetRotation(rot);
        }
        minion->OnInteractiveAnimatedPusherStop(m_upSpeed, m_frontSpeed, m_frontSpeedRestoreTime, m_jumpDefinition, m_exitPoint);
        if (m_engineEventOnPushed != -1) minion->SM_OnEngineEvent(m_engineEventOnPushed);
        if (m_exitPoint) {
            m_exitPoint->LinkTo(0, jet::String::null, 0);
            m_exitPoint->SetPosition(pos);
            m_exitPoint->SetRotation(rot);
        }
    }
}
