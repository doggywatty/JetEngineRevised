// source/game/gamemanagers/EdgeProbabilityMgr.cpp
#include "EdgeProbabilityMgr.h"
#include "BackgroundMgr.h"
#include "LocationsMgr.h"
#include "MissionMgr.h"
#include "../GameLevel.h"
#include "../Statistics.h"
#include "../entities/LocationConfig.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../leveltemplate/LevelSequenceParser.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "../utils/ControlledRandom.h"
#include "param.h"
#include "project.h"

const jet::String EdgeProbabilityMgr::k_ProbabilityEvolutionSet_tmplName("ProbabilityEvolutionSet"),
    EdgeProbabilityMgr::k_ProbabilityIntervalDef_tmplName("ProbabilityIntervalDef");

EdgeProbabilityMgr::~EdgeProbabilityMgr() {
    for (ProbabilityInterval *i : m_probabilityIntervals) delete i;
}

// @0x4c736c: the set of the project named by the edge
void EdgeProbabilityMgr::Create(const jet::String &name, LevelSequenceParser&) {
    clara::Project *prj = clara::Project::GetInstance();
    clara::DataEntity *e = prj->FindEntityByName(name);
    if (const clara::Param *p = e->FindParamByName(jet::String("ProbabilityIntervals")))
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            clara::DataEntity *d = p->GetAsEntity(i);
            ProbabilityInterval *pi = new ProbabilityInterval;
            pi->roomAmount = 0;
            pi->increment = 0.0f;
            d->GetParam(jet::String("RoomAmount"), pi->roomAmount);
            d->GetParam(jet::String("ProbabilityIncrement"), pi->increment);
            m_probabilityIntervals.push_back(pi);
        }
    e->GetParam(jet::String("InitialProbability"), m_initialProbability);
    e->GetParam(jet::String("HasMission"), m_hasMission);
    if (m_hasMission) {
        const clara::Param *p = e->FindParamByName(jet::String("MissionsRef"));
        for (unsigned i = 0; i < p->GetComponentCount(); i++) m_missions.push_back(prj->FindEntityByPath(p->GetAsPath(i)));
    }
    e->GetParam(jet::String("BossDependent"), m_bossDependent);
    if (m_bossDependent) e->GetParam(jet::String("BossLocationDef"), m_bossLocation);
    e->GetParam(jet::String("IsLocationDependent"), m_isLocationDependent);
    if (m_isLocationDependent) e->GetParam(jet::String("LocationDef"), m_location);
    if (!m_probabilityIntervals.empty()) {
        m_roomsRunned = 0;
        m_currentInterval = 0;
        m_distanceRunned = 0;
    }
}

// @0x4c7684
bool EdgeProbabilityMgr::Evaluate() {
    const unsigned r = (unsigned)GameLevel::s_randomizer[0]->Rand();
    return (float)(int)(r % 100) < GetCurrentProbability();
}

// @0x4c76ec
void EdgeProbabilityMgr::Reset() {
    m_distanceRunned = 0;
    m_currentInterval = 0;
    m_roomsRunned = 0;
    m_initialProbability = 0;
}

// @0x4c7708
void EdgeProbabilityMgr::ResetData() {
    m_currentInterval = 0;
    m_roomsRunned = 0;
    m_initialProbability = 0;
}

// @0x4c7720: certain in the all probability mode, 100 when the boss is beaten, 0 while a mission is going, else the grown probability
float EdgeProbabilityMgr::GetCurrentProbability() const {
    float p;
    bool active;
    if (!m_bossDependent) {
        p = m_initialProbability;
        active = IsAnyMissionActive();
    } else if (IsBossBeaten()) {
        p = m_initialProbability;
        goto done;
    } else {
        p = 0.0f;
        active = IsAnyMissionActive();
    }
    if (active) p = 100.0f;
done:
    return GameLevel::s_allProbabilityMode ? 0.0f : p;
}

// @0x4c77b4
bool EdgeProbabilityMgr::IsAnyMissionActive() const {
    if (!m_hasMission) return false;
    MissionMgr *mm = MissionMgr::GetInstance();
    for (clara::DataEntity *m : m_missions)
        for (unsigned i = 0; i < mm->GetActiveMissionCount(); i++) {
            const MissionMgrSaveData::ActiveMissionInfo &a = mm->GetActiveMissionInfo(i);
            if (a.id.Equals(m->GetName()) && a.state == 1) return true;
        }
    return false;
}

// @0x4c784c
bool EdgeProbabilityMgr::IsAnyMissionDone() const {
    if (!m_hasMission) return false;
    for (clara::DataEntity *m : m_missions)
        if (MissionMgr::GetInstance()->WasMissionIdCompleted(m->GetName())) return true;
    return false;
}

// @0x4c7910: no boss, or the one the location has was beaten once, or its mission is done
bool EdgeProbabilityMgr::IsBossBeaten() const {
    LocationConfig *c = LocationsMgr::GetInstance()->GetLocationConfig(m_bossLocation);
    // (the .so has every location's config; one whose data is not here has no boss to beat)
    clara::DataEntity *boss = c ? c->GetBoss() : 0;
    if (!boss) return true;
    const jet::String &t = boss->GetTemplateName();
    const jet::String *scope;
    if (t.Equals("Boss")) scope = &StatisticsSaveData::k_statScope_defeatVector;
    else if (t.Equals("BossMeena")) scope = &StatisticsSaveData::k_statScope_defeatMeena;
    else if (t.Equals("BossMacho")) scope = &StatisticsSaveData::k_statScope_defeatMacho;
    else return true;
    if (Statistics::GetInstance()->GetIntValue(-1, *scope, 0) < 1) return IsAnyMissionDone();
    return true;
}

// @0x4c7a44: every 180 meters of a straight template the rooms run grow, and with them the probability by the interval's increment
void EdgeProbabilityMgr::OnAdvanceLevelTemplateList() {
    GameplayTemplateInstance *g = GameLevel::GetInstance()->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance();
    if (g->GetBifurcation() != Bifurcation_Center) return;
    if (!GameLevel::GetInstance()->IsGameplayStateCreated()) return;
    if (GameLevel::GetInstance()->GetGameplayMode() == 4) return;
    if (m_isLocationDependent && BackgroundMgr::GetInstance()->GetLocation() != m_location) return;
    m_distanceRunned += (unsigned)g->GetDefinition()->GetSizeY();
    while (!((float)m_distanceRunned < 180.0f)) {
        m_distanceRunned -= 180;
        m_roomsRunned++;
        if (m_currentInterval < (int)m_probabilityIntervals.size() && (unsigned)m_probabilityIntervals[m_currentInterval]->roomAmount <= m_roomsRunned) {
            m_roomsRunned = 0;
            m_initialProbability += m_probabilityIntervals[m_currentInterval]->increment;
            m_currentInterval++;
        }
    }
}
