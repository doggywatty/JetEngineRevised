// source/game/entities/Enemy.h
#ifndef SOURCE_GAME_ENTITIES_ENEMY_H
#define SOURCE_GAME_ENTITIES_ENEMY_H

#include "Actor.h"

class Minion;

// An actor the minion meets on the way and fights (the template "Enemy"; the concrete enemies attack in ExecuteAttack). The state it enters
// tells it what the minion did: a touch (engine event 3) or a hit (engine event 6). 0x1CC bytes of Actor, then the minion it was hit by.
class Enemy : public Actor {  // sizeof 0x1D0
public:
    Enemy(const clara::Template*);
    virtual ~Enemy();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    void PushEntity(const math::vec2<float>&);
    // @0x561218: the minion touched it: the state machine takes the engine event 3
    void OnMinionTouch(Minion*);
    // @0x561238: the minion hit it: the state machine takes the engine event 6
    void OnMinionHit(Minion*);
    virtual void SM_OnChangeState(ActorState*, ActorState*);
    virtual void ExecuteAttack() = 0;
    static jet::String k_tmplName;
protected:
    Minion *m_minionHitted;  // 0x1CC
};

#endif
