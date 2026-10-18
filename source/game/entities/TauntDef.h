// source/game/entities/TauntDef.h
#ifndef SOURCE_GAME_ENTITIES_TAUNTDEF_H
#define SOURCE_GAME_ENTITIES_TAUNTDEF_H

class TauntDef : public GameEntity {  // sizeof 0x9C
public:
    TauntDef(const clara::Template*);
    virtual ~TauntDef();
    virtual void Init();
    virtual void PostInit();
    bool IsDefaultTaunt();
    const jet::String &GetAnimixerState();
    const Price &GetPrice();
    void MarkAsBought();
    bool WasBought();
    static jet::String k_tmplName;
private:
    jet::String m_animixerState;  // 0x90
    Price *m_price;  // 0x94
    bool m_wasBought;  // 0x98
};

#endif
