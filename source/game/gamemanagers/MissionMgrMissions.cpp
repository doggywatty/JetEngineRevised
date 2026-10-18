// source/game/gamemanagers/MissionMgrMissions.cpp
//
// The missions' side of MissionMgr (MissionMgr.cpp has the save). Not ported: the online publication of the multiplier level
// (OnlinePlayerData::PublishDespicableMultiplierActivity) and the CRM point cut (GameCrmMgr::LevelUpPointCut).
#include "MissionMgr.h"
#include "core/util.h"
#include "entity.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/Statistics.h"
#include "game/entities/Mission.h"
#include "game/entities/MissionSet.h"
#include "param.h"
#include "project.h"

static clara::DataEntity *Find(const jet::String &name) { return clara::Project::GetInstance()->FindEntityByName(name); }

// the values the gameplay definition gives (GameplayDef::NotifyParamChanged @0x568a40): the points of the levels of the
// multiplier and the sets the missions start with
void MissionMgr::LoadGameplayDef(clara::DataEntity *def) {
    if (!def) return;
    if (clara::Param *p = def->FindParamByName(jet::String("MissionMultiplierLvlArray"))) MissionMultiplierLevel_Load(p);
    if (clara::Param *p = def->FindParamByName(jet::String("MissionsInitialOrder"))) SetInitialMissionSetOrder(p);
}

MissionMgrSaveData::SaveMissionSetInfo *MissionMgr::FindSetInfo(const jet::String &name) {
    for (size_t i = 0; i < m_data.missionSets.size(); i++)
        if (m_data.missionSets[i].id.Equals(name)) return &m_data.missionSets[i];
    return 0;
}

// @0x4f66f4: the sets of the level that the saved missions do not know start before their first mission; the infinite looper
// set is kept apart (and only counts when none of its missions is twice in it and it has more than five)
void MissionMgr::Reset(std::vector<Mission*> &missions, std::vector<MissionSet*> &sets) {
    if (m_wasReset) {
        m_wasReset = false;
        for (size_t i = 0; i < missions.size(); i++) missions[i]->OnReset();
    }
    m_infiniteSet = 0;
    for (size_t i = 0; i < sets.size(); i++) {
        MissionSet *set = sets[i];
        if (set->IsInfiniteMissionLooper()) {
            m_infiniteSet = set;
            continue;
        }
        if (FindSetInfo(set->GetName())) continue;
        MissionMgrSaveData::SaveMissionSetInfo info;
        info.id = set->GetName();
        info.index = (unsigned)-1;
        m_data.missionSets.push_back(info);
    }
    if (m_infiniteSet) {
        for (size_t i = 0; i < m_data.missionSets.size(); i++)
            if (m_data.missionSets[i].id.Equals(m_infiniteSet->GetName())) {
                m_data.missionSets.erase(m_data.missionSets.begin() + (long)i);
                break;
            }
        bool duplicated = false;
        if (m_infiniteSet->GetMissionCount() <= 5) {
            m_infiniteSet = 0;
        } else {
            for (unsigned i = 0; i + 1 < m_infiniteSet->GetMissionCount(); i++)
                for (unsigned j = i + 1; j < m_infiniteSet->GetMissionCount(); j++)
                    if (m_infiniteSet->GetMission(i)->GetName().Equals(m_infiniteSet->GetMission(j)->GetName())) duplicated = true;
            if (duplicated) m_infiniteSet = 0;
        }
    }
    if (m_loadedV1) {
        for (size_t i = 0; i < m_data.missionSets.size(); i++)
            m_data.missionSets[i].index = MissionsGoldToUP1Transition(static_cast<MissionSet*>(Find(m_data.missionSets[i].id)), m_data.missionSets[i].index);
        StoreMissionIdsGoldToUP1();
        m_loadedV1 = false;
    }
}

// @0x4f7894: the old saves counted the missions of the first version only
unsigned MissionMgr::MissionsGoldToUP1Transition(MissionSet *set, unsigned index) const {
    if (index == (unsigned)-1) return index;
    unsigned i = 0, seen = 0;
    for (; i < set->GetMissionCount(); i++)
        if (set->GetMission(i)->GetVersion() == 0 && ++seen > index) break;
    return i;
}

