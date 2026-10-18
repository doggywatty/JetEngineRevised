// source/game/challenges/ChallengeInfo.h
#ifndef SOURCE_GAME_CHALLENGES_CHALLENGEINFO_H
#define SOURCE_GAME_CHALLENGES_CHALLENGEINFO_H

class ChallengeInfo {  // sizeof 0x18
public:
    static const jet::String k_typeScore;
    static const jet::String k_typeBananas;
    static const jet::String k_typeDistance;
    bool m_isReadDataValid;  // 0x0
    jet::String m_socialUserId;  // 0x4
    jet::String m_challengeType;  // 0x8
    int m_challengeValue;  // 0xC
    jet::String m_tauntClaraNode;  // 0x10
    jet::String m_costumeClaraName;  // 0x14
};

#endif
