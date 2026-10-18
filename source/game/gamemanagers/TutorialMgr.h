// source/game/gamemanagers/TutorialMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_TUTORIALMGR_H
#define SOURCE_GAME_GAMEMANAGERS_TUTORIALMGR_H

#include <map>
#include "jsingleton.h"
#include "core/pstring.h"
#include "common/utils/date.h"
#include "game/utils/GameSaveable.h"

// The tutorials the player has been through: by name (with the name of what they were shown with) and by mission
struct TutorialMgrSaveData {
    struct TutorialElement {
        jet::String name;
        bool finished;
        TutorialElement() : finished(false) {}
    };
    std::map<jet::String, TutorialElement> tutorials;  // 0x0
    std::map<unsigned, bool> missions;                 // 0x18
    Date date;                                         // 0x30
};

// The ids of the tutorials (their names are their own spelling)
struct ETutorialId {
    static const jet::String k_firstPlay, k_bossVectorDrone, k_bossVectorMissile, k_sledge, k_buySomething, k_weeklyContest1, k_weeklyContest2,
        k_weeklyContest3, k_perksX10, k_perksX20, k_perksX30, k_perksX50, k_perksX100;
    static const jet::String *const k_arrayTutorials[13];
};

// Saved as the record "TutorialMgr". The gui objects tutorials show and the weekly contest dialogs come with their parts.
class TutorialMgr : public Singleton<TutorialMgr>, public GameSaveable<TutorialMgrSaveData> {  // sizeof 0x5C
public:
    static const jet::String k_saveRecordName;
    // @0xc8ae6c: the bananas a player has to have for the shop's tutorial to start
    static const int k_minBananas = 150;
    static const int k_saveGameVersion = 1, k_signatureV1 = 0xaa0007;
    TutorialMgr() { ResetData(m_data); }
    virtual ~TutorialMgr() {}
    void Reset() { ResetData(m_data); }
    virtual void ResetData(TutorialMgrSaveData&) const;
    virtual void SaveDataInStream(jet::stream::IStreamW&, TutorialMgrSaveData&) const;
    virtual bool LoadDataFromStream(jet::stream::IStream&, TutorialMgrSaveData&) const;
    const jet::String &GetTutorialName(const jet::String &id) { return m_data.tutorials[id].name; }
    // a tutorial the data does not know counts as finished
    bool IsTutorialFinished(const jet::String &id) const;
    void SetTutorialFinished(const jet::String &id) { m_data.tutorials[id].finished = true; }
    void RegisterMission(unsigned id);
    void SetTutorialMission(unsigned id, bool b) { m_data.missions[id] = b; }
    bool IsMissionFinished(unsigned id) const;
    // @0x4fb4c4: the dialog of a level of the multiplier (x10, x20, x30, x50, x100) is shown the first time it is reached
    void ShowMuliplierPerkDialog();
private:
    bool IsDesactivated(const jet::String &id) const { return id.Equals(ETutorialId::k_buySomething); }
    void SerializeV1(jet::stream::IStreamW&, TutorialMgrSaveData&) const;
    bool DeserializeV1(jet::stream::IStream&, TutorialMgrSaveData&) const;
};

#endif
