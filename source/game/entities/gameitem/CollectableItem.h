// source/game/entities/gameitem/CollectableItem.h
#ifndef SOURCE_GAME_ENTITIES_GAMEITEM_COLLECTABLEITEM_H
#define SOURCE_GAME_ENTITIES_GAMEITEM_COLLECTABLEITEM_H

#include "../Object.h"

class Minion;

// What the minion picks up on the way (tokens, bapples, the power ups of the game items; 0xE0 bytes in the .so): once collected it hides
// with what is linked to it. The contact with the minion (Collision3d) calls OnMinionTouch.
class CollectableItem : public Object {
public:
    struct CollectableStateDef { enum { Idle = 0, Collected = 1 }; };
    explicit CollectableItem(const clara::Template*);
    virtual ~CollectableItem();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    void OnMinionTouch(Minion *m) { Collect(m); }
    // whether the minion can pick it up while it rides a vehicle
    virtual bool CanBeCollectedInVehicle() const { return false; }
    static const jet::String k_appearProbability;
protected:
    void SetState(int s) { m_state = s; }
    virtual void Collect(Minion*) = 0;
    int m_state;  // 0xC8
};

#endif
