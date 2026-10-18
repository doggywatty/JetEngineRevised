// source/game/challenges/ChallengeMgr.h
#ifndef SOURCE_GAME_CHALLENGES_CHALLENGEMGR_H
#define SOURCE_GAME_CHALLENGES_CHALLENGEMGR_H

// Same zero-includes PDB-extraction gap as every other header this session has touched.
#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "ChallengeInfo.h"
class Challenge;
class OnlineUser;

class ChallengeMgr : public Singleton<ChallengeMgr> {  // sizeof 0x54
public:
    ChallengeMgr();
    virtual ~ChallengeMgr();
    void Reset();
    void Update(int);
    OnlineUser *OnlineUser_Get();
    const jet::String &OnlineUser_GetName();
    void OnlineUser_Add(const ChallengeInfo&);
    bool OnlineUser_IsSet();
    Challenge *GetChallenge();
    Challenge *GetChallengeType(const jet::String&);
    void Challenge_Compute();
    bool Challenge_IsActive();
    bool Challenge_IsCompleted();
    void ClearCurrentChallenge();
    void RefreshChallenges();
    bool IsReady();
    bool CheckStartCurrentChallenge();
    void OnUnloadClaraData();
    ChallengeInfo &GetChallengeToSendInfo();
    jet::String GetChallengeCostumeName();
    jet::String GetChallengeTauntAnimixerState();
private:
    void _RefreshChallenges();
    void _ClearCurrentChallenge();
    void ValidateChallengeExistence(const jet::String&);
    std::vector<ChallengeInfo,std::allocator<ChallengeInfo> > m_onlineUserList;  // 0x4
    Challenge *m_challenge;  // 0x18
    std::map<jet::String,Challenge *,std::less<jet::String>,std::allocator<std::pair<jet::String const ,Challenge *> > > m_challengesMap;  // 0x1C
    bool m_deletingChallenge;  // 0x38
    bool m_refreshChallenge;  // 0x39
    bool m_challengeStarted;  // 0x3A
    static const int k_refreshChallengesTime;
    ChallengeInfo m_challengeToSendInfo;  // 0x3C
};

#endif
