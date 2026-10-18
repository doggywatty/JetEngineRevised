// source/game/entities/BossKiller.h
#ifndef SOURCE_GAME_ENTITIES_BOSSKILLER_H
#define SOURCE_GAME_ENTITIES_BOSSKILLER_H

#include "Object.h"

// The end of the boss gameplay: when the minion's pacesetter is at the killer's plane while the boss is active (or defeated), the boss gameplay
// ends with the states the data names for the two cases (the exit state, or the defeated exit state).
class BossKiller : public Object {  // sizeof 0xD0
public:
    BossKiller(const clara::Template*);
    virtual ~BossKiller();
    virtual void Update();
    virtual void PostInit();
    static jet::String k_tmplName;
private:
    jet::String m_exitStateName;          // 0xC8
    jet::String m_defeatedExitStateName;  // 0xCC
    static const jet::String k_exitStateParamName;
    static const jet::String k_defeatedExitStateParamName;
};

#endif
