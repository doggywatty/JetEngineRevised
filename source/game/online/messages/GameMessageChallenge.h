// source/game/online/messages/GameMessageChallenge.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECHALLENGE_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECHALLENGE_H

class GameMessageChallenge : public GameMessage {  // sizeof 0x54
public:
    GameMessageChallenge(const social::MessageIn*);
    GameMessageChallenge(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, GameMessageChallenge::ChallengeMessageType);
    virtual ~GameMessageChallenge();
    virtual social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> CreateMsg(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    GameMessageChallenge::ChallengeMessageType GetMessageChallengeType();
    void SetChallengeFinishedData(bool, const ChallengeInfo&);
    bool IsChallengeCompleted();
    const ChallengeInfo &GetChallengeInfo();
private:
    void WriteTo(Json::Value&, const ChallengeInfo&);
    void ReadChallengeInfoFrom(Json::Value&);
    static const char *k_param_challengeType;
    static const char *k_param_challengeValue;
    static const char *k_param_challengeClaraNode;
    static const char *k_param_challengeCostume;
    static const char *k_param_challengeCompleted;
    GameMessageChallenge::ChallengeMessageType m_challengeMessageType;  // 0x34
    bool m_challengeCompleted;  // 0x38
    ChallengeInfo m_challengeInfo;  // 0x3C
};

#endif
