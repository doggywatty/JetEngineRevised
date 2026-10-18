// source/game/gamemanagers/MissionMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_MISSIONMGR_H
#define SOURCE_GAME_GAMEMANAGERS_MISSIONMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/protectedstorage.h"
#include "core/pstring.h"
#include "game/utils/GameSaveable.h"
#include "core/protectedstorage.h"

// What is saved of the missions: the active ones, the missions sets in progress, the repeated ones and the completed ids
class Mission;
class MissionSet;
namespace clara { class DataEntity; class Param; }


struct MissionMgrSaveData {
    struct ActiveMissionInfo { jet::String id; int a; int state; ActiveMissionInfo() : a(0), state(0) {} };    // 0xC
    struct SaveMissionSetInfo { jet::String id; unsigned index; jet::String next; SaveMissionSetInfo() : index(0) {} };  // 0xC
    struct RepeatedMissionInfo { jet::String id; int count; RepeatedMissionInfo() : count(0) {} };           // 0x8
    unsigned multiplierLevel;                          // 0x0
    unsigned multiplierPoints;                         // 0x4
    int u8;                                            // 0x8: the set of the initial order the player is at (-1 for none): the set of the initial order the player is at (-1 for none)
    std::vector<ActiveMissionInfo> activeMissions;     // 0xC
    std::vector<SaveMissionSetInfo> missionSets;       // 0x18
    std::vector<RepeatedMissionInfo> repeatedMissions; // 0x24
    std::vector<jet::String> v30;                      // 0x30: the repeated missions done lately
    std::vector<jet::String> v3c;                      // 0x3C: the names of the completed missions of the old save
    std::vector<jet::String> completedMissionIds;      // 0x48
    MissionMgrSaveData() : multiplierLevel(0), multiplierPoints(0), u8(-1) {}
};

// The missions of the player and their progress, saved as the record "MissionMgr": up to three active missions (each one
// active (1), done and not yet shown (3), done and shown (2) or none (0)), the sets of missions that go on one after the
// other, the missions repeated when the sets are done, and the levels of the score multiplier the missions' points fill.
class MissionMgr : public Singleton<MissionMgr>, public GameSaveable<MissionMgrSaveData> {  // sizeof 0xE8
public:
    static const int k_signatureV1 = 0xaa0006, k_signatureV2 = 0xbb0006, k_saveGameVersion = 2;
    static const int k_activeMissions = 3;
    static const jet::String k_saveRecordName;
    MissionMgr() : m_loadedV1(false), m_infiniteSet(0), m_wasReset(false) {}
    virtual ~MissionMgr() {}
    virtual void ResetData(MissionMgrSaveData &d) const;
    virtual void SaveDataInStream(jet::stream::IStreamW &s, MissionMgrSaveData &d) const;
    virtual bool LoadDataFromStream(jet::stream::IStream &s, MissionMgrSaveData &d) const;
    void Reset() { ResetData(m_data); }
    // @0x4f66f4: the level was loaded: the sets of missions it has, the infinite one apart
    void Reset(std::vector<Mission*> &missions, std::vector<MissionSet*> &sets);
    // the values GameplayDef::NotifyParamChanged @0x568a40 gives the manager from the gameplay definition
    void LoadGameplayDef(clara::DataEntity *def);
    void ComputeGameMissions();                                         // @0x4f7acc
    void RefreshGameMissions();                                         // @0x4f6bf8
    void ReactivateGameMissions();                                      // @0x4f6c90
    void OnStartGameShowPendingIGNotifies();                            // @0x4f6d38
    void OnMissionJustCompleted(const jet::String &missionName, int);  // @0x4f81fc
    void MarkIGNotifyShownFor(const jet::String &missionName);         // @0x4f8314
    bool WasMissionCompletedOnce(const jet::String &name) const;       // @0x4f838c
    void SetInitialMissionSetOrder(clara::Param*);                      // @0x4f86ac
    void MissionMultiplierLevel_Load(clara::Param*);                    // @0x4f872c
    unsigned MissionMultiplierLevel_GetValue(unsigned level) const;     // @0x4f885c
    void AddMissionPoints(unsigned);                                    // @0x4f888c
    void RemoveCompletedMission(int slot);                              // @0x4f891c
    void ForceActiveMissionUpdate();                                    // @0x4f8a34
    unsigned ComputeMultiplierLevelIfAdding(int points) const;          // @0x4f8af8
    unsigned GetMultiplierLevel() const { return m_data.multiplierLevel; }
    unsigned GetMultiplierLevelPoints() const { return m_data.multiplierPoints; }
    unsigned GetActiveMissionCount() const { return (unsigned)m_data.activeMissions.size(); }
    const MissionMgrSaveData::ActiveMissionInfo &GetActiveMissionInfo(unsigned i) const { return m_data.activeMissions[i]; }
    bool WasMissionIdCompleted(const jet::String &id) const;
    // @0x4f872c: the points each level of the multiplier asks for, from the gameplay definition's array
    void MissionMultiplierLevel_Load(const clara::Param*);
    // @0x4f885c: the points to go from the level to the next one (the last level's for any level after it). The .so receives
    // the array when the gameplay definition notifies its parameter changed (not ported): it is read from it on first use
    int MissionMultiplierLevel_GetValue(unsigned level);
private:
    int GetActiveMissionsEmtpySlot();                                   // @0x4f78f8
    void SerializeV1(jet::stream::IStreamW &s, MissionMgrSaveData &d) const;
    void SerializeV2(jet::stream::IStreamW &s, MissionMgrSaveData &d) const;
    bool DeserializeV1(jet::stream::IStream &s, MissionMgrSaveData &d) const;
    bool DeserializeV2(jet::stream::IStream &s, MissionMgrSaveData &d) const;
    void RemoveUnfoundMissions();                                       // @0x4f7c64
    void OnStartNewMissionFrom(MissionSet&, int slot);                  // @0x4f7cf8
    bool OnStartRepeatedMission(int slot);                              // @0x4f7e18
    bool RepeatMission_WasRecentlyUsed(const Mission&) const;           // @0x4f8118
    void RepeatMission_MarkAsRecentlyUsed(const Mission&);              // @0x4f8168
    MissionSet *GetInitialOrderMissionSet();                            // @0x4f83dc
    MissionSet *GetRandomMissionSet(bool onlyInactive);                 // @0x4f84e8
    bool IsAnyActiveMissionIn(const MissionSet&) const;                 // @0x4f8608
    unsigned MissionsGoldToUP1Transition(MissionSet*, unsigned) const;  // @0x4f7894
    void StoreMissionIdsGoldToUP1();                                    // @0x4f6b2c
    MissionMgrSaveData::SaveMissionSetInfo *FindSetInfo(const jet::String &name);
    mutable bool m_loadedV1;  // 0xB0
    std::vector<MissionSet*> m_initialOrder;                            // 0xB8
    MissionSet *m_infiniteSet;                                          // 0xC4: the set the missions go on in when the others are done
    bool m_wasReset;                                                    // 0xC8
    std::vector<jet::core::ProtectedStorage<int> > m_multiplierLevels;  // 0xCC: the points each level of the multiplier takes
};

#endif
