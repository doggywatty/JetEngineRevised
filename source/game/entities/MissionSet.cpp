// source/game/entities/MissionSet.cpp
#include "MissionSet.h"
#include "Mission.h"
#include "project.h"

const jet::String MissionSet::k_tmplName("MissionSet");

// @0x57e43c
MissionSet::MissionSet(const clara::Template *t) : clara::DataEntity(t), m_isInfiniteMissionLooper(false) {}
MissionSet::~MissionSet() {}

// @0x57e4e0: the missions of the "Missions" paths
void MissionSet::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("IsInfiniteMissionLooper"), m_isInfiniteMissionLooper);
    const clara::Param *list = FindParamByName(jet::String("Missions"));
    for (unsigned i = 0; list && i < list->GetComponentCount(); i++)
        m_missions.push_back(static_cast<Mission*>(clara::Project::GetInstance()->FindEntityByPath(list->GetAsPath(i))));
}
