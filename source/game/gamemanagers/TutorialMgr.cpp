// source/game/gamemanagers/TutorialMgr.cpp
#include "TutorialMgr.h"
#include "MissionMgr.h"

const jet::String ETutorialId::k_firstPlay("k_firstPlay"), ETutorialId::k_bossVectorDrone("k_bossVectorDrone"), ETutorialId::k_bossVectorMissile("k_bossVectorMissile"),
    ETutorialId::k_sledge("k_sledge"), ETutorialId::k_buySomething("k_buySomething"), ETutorialId::k_weeklyContest1("k_weeklyContest1"),
    ETutorialId::k_weeklyContest2("k_weeklyContest2"), ETutorialId::k_weeklyContest3("k_weeklyContest3"), ETutorialId::k_perksX10("k_perksX10"),
    ETutorialId::k_perksX20("k_perksX20"), ETutorialId::k_perksX30("k_perksX30"), ETutorialId::k_perksX50("k_perksX50"), ETutorialId::k_perksX100("k_perksX100");
const jet::String *const ETutorialId::k_arrayTutorials[13] = { &k_firstPlay, &k_bossVectorDrone, &k_bossVectorMissile, &k_sledge, &k_buySomething, &k_weeklyContest1,
    &k_weeklyContest2, &k_weeklyContest3, &k_perksX10, &k_perksX20, &k_perksX30, &k_perksX50, &k_perksX100 };
const jet::String TutorialMgr::k_saveRecordName("TutorialMgr");

// @0x4fab9c
void TutorialMgr::ResetData(TutorialMgrSaveData &d) const {
    d.tutorials.clear();
    for (const jet::String *id : ETutorialId::k_arrayTutorials) d.tutorials[*id] = TutorialMgrSaveData::TutorialElement();
    d.date.SetToNow();
}

// @0x4fac48
void TutorialMgr::SaveDataInStream(jet::stream::IStreamW &s, TutorialMgrSaveData &d) const {
    int v = k_saveGameVersion;
    s << v;
    SerializeV1(s, d);
}

// @0x4fac94
bool TutorialMgr::LoadDataFromStream(jet::stream::IStream &s, TutorialMgrSaveData &d) const {
    int v = 0;
    s >> v;
    return v == 1 && DeserializeV1(s, d);
}

// @0x4face0
void TutorialMgr::SerializeV1(jet::stream::IStreamW &s, TutorialMgrSaveData &d) const {
    int sig = k_signatureV1;
    s << sig;
    unsigned n = (unsigned)d.tutorials.size();
    s << n;
    for (auto &kv : d.tutorials) {
        jet::WriteString(s, kv.first);
        jet::WriteString(s, kv.second.name);
        s << kv.second.finished;
    }
    n = (unsigned)d.missions.size();
    s << n;
    for (auto &kv : d.missions) s << kv.first << kv.second;
    char raw[0x1c] = {};
    memcpy(raw, &d.date, sizeof(Date) < 0x1c ? sizeof(Date) : 0x1c);
    s.Write(raw, 0x1c);
}

// @0x4fadd0
bool TutorialMgr::DeserializeV1(jet::stream::IStream &s, TutorialMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    unsigned n = 0;
    s >> n;
    for (unsigned i = 0; i < n; i++) {
        jet::String id, name;
        bool finished;
        s >> id >> name;
        s >> finished;
        TutorialMgrSaveData::TutorialElement &e = d.tutorials[id];
        e.name = name;
        e.finished = finished;
    }
    s >> n;
    for (unsigned i = 0; i < n; i++) {
        unsigned id;
        bool b;
        s >> id >> b;
        d.missions[id] = b;
    }
    char raw[0x1c] = {};
    s.Read(raw, 0x1c);
    memcpy(&d.date, raw, sizeof(Date) < 0x1c ? sizeof(Date) : 0x1c);
    return true;
}

// @0x4fb004
bool TutorialMgr::IsTutorialFinished(const jet::String &id) const {
    if (IsDesactivated(id)) return true;
    auto it = m_data.tutorials.find(id);
    return it == m_data.tutorials.end() || it->second.finished;
}

// @0x4fb25c
void TutorialMgr::RegisterMission(unsigned id) {
    if (m_data.missions.find(id) == m_data.missions.end()) m_data.missions[id] = false;
}

// @0x4fb2ec
bool TutorialMgr::IsMissionFinished(unsigned id) const {
    auto it = m_data.missions.find(id);
    return it == m_data.missions.end() || it->second;
}

// @0x4fb4c4: the multiplier the player is at is one of the levels that have a dialog (it explains the perk of the level):
// the first time, the dialog is shown and the tutorial is finished. (TutorialMgr::ShowGuiObject, the popup, is not ported:
// only the tutorial is finished.)
void TutorialMgr::ShowMuliplierPerkDialog() {
    const jet::String *id = 0;
    switch ((int)MissionMgr::GetInstance()->GetMultiplierLevel() + 1) {
        case 10: id = &ETutorialId::k_perksX10; break;
        case 20: id = &ETutorialId::k_perksX20; break;
        case 30: id = &ETutorialId::k_perksX30; break;
        case 50: id = &ETutorialId::k_perksX50; break;
        case 100: id = &ETutorialId::k_perksX100; break;
        default: return;
    }
    if (IsTutorialFinished(*id)) return;
    // ShowGuiObject(*id, false, popup type 1)
    SetTutorialFinished(*id);
}