// @0x4f6b2c: the missions the old save says were done give their ids
void MissionMgr::StoreMissionIdsGoldToUP1() {
    for (size_t i = 0; i < m_data.v3c.size(); i++) {
        Mission *m = static_cast<Mission*>(Find(m_data.v3c[i]));
        if (!WasMissionIdCompleted(m->GetMissionIdString())) m_data.completedMissionIds.push_back(m->GetMissionIdString());
    }
}

// @0x4f6bf8: the missions that cannot be, the done ones whose points are not added yet, and the new ones that fill the slots
void MissionMgr::RefreshGameMissions() {
    RemoveUnfoundMissions();
    RemoveCompletedMission(-1);
    ComputeGameMissions();
    ReactivateGameMissions();
}

// @0x4f6c90: the missions of the slots count again (the active ones) or are marked done (those that were done)
void MissionMgr::ReactivateGameMissions() {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
        MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[i];
        if (info.state == 1) static_cast<Mission*>(Find(info.id))->OnAddedToActiveMission((int)i, info.a, false);
        if (info.state == 3) static_cast<Mission*>(Find(info.id))->OnMarkAsPreviouslyCompleted();
    }
}

// @0x4f6d38
void MissionMgr::OnStartGameShowPendingIGNotifies() {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++)
        if (m_data.activeMissions[i].state == 3) static_cast<Mission*>(Find(m_data.activeMissions[i].id))->ShowIngameNotification();
}

// @0x4f78f8: the slots are three; the first free one, or -1
int MissionMgr::GetActiveMissionsEmtpySlot() {
    while (m_data.activeMissions.size() < (size_t)k_activeMissions) m_data.activeMissions.push_back(MissionMgrSaveData::ActiveMissionInfo());
    for (int i = 0; i < k_activeMissions; i++)
        if (m_data.activeMissions[(size_t)i].state == 0) return i;
    return -1;
}

// @0x4f7acc: free slots get missions: the next of the initial order, of a set that has none active, of any set, or one of the
// infinite set; and the missions that are not there or cannot be seen go (without the social features all do)
void MissionMgr::ComputeGameMissions() {
    if (!Game::GetInstance()->AreSocialFeaturesEnabled()) {
        for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
            MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[i];
            if (info.state == 0) continue;
            Mission *m = dynamic_cast<Mission*>(Find(info.id));
            if (!m) {
                info.state = 0;
                continue;
            }
            if (m->CanBeSeenByUser()) continue;
            m->OnReset();
            m->UnregisterObservers();
            info.state = 0;
        }
    }
    for (int tries = 0; tries < 11; tries++) {
        const int slot = GetActiveMissionsEmtpySlot();
        if (slot < 0) return;
        MissionSet *set = GetInitialOrderMissionSet();
        if (!set) set = GetRandomMissionSet(true);
        if (!set) set = GetRandomMissionSet(false);
        if (set) {
            OnStartNewMissionFrom(*set, slot);
            continue;
        }
        if (!OnStartRepeatedMission(slot)) return;
    }
}

// @0x4f7c64
void MissionMgr::RemoveUnfoundMissions() {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
        MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[i];
        if (info.state == 0) continue;
        Mission *m = dynamic_cast<Mission*>(Find(info.id));
        if (!m || !m->IsActive()) info.state = 0;
    }
}

// @0x4f7cf8: the next mission of the set takes the slot; one that cannot be seen is passed over, if it is not a mandatory one
void MissionMgr::OnStartNewMissionFrom(MissionSet &set, int slot) {
    MissionMgrSaveData::SaveMissionSetInfo *info = FindSetInfo(set.GetName());
    if (!info) return;
    const unsigned next = info->index + 1;
    if (next >= set.GetMissionCount()) return;
    Mission *m = set.GetMission(next);
    if (m->CanBeSeenByUser()) {
        info->index = next;
        info->next = m->GetName();
        m_data.activeMissions[(size_t)slot].id = m->GetName();
        m_data.activeMissions[(size_t)slot].a = 0;
        m_data.activeMissions[(size_t)slot].state = 1;
        m->OnAddedToActiveMission(slot, 0, false);
        RepeatMission_MarkAsRecentlyUsed(*m);
        return;
    }
    if (m->IsMandatory()) return;
    info->index = next;
    info->next = m->GetName();
}

