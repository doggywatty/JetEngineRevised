// source/game/entities/PlatformTemplate.cpp
#include "PlatformTemplate.h"
#include "Object.h"
#include "param.h"
#include "project.h"

jet::String PlatformTemplate::k_tmplName("PlatformTemplate"), PlatformTemplate::k_LaneInterval_tmplName("LaneInterval");

// @0x4afc20
PlatformTemplate::PlatformTemplate(const clara::Template *t)
    : GameEntity(t), m_createdEntity(0), m_keepCreatedEntity(false), m_bifurcation(Bifurcation_Center), m_hasSecretExit(false), m_secretExitDirection(Bifurcation_Center),
      m_hasForbiddenMovements(false), m_hasBonusIntervals(false), m_beginMinionSledgeOffset(0), m_endMinionSledgeOffset(0), m_sledgeType(0), m_teeLeft(0), m_teeRight(0) {
    m_type = 0xd;
}

// @0x4afc98
PlatformTemplate::~PlatformTemplate() {
    if (!m_keepCreatedEntity) delete m_createdEntity;
    for (LaneInterval *i : m_laneForbiddenMovements) delete i;
    for (LaneInterval *i : m_bonusIntervals) delete i;
}

Bifurcation PlatformTemplate::ParseBifurcation(const jet::String &s, Bifurcation current, bool allowTee) {
    if (s.Equals("center")) return Bifurcation_Center;
    if (s.Equals("left")) return Bifurcation_Left;
    if (s.Equals("right")) return Bifurcation_Right;
    if (allowTee && s.Equals("tee")) return Bifurcation_Tee;
    return current;
}

// @0x4a00f4, @0x4a0210
void PlatformTemplate::LoadIntervals(const clara::Param *p, std::vector<LaneInterval*> &out) {
    if (!p) return;
    for (unsigned i = 0; i < p->GetComponentCount(); i++) {
        clara::DataEntity *e = p->GetAsEntity(i);
        LaneInterval *l = new LaneInterval;
        e->GetParam(jet::String("originLane"), l->originLane);
        e->GetParam(jet::String("destinationLane"), l->destinationLane);
        l->nodeInterval = math::vec2<float>(0, 0);
        e->GetParam(jet::String("nodeInterval"), l->nodeInterval);
        out.push_back(l);
    }
}

// @0x49fd8c: a template that is not a library one makes its object
void PlatformTemplate::Init() {
    jet::String s;
    GetParam(jet::String("Bifurcation"), s);
    m_bifurcation = ParseBifurcation(s, m_bifurcation, true);
    if (m_bifurcation == Bifurcation_Tee) {
        GetParam(jet::String("TeeLeft"), reinterpret_cast<int&>(m_teeLeft));
        GetParam(jet::String("TeeRight"), reinterpret_cast<int&>(m_teeRight));
    }
    GetParam(jet::String("HasSecretExit"), m_hasSecretExit);
    if (m_hasSecretExit) {
        GetParam(jet::String("SecretExitDirection"), s);
        m_secretExitDirection = ParseBifurcation(s, m_secretExitDirection, false);
        m_levelTemplateList.clear();
        const clara::Param *p = FindParamByName(jet::String("LevelTemplateList"));
        for (unsigned i = 0; i < p->GetComponentCount(); i++) m_levelTemplateList.push_back(p->GetAsU32(i));
    }
    GetParam(jet::String("HasForbiddenMovements"), m_hasForbiddenMovements);
    if (m_hasForbiddenMovements) LoadIntervals(FindParamByName(jet::String("LaneForbiddenMovements")), m_laneForbiddenMovements);
    GetParam(jet::String("hasBonusIntervals"), m_hasBonusIntervals);
    if (m_hasBonusIntervals) LoadIntervals(FindParamByName(jet::String("bonusIntervals")), m_bonusIntervals);
    if (m_flags & F_LIBRARY) return;
    clara::DataEntity::Init();
    m_createdEntity = new Object(clara::Project::GetInstance()->FindTemplateByName(jet::String("Object")));
    m_createdEntity->ConstructFrom(static_cast<const GameEntity*>(this));
}

// @0x4a0348
void PlatformTemplate::PostInit() {
    GetParam(jet::String("BeginMinionSledgeOffset"), m_beginMinionSledgeOffset);
    GetParam(jet::String("EndMinionSledgeOffset"), m_endMinionSledgeOffset);
    GetParam(jet::String("sledgeType"), m_sledgeType);
    if (m_flags & F_LIBRARY) return;
    clara::DataEntity::PostInit();
    RegisterForRender(false);
}

// @0x4a03d4
GameEntity *PlatformTemplate::GetDecoReference() const {
    return static_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByPath(FindParamByName(jet::String("objectDeco"))->GetAsPath(0)));
}

// @0x4a0418
GameEntity *PlatformTemplate::GetCollisionReference() const {
    return static_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByPath(FindParamByName(jet::String("objectCollision"))->GetAsPath(0)));
}

// @0x4a0474
void PlatformTemplate::SetPersistenceType(int t) {
    GameEntity::SetPersistenceType(t);
    if (m_createdEntity) m_createdEntity->SetPersistenceType(t);
}
