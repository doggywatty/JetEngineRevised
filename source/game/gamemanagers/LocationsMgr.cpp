// source/game/gamemanagers/LocationsMgr.cpp
#include "LocationsMgr.h"
#include "project.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/entities/LocationConfig.h"
#include "game/Statistics.h"

const std::vector<int> LocationsMgr::s_emptyLocationArray;

// @0x4874d0
LocationInfoEntity *LocationsMgr::GetLocationInfo(int location) const {
    std::map<int, LocationInfoEntity*>::const_iterator i = m_info.find(location);
    return i == m_info.end() ? 0 : i->second;
}

// @0x48752c
const std::vector<int> &LocationsMgr::GetAssociatedLocationsFor(int location) const {
    LocationInfoEntity *e = GetLocationInfo(location);
    return e ? e->GetAssociatedLocations() : s_emptyLocationArray;
}

// @0x487560: the entities of the data; a location's statistics are copied to its associated ones when theirs are empty
void LocationsMgr::ResetLocationInfo() {
    ClearLocationInfo();
    clara::Project *p = clara::Project::GetInstance();
    for (unsigned i = 0; i < p->GetEntityCount(); i++) {
        clara::DataEntity *e = p->GetEntity(i);
        if (e && e->GetTemplateName().Equals(LocationInfoEntity::k_tmplName)) {
            LocationInfoEntity *l = static_cast<LocationInfoEntity*>(e);
            m_info[l->GetLocationId()] = l;
        }
    }
    for (std::map<int, LocationInfoEntity*>::iterator it = m_info.begin(); it != m_info.end(); ++it) {
        LocationInfoEntity *l = it->second;
        const std::vector<int> &a = l->GetAssociatedLocations();
        for (size_t k = 0; k < a.size(); k++)
            if (l->GetLocationId() != a[k] && Statistics::HasInstance()) Statistics::GetInstance()->MatchStatisticsIfEmpty(l->GetLocationId(), a[k]);
    }
}

// @0x477710
LocationConfig *LocationsMgr::GetLocationConfig(int location) {
    std::map<int, LocationConfig*>::iterator i = m_config.find(location);
    return i == m_config.end() ? 0 : i->second;
}

// @0x487764
void LocationsMgr::ResetLocationConfig() {
    ClearLocationConfig();
    clara::Project *p = clara::Project::GetInstance();
    for (unsigned i = 0; i < p->GetEntityCount(); i++) {
        clara::DataEntity *e = p->GetEntity(i);
        if (e && e->GetTemplateName().Equals(LocationConfig::k_tmplName)) {
            LocationConfig *c = static_cast<LocationConfig*>(e);
            m_config[c->GetLocationId()] = c;
        }
    }
}

// @0x487880
const jet::String &LocationsMgr::GetMainMenuMusicLabel(int location) { return GetLocationConfig(location)->GetMainMenuMusic(); }
// @0x4878ac
const jet::String &LocationsMgr::GetMainMenuAmbientLabel(int location) { return GetLocationConfig(location)->GetMainMenuAmbient(); }
