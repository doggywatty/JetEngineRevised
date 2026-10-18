// source/game/entities/BossActionTrigger.h
#ifndef SOURCE_GAME_ENTITIES_BOSSACTIONTRIGGER_H
#define SOURCE_GAME_ENTITIES_BOSSACTIONTRIGGER_H

// A trigger in the run: when the minion's pacesetter passes its plane close to the trigger, the level's boss gets the trigger's engine event
// in its state machine, once.
#include "GameEntity.h"

class BossActionTrigger : public GameEntity {  // sizeof 0x98
public:
    // the trigger waits for the minion, then it has fired
    enum TriggerState { TriggerState_Waiting = 0, TriggerState_Fired = 1 };
    BossActionTrigger(const clara::Template*);
    virtual ~BossActionTrigger();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    static jet::String k_tmplName;
private:
    int m_engineEvent;                    // 0x8C
    BossActionTrigger::TriggerState m_state;  // 0x90
    static const jet::String k_engineEventParam;
};

#endif
