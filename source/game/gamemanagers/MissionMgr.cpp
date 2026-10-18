// source/game/gamemanagers/MissionMgr.cpp
#include "MissionMgr.h"
#include "param.h"
#include "project.h"
#include "game/GameLevel.h"

const jet::String MissionMgr::k_saveRecordName("MissionMgr");

// @0x488818
void MissionMgr::ResetData(MissionMgrSaveData &d) const {
    d.multiplierLevel = 0;
    d.multiplierPoints = 0;
    d.u8 = -1;
    d.activeMissions.clear();
    d.missionSets.clear();
    d.repeatedMissions.clear();
    d.v30.clear();
    d.completedMissionIds.clear();
}

// @0x48888c
void MissionMgr::SaveDataInStream(jet::stream::IStreamW &s, MissionMgrSaveData &d) const {
    int version = k_saveGameVersion;
    s << version;
    SerializeV2(s, d);
}

// @0x4788d8
bool MissionMgr::LoadDataFromStream(jet::stream::IStream &s, MissionMgrSaveData &d) const {
    int version = 0;
    s >> version;
    if (version == 1) return DeserializeV1(s, d);
    if (version == 2) return DeserializeV2(s, d);
    return false;
}

static void WriteHead(jet::stream::IStreamW &s, const MissionMgrSaveData &d, int sig) {
    s << sig << d.multiplierLevel << d.multiplierPoints << d.u8;
    unsigned n = (unsigned)d.activeMissions.size();
    s << n;
    for (size_t i = 0; i < d.activeMissions.size(); i++) {
        jet::WriteString(s, d.activeMissions[i].id);
        s << d.activeMissions[i].a << d.activeMissions[i].state;
    }
}

static void WriteStrings(jet::stream::IStreamW &s, const std::vector<jet::String> &v) {
    unsigned n = (unsigned)v.size();
    s << n;
    for (size_t i = 0; i < v.size(); i++) jet::WriteString(s, v[i]);
}

// @0x488944
void MissionMgr::SerializeV1(jet::stream::IStreamW &s, MissionMgrSaveData &d) const {
    WriteHead(s, d, k_signatureV1);
    unsigned n = (unsigned)d.missionSets.size();
    s << n;
    for (size_t i = 0; i < d.missionSets.size(); i++) { jet::WriteString(s, d.missionSets[i].id); s << d.missionSets[i].index; }
    n = (unsigned)d.repeatedMissions.size();
    s << n;
    for (size_t i = 0; i < d.repeatedMissions.size(); i++) { jet::WriteString(s, d.repeatedMissions[i].id); s << d.repeatedMissions[i].count; }
    WriteStrings(s, d.v30);
    WriteStrings(s, d.v3c);
}

// @0x488bcc
void MissionMgr::SerializeV2(jet::stream::IStreamW &s, MissionMgrSaveData &d) const {
    WriteHead(s, d, k_signatureV2);
    unsigned n = (unsigned)d.missionSets.size();
    s << n;
    for (size_t i = 0; i < d.missionSets.size(); i++) {
        jet::WriteString(s, d.missionSets[i].id);
        s << d.missionSets[i].index;
        jet::WriteString(s, d.missionSets[i].next);
    }
    n = (unsigned)d.repeatedMissions.size();
    s << n;
    for (size_t i = 0; i < d.repeatedMissions.size(); i++) { jet::WriteString(s, d.repeatedMissions[i].id); s << d.repeatedMissions[i].count; }
    WriteStrings(s, d.v30);
    WriteStrings(s, d.completedMissionIds);
}

static void ReadStrings(jet::stream::IStream &s, std::vector<jet::String> &v) {
    int n = 0;
    s >> n;
    v.assign(n, jet::String());
    for (size_t i = 0; i < v.size(); i++) s >> v[i];
}

static void ReadHead(jet::stream::IStream &s, MissionMgrSaveData &d) {
    s >> d.multiplierLevel >> d.multiplierPoints >> d.u8;
    int n = 0;
    s >> n;
    d.activeMissions.assign(n, MissionMgrSaveData::ActiveMissionInfo());
    for (int i = 0; i < n; i++) s >> d.activeMissions[i].id >> d.activeMissions[i].a >> d.activeMissions[i].state;
}

static void ReadRepeated(jet::stream::IStream &s, MissionMgrSaveData &d) {
    int n = 0;
    s >> n;
    d.repeatedMissions.assign(n, MissionMgrSaveData::RepeatedMissionInfo());
    for (int i = 0; i < n; i++) s >> d.repeatedMissions[i].id >> d.repeatedMissions[i].count;
}

// @0x488e94: the old format has no "next" of the sets and keeps the ids that become the completed ones in v3c
bool MissionMgr::DeserializeV1(jet::stream::IStream &s, MissionMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    m_loadedV1 = true;
    ReadHead(s, d);
    int n = 0;
    s >> n;
    d.missionSets.assign(n, MissionMgrSaveData::SaveMissionSetInfo());
    for (int i = 0; i < n; i++) s >> d.missionSets[i].id >> d.missionSets[i].index;
    ReadRepeated(s, d);
    ReadStrings(s, d.v30);
    ReadStrings(s, d.v3c);
    return true;
}

// @0x489194
bool MissionMgr::DeserializeV2(jet::stream::IStream &s, MissionMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV2) return false;
    ReadHead(s, d);
    int n = 0;
    s >> n;
    d.missionSets.assign(n, MissionMgrSaveData::SaveMissionSetInfo());
    for (int i = 0; i < n; i++) s >> d.missionSets[i].id >> d.missionSets[i].index >> d.missionSets[i].next;
    ReadRepeated(s, d);
    ReadStrings(s, d.v30);
    ReadStrings(s, d.completedMissionIds);
    return true;
}

// @0x48a530
bool MissionMgr::WasMissionIdCompleted(const jet::String &id) const {
    for (size_t i = 0; i < m_data.completedMissionIds.size(); i++)
        if (id.Equals(m_data.completedMissionIds[i])) return true;
    return false;
}

// @0x4f872c
void MissionMgr::MissionMultiplierLevel_Load(const clara::Param *p) {
    m_multiplierLevels.clear();
    for (unsigned i = 0; i < p->GetComponentCount(); i++) {
        jet::core::ProtectedStorage<int> v;
        v.SetValue((int)p->GetAsU32(i));
        m_multiplierLevels.push_back(v);
    }
}

// @0x4f885c
int MissionMgr::MissionMultiplierLevel_GetValue(unsigned level) {
    if (m_multiplierLevels.empty())
        if (clara::DataEntity *def = GameLevel::GetInstance() ? GameLevel::GetInstance()->GetGameplayDef() : 0)
            if (const clara::Param *p = def->FindParamByName(jet::String("MissionMultiplierLvlArray"))) MissionMultiplierLevel_Load(p);
    if (m_multiplierLevels.empty()) return 0;
    return (level < m_multiplierLevels.size() ? m_multiplierLevels[level] : m_multiplierLevels.back()).GetValue();
}
