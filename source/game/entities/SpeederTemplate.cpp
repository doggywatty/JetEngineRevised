// source/game/entities/SpeederTemplate.cpp
#include "SpeederTemplate.h"
#include "Collision3d.h"
#include "Minion.h"
#include "../Statistics.h"
#include "param.h"

jet::String SpeederTemplate::k_tmplName("SpeederTemplate");
const jet::String SpeederTemplate::k_distanceParam("distance"), SpeederTemplate::k_timeParam("time");

// @0x522028
SpeederTemplate::SpeederTemplate(const clara::Template *t) : Object(t), m_distance(0), m_time(0), m_sendEngineEvent(true) { m_type = 0x12; }

// @0x522094
SpeederTemplate::~SpeederTemplate() {}

// @0x52212c
void SpeederTemplate::Init() {
    if (m_flags & F_LIBRARY) return;
    Object::Init();
}

// @0x52213c
void SpeederTemplate::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    GetParam(k_distanceParam, m_distance);
    GetParam(k_timeParam, m_time);
    SynchronizedObject::Init(this);
}

// @0x5221ac
void SpeederTemplate::Update() {
    Object::Update();
    SynchronizedObject::Update();
}

// @0x52200c
void SpeederTemplate::OnMinionRevive() {
    ResetPacestterCourse();
    m_sendEngineEvent = true;
}

// @0x522200: the minion that runs onto the speeder is pushed forward by its distance and time (the engine event goes with the first push of the
// run); the speeder statistic counts that first contact
void SpeederTemplate::CollidesWith(Collision3d *c, const CollisionPair &pair) {
    Collision3dListener *listener = c->GetCollisionListener();
    if (!listener) return;
    Minion *minion = dynamic_cast<Minion*>(listener->GetAsEntity());
    if (!minion || !minion->UseSpeeder()) return;
    if (!minion->PushFront(m_distance, m_time, m_sendEngineEvent)) return;
    if (!m_sendEngineEvent) return;
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_usedSpeederCount, 1);
    Object::CollidesWith(c, pair);
    m_sendEngineEvent = false;
}
