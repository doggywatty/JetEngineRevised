// source/game/entities/collision3dlistener.h
#ifndef SOURCE_GAME_ENTITIES_COLLISION3DLISTENER_H
#define SOURCE_GAME_ENTITIES_COLLISION3DLISTENER_H

#include "physics/collisionevent.h"

class Collision3d;
class GameEntity;

// What a collision tells when another one touches it (the object that owns it)
class Collision3dListener {  // sizeof 0x4
public:
    virtual void CollidesWith(Collision3d*, const CollisionPair&) = 0;
    virtual GameEntity *GetAsEntity() = 0;
    virtual ~Collision3dListener() {}  // @0x518624
};

#endif