// @0x4f7e18: a mission of the infinite set that was not done lately takes the slot, and its condition asks for more each time
bool MissionMgr::OnStartRepeatedMission(int slot) {
    if (!m_infiniteSet) return false;
    std::vector<Mission*> candidates;
    for (unsigned i = 0; i < m_infiniteSet->GetMissionCount(); i++) {
        Mission *m = m_infiniteSet->GetMission(i);
        if (!RepeatMission_WasRecentlyUsed(*m)) candidates.push_back(m);
    }
    if (candidates.empty()) return false;
    Mission *m = candidates[(size_t)jet::core::Rand2(0, (int)candidates.size())];
    if (!m->CanBeSeenByUser()) return false;
    size_t found = m_data.repeatedMissions.size();
    for (size_t i = 0; i < m_data.repeatedMissions.size(); i++)
        if (m_data.repeatedMissions[i].id.Equals(m->GetName())) found = i;
    if (found == m_data.repeatedMissions.size()) {
        MissionMgrSaveData::RepeatedMissionInfo info;
        info.id = m->GetName();
        info.count = -1;
        m_data.repeatedMissions.push_back(info);
    }
    MissionMgrSaveData::RepeatedMissionInfo &rep = m_data.repeatedMissions[found];
    rep.count++;
    MissionMgrSaveData::ActiveMissionInfo &a = m_data.activeMissions[(size_t)slot];
    a.id = m->GetName();
    a.state = 1;
    a.a = rep.count;
    m->OnAddedToActiveMission(slot, rep.count, false);
    RepeatMission_MarkAsRecentlyUsed(*m);
    return true;
}

// @0x4f8118
bool MissionMgr::RepeatMission_WasRecentlyUsed(const Mission &m) const {
    for (size_t i = 0; i < m_data.v30.size(); i++)
        if (m_data.v30[i].Equals(m.GetName())) return true;
    return false;
}

// @0x4f8168: the last five missions started are kept
void MissionMgr::RepeatMission_MarkAsRecentlyUsed(const Mission &m) {
    if (RepeatMission_WasRecentlyUsed(m)) return;
    if (m_data.v30.size() > 4) {
        for (size_t i = 0; i < 4; i++) m_data.v30[i] = m_data.v30[i + 1];
        m_data.v30[4] = m.GetName();
    } else {
        m_data.v30.push_back(m.GetName());
    }
}

// @0x4f81fc: a mission was done: its slot is for the notification, the count of the statistics and the ids of the done go on
void MissionMgr::OnMissionJustCompleted(const jet::String &name, int) {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
        MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[i];
        if (!info.id.Equals(name)) continue;
        info.state = 3;
        Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_completedMissionsCount, 1);
        Mission *m = static_cast<Mission*>(Find(name));
        if (!WasMissionIdCompleted(m->GetMissionIdString())) m_data.completedMissionIds.push_back(m->GetMissionIdString());
        Game::GetInstance()->Save(false, false);
    }
}

// @0x4f8314: the notification of a done mission was shown, the mission waits for its points
void MissionMgr::MarkIGNotifyShownFor(const jet::String &name) {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
        MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[i];
        if (info.id.Equals(name) && info.state == 3) {
            info.state = 2;
            Game::GetInstance()->Save(false, false);
        }
    }
}

// @0x4f838c
bool MissionMgr::WasMissionCompletedOnce(const jet::String &name) const {
    for (size_t i = 0; i < m_data.v3c.size(); i++)
        if (m_data.v3c[i].Equals(name)) return true;
    return false;
}

// @0x4f83dc: the next set of the initial order, when its next mission can be seen
MissionSet *MissionMgr::GetInitialOrderMissionSet() {
    const unsigned next = (unsigned)(m_data.u8 + 1);
    if (next >= m_initialOrder.size()) return 0;
    MissionSet *set = m_initialOrder[next];
    MissionMgrSaveData::SaveMissionSetInfo *info = FindSetInfo(set->GetName());
    const unsigned index = info ? info->index : 0;
    if (index + 1 >= set->GetMissionCount()) return 0;
    if (!set->GetMission(index + 1)->CanBeSeenByUser()) return 0;
    m_data.u8++;
    return m_initialOrder[(size_t)m_data.u8];
}

