// source/game/gamemanagers/AchievementsMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_ACHIEVEMENTSMGR_H
#define SOURCE_GAME_GAMEMANAGERS_ACHIEVEMENTSMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "game/utils/GameSaveable.h"

// What is saved of the achievements: the ones done, whether their prize was taken and whether their notification was shown
struct AchievementsMgrSaveData {
    struct SAchievementInfo {  // 0x8
        jet::String id;        // 0x0
        bool prizeClaimed;     // 0x4
        bool igNotifyShown;    // 0x5
        SAchievementInfo() : prizeClaimed(false), igNotifyShown(true) {}
    };
    std::vector<SAchievementInfo> achievements;
};

// The achievements done, saved as the record "AchievementsMgr"
class AchievementsMgr : public Singleton<AchievementsMgr>, public GameSaveable<AchievementsMgrSaveData> {  // sizeof 0x30
public:
    static const int k_signatureV1 = 0xaa0002, k_signatureV2 = 0xaa0003, k_saveGameVersion = 2;
    static const jet::String k_saveRecordName;
    AchievementsMgr() {}
    virtual ~AchievementsMgr() {}
    void Reset();                                              // @0x540514
    virtual void ResetData(AchievementsMgrSaveData&) const;    // @0x5405ec
    virtual void SaveDataInStream(jet::stream::IStreamW&, AchievementsMgrSaveData&) const;  // @0x540634
    virtual bool LoadDataFromStream(jet::stream::IStream&, AchievementsMgrSaveData&) const; // @0x540680
    void OnAchievementJustCompleted(const jet::String&);       // @0x5409cc
    void OnStartGameShowPendingIGNotifies();                   // @0x540a80
    void MarkIGNotifyShownFor(const jet::String&);             // @0x540af4
    bool HasAnyPrizeToClaim();                                 // @0x540b7c
    bool WasPrizeClaimedFor(const jet::String&);               // @0x540bc0
    void SetPrizeClaimedFor(const jet::String&);               // @0x540c14
    unsigned GetCountCompletedAchievements();                  // @0x540cf4
private:
    void SerializeV1(jet::stream::IStreamW&, AchievementsMgrSaveData&) const;  // @0x5406ec
    bool DeserializeV1(jet::stream::IStream&, AchievementsMgrSaveData&) const; // @0x540778
    void SerializeV2(jet::stream::IStreamW&, AchievementsMgrSaveData&) const;  // @0x540854
    bool DeserializeV2(jet::stream::IStream&, AchievementsMgrSaveData&) const; // @0x5408ec
};

#endif
