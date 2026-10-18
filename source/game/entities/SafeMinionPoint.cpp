// source/game/entities/SafeMinionPoint.cpp
#include "SafeMinionPoint.h"
#include "../GameLevel.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "scene/scenemgr.h"

jet::String SafeMinionPoint::k_tmplName("SafeMinionPoint");
const jet::String SafeMinionPoint::k_isLandableParam("isLandable");
const float SafeMinionPoint::k_distanceToCheckSq = 25.0f;  // @0xc8b384

// @0x521604
SafeMinionPoint::SafeMinionPoint(const clara::Template *t)
    : GameEntity(t), m_isRegistered(false), m_notifier(0), m_plane(math::vec3<float>(0, 0, 1), 0.0f), m_lane(1), m_unregisterFromNotifier(false), m_isLandable(true) {}

// @0x521670
SafeMinionPoint::~SafeMinionPoint() {
    if (m_isRegistered) UnregisterSafeMinionPoint();
    if (m_notifier) UnregisterFromNotifier();
    RegisterForRender(false);
    RegisterForUpdate(false);
}

// @0x521728
void SafeMinionPoint::Init() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::Init();
}

// @0x521738
void SafeMinionPoint::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::PostInit();
    GetParam(k_isLandableParam, m_isLandable);
    RegisterForUpdate(true);
}

// @0x52178c: the plane across the way (the front of the point) through it
void SafeMinionPoint::ComputeCollisionPlane() {
    const math::vec3<float> front = GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    m_plane.setPlane(GetPosition(), front);
}

// @0x5217f0: a point whose template resolved its bifurcation stops listening to it. The .so calls UnregisterFromNotifier (@0x5218e0) here
// unguarded, which reads the notifier at +0x94; a template destroyed after its bifurcation resolved has cleared the notifier (the Destroyed
// event above), so the point only drops the flag then.
void SafeMinionPoint::Update() {
    if (!m_unregisterFromNotifier) return;
    if (m_notifier) UnregisterFromNotifier();
    m_unregisterFromNotifier = false;
}

// @0x521814
float SafeMinionPoint::GetDistance(const math::vec3<float> &p) { return (GetPosition() - p).getLength(); }

// @0x521860: the template is gone (the point is not a safe place any more), or it resolved its bifurcation (the point is, but stops listening)
void SafeMinionPoint::OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent &e) {
    if (e.type == GameplayTemplateInstanceEvent::Destroyed) {
        UnregisterSafeMinionPoint();
        m_notifier = 0;
    }
    if (e.type != GameplayTemplateInstanceEvent::BifurcationResolved) return;
    UnregisterSafeMinionPoint();
    m_unregisterFromNotifier = true;
}

// @0x5218a4
void SafeMinionPoint::UnregisterSafeMinionPoint() {
    GameLevel::GetInstance()->RegisterSafePoint(this, false);
    m_isRegistered = false;
}

// @0x5218e0
void SafeMinionPoint::UnregisterFromNotifier() {
    m_notifier->UnregisterListener(this);
    m_notifier = 0;
}

// @0x52190c
void SafeMinionPoint::Register(GameplayTemplateInstance *g) {
    RegisterSafeMinionPoint();
    RegisterToNotifier(g);
}

// @0x52192c
void SafeMinionPoint::RegisterSafeMinionPoint() {
    m_isRegistered = true;
    GameLevel::GetInstance()->RegisterSafePoint(this, true);
}

// @0x521958
void SafeMinionPoint::RegisterToNotifier(GameplayTemplateInstance *g) {
    m_notifier = g;
    g->RegisterListener(this);
}
