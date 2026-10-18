// source/game/entities/TauntDefSet.h
#ifndef SOURCE_GAME_ENTITIES_TAUNTDEFSET_H
#define SOURCE_GAME_ENTITIES_TAUNTDEFSET_H

class TauntDefSet : public GameEntity, public Singleton<TauntDefSet> {  // sizeof 0xA8
public:
    TauntDefSet(const clara::Template*);
    virtual ~TauntDefSet();
    virtual void Init();
    virtual void PostInit();
    ustl::vector<TauntDef *> &GetTauntsArray();
    TauntDef *GetDefaultTaunt();
    static jet::String k_tmplName;
private:
    ustl::vector<TauntDef *> m_taunts;  // 0x94
    TauntDef *m_defaultTaunt;  // 0xA4
};

#endif
