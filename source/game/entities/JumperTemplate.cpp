// source/game/entities/JumperTemplate.cpp
#include "JumperTemplate.h"
#include "Collision3d.h"
#include "Deco3d.h"
#include "JumpDefinition.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "../Statistics.h"
#include "param.h"
#include "project.h"

jet::String JumperTemplate::k_tmplName("JumperTemplate");
const jet::String JumperTemplate::k_jumpDefininitionParam("JumpDefininition"), JumperTemplate::k_jumpSpeedParam("JumpSpeed"),
    JumperTemplate::k_jumpFrontMinSpeedParam("FrontMinSpeed"), JumperTemplate::k_jumpFrontMinSpeedTimeParam("FrontMinSpeedTransitionTime"),
    JumperTemplate::k_engineEventParam("engineEvent");

// @0x56f07c
JumperTemplate::JumperTemplate(const clara::Template *t)
    : Object(t), m_jumpDefinition(0), m_jumpDone(false), m_engineEvent(0), m_jumpSpeed(0), m_jumpFrontMinSpeed(0), m_jumpFrontMinSpeedTime(0) {
    m_type = 0x11;
}

// @0x56f0e8
JumperTemplate::~JumperTemplate() {}

// @0x56f180
void JumperTemplate::Init() {
    if (m_flags & F_LIBRARY) return;
    Object::Init();
}

// @0x56f190
void JumperTemplate::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    GetParam(k_jumpSpeedParam, m_jumpSpeed);
    GetParam(k_jumpFrontMinSpeedParam, m_jumpFrontMinSpeed);
    int time = 0;
    GetParam(k_jumpFrontMinSpeedTimeParam, time);
    m_jumpFrontMinSpeedTime = (unsigned)time;
    GetParam(k_engineEventParam, m_engineEvent);
    clara::Path path;
    if (const clara::Param *p = FindParamByName(k_jumpDefininitionParam)) path = p->GetAsPath(0);
    m_jumpDefinition = dynamic_cast<JumpDefinition*>(clara::Project::GetInstance()->FindEntityByPath(path));
    SynchronizedObject::Init(this);
}

// @0x56f298
void JumperTemplate::Update() {
    Object::Update();
    SynchronizedObject::Update();
}

// @0x56f060
void JumperTemplate::OnMinionRevive() {
    ResetPacestterCourse();
    m_jumpDone = false;
}

// @0x56f2b0: the jumper is taken once per run (m_jumpDone, set by the first listener). The minion that runs onto it, with a pacesetter and
// InteractWithJumpers, is counted in the jumpers statistic, then thrown along the jump definition from the jumper's place (or the link point of
// the deco it is linked to) at the pacesetter's rotation, and the jumper's engine event is sent to it; without a jump definition it is pushed up
// by the jump speed.
void JumperTemplate::CollidesWith(Collision3d *c, const CollisionPair &pair) {
    Object::CollidesWith(c, pair);
    if (m_jumpDone) return;
    Collision3dListener *listener = c->GetCollisionListener();
    if (!listener) return;
    m_jumpDone = true;
    Minion *minion = dynamic_cast<Minion*>(listener->GetAsEntity());
    if (!minion || !minion->GetPacesetter()) return;
    if (!minion->InteractWithJumpers()) return;
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_usedJumpersCount, 1);
    if (!m_jumpDefinition) {
        minion->PushUp(m_jumpSpeed, m_jumpFrontMinSpeed, m_engineEvent, (int)m_jumpFrontMinSpeedTime);
        return;
    }
    math::vec3<float> pos = GetPosition();
    if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) {
        math::quat<float> rot(0, 0, 0, 1);
        GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
    }
    if (getenv("JET_DBGMINION")) fprintf(stderr, "jumper launch at (%.1f %.1f %.1f) minion (%.1f %.1f %.1f) speed %.1f\n", pos.x, pos.y, pos.z, minion->GetPosition().x, minion->GetPosition().y, minion->GetPosition().z, m_jumpSpeed);
    minion->StartJumpDefinition(m_jumpDefinition, pos, minion->GetPacesetter()->GetRotation());
    minion->SM_OnEngineEvent(m_engineEvent);
}
