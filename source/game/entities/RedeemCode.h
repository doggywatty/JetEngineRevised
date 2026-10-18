// source/game/entities/RedeemCode.h
#ifndef SOURCE_GAME_ENTITIES_REDEEMCODE_H
#define SOURCE_GAME_ENTITIES_REDEEMCODE_H

class RedeemCode : public GameEntity {  // sizeof 0xB0
public:
    RedeemCode(const clara::Template*);
    virtual ~RedeemCode();
    virtual void Init();
    virtual void PostInit();
    bool IsValidRedeemCode(const ustl::vector<int>&, bool);
    void GetPrize();
    void InvalidCode();
    void AlreadyUsed();
    static jet::String k_tmplName;
private:
    ustl::vector<int> m_redeemCodeArray;  // 0x90
    bool m_isUsingNumberInput;  // 0xA0
    unsigned m_type_redeem;  // 0xA4
    clara::Entity *m_reward;  // 0xA8
    unsigned m_reward_value;  // 0xAC
};

#endif