// @0x4f84e8: one of the sets that have missions left (and, asked to, none active), by chance
MissionSet *MissionMgr::GetRandomMissionSet(bool onlyInactive) {
    std::vector<MissionSet*> candidates;
    for (size_t i = 0; i < m_data.missionSets.size(); i++) {
        MissionSet *set = dynamic_cast<MissionSet*>(Find(m_data.missionSets[i].id));
        if (!set) continue;
        if (m_data.missionSets[i].index + 1 >= set->GetMissionCount()) continue;
        if (onlyInactive && IsAnyActiveMissionIn(*set)) continue;
        candidates.push_back(set);
    }
    if (candidates.empty()) return 0;
    return candidates[(size_t)(((long long)jet::core::Rand() >> 2) % (long long)candidates.size())];
}

// @0x4f8608
bool MissionMgr::IsAnyActiveMissionIn(const MissionSet &set) const {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++) {
        if (m_data.activeMissions[i].state != 1) continue;
        for (unsigned j = 0; j < set.GetMissionCount(); j++)
            if (set.GetMission(j)->GetName().Equals(m_data.activeMissions[i].id)) return true;
    }
    return false;
}

// @0x4f86ac
void MissionMgr::SetInitialMissionSetOrder(clara::Param *order) {
    m_initialOrder.clear();
    for (unsigned i = 0; i < order->GetComponentCount(); i++)
        if (MissionSet *s = dynamic_cast<MissionSet*>(clara::Project::GetInstance()->FindEntityByPath(order->GetAsPath(i)))) m_initialOrder.push_back(s);
}

// @0x4f872c
void MissionMgr::MissionMultiplierLevel_Load(clara::Param *levels) {
    m_multiplierLevels.clear();
    for (unsigned i = 0; i < levels->GetComponentCount(); i++) {
        jet::core::ProtectedStorage<int> v;
        v.SetValue((int)levels->GetAsU32(i));
        m_multiplierLevels.push_back(v);
    }
}

// @0x4f885c: the points a level takes (the last level's for the ones after it)
unsigned MissionMgr::MissionMultiplierLevel_GetValue(unsigned level) const {
    if (m_multiplierLevels.empty()) return 0;
    return (unsigned)(level >= m_multiplierLevels.size() ? m_multiplierLevels.back().GetValue() : m_multiplierLevels[level].GetValue());
}

// @0x4f888c: the points fill the levels
void MissionMgr::AddMissionPoints(unsigned points) {
    m_data.multiplierPoints += points;
    while (m_data.multiplierPoints >= MissionMultiplierLevel_GetValue(m_data.multiplierLevel)) {
        const unsigned v = MissionMultiplierLevel_GetValue(m_data.multiplierLevel);
        if (v == 0) break;  // (the .so never has levels of no points)
        m_data.multiplierLevel++;
        m_data.multiplierPoints -= v;
        // OnlinePlayerData::PublishDespicableMultiplierActivity, GameCrmMgr::LevelUpPointCut
    }
}

// @0x4f891c: a done mission whose notification was shown is taken: its points are added and the slot is free
void MissionMgr::RemoveCompletedMission(int slot) {
    if (slot < 0) {
        for (size_t i = 0; i < m_data.activeMissions.size(); i++) RemoveCompletedMission((int)i);
        return;
    }
    if ((size_t)slot >= m_data.activeMissions.size()) return;
    MissionMgrSaveData::ActiveMissionInfo &info = m_data.activeMissions[(size_t)slot];
    if (info.state != 2) return;
    info.state = 0;
    AddMissionPoints(static_cast<Mission*>(Find(info.id))->GetMissionMultiplierPoints());
    Game::GetInstance()->Save(false, false);
}

// @0x4f8a34
void MissionMgr::ForceActiveMissionUpdate() {
    for (size_t i = 0; i < m_data.activeMissions.size(); i++)
        if (m_data.activeMissions[i].state == 1) static_cast<Mission*>(Find(m_data.activeMissions[i].id))->ForceConditionUpdate();
}

// @0x4f8af8: the level the multiplier would be at with the points added (counting from one)
unsigned MissionMgr::ComputeMultiplierLevelIfAdding(int add) const {
    unsigned points = m_data.multiplierPoints + (unsigned)add, level = m_data.multiplierLevel;
    while (points >= MissionMultiplierLevel_GetValue(level)) {
        const unsigned v = MissionMultiplierLevel_GetValue(level);
        if (v == 0) break;
        points -= v;
        level++;
    }
    return level + 1;
}
