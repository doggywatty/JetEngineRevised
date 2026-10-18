// source/game/entities/ChallengePrize.h
#ifndef SOURCE_GAME_ENTITIES_CHALLENGEPRIZE_H
#define SOURCE_GAME_ENTITIES_CHALLENGEPRIZE_H

class ChallengePrize : public GameEntity {  // sizeof 0x9C
public:
    ChallengePrize(const clara::Template*);
    virtual ~ChallengePrize();
    virtual void Init();
    virtual void PostInit();
    int GetConditionValue();
    const Price &GetReceiverPrize();
    const Price &GetSenderPrize();
    static jet::String k_tmplName;
private:
    int m_conditionValue;  // 0x90
    Price *m_receiverPrize;  // 0x94
    Price *m_senderPrize;  // 0x98
};

#endif
