// source/game/entities/Puppet.h
#ifndef SOURCE_GAME_ENTITIES_PUPPET_H
#define SOURCE_GAME_ENTITIES_PUPPET_H

#include "Object.h"
#include "game/actors/statemachine/StateMachine.h"

class Boss;

// The puppet of a boss: an object with a state machine that is linked to the boss's deco and moves with it (the boss's state machine is
// linked to the puppet's, so the boss's engine events reach it). Made by BossGameplay::CreateBoss from the boss's linked entities.
class Puppet : public Object, public StateMachine {
public:
    Puppet(const clara::Template*);
    virtual ~Puppet();
    virtual void Update();
    virtual void PostInit();
    virtual void ConstructFrom(const GameEntity*);
    // @0x51bb84: the puppet takes the boss's deco as its link and is the boss's puppet (the boss's state machine is linked to the puppet's)
    void LinkToBoss(Boss*);
    virtual bool IsOutOfScene() const;
    static jet::String k_tmplName;
private:
    static const jet::String k_stateMachine;
};

#endif
