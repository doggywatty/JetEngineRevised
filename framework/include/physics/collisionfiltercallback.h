// framework/include/physics/collisionfiltercallback.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONFILTERCALLBACK_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONFILTERCALLBACK_H

#include "physics/collisionbody.h"
#include "physics/collisionevent.h"

// The game's say in which bodies meet: before the broad phase and the narrow phase, and for each contact the physics adds
class CollisionFilterCallback {  // sizeof 0x4
public:
    virtual ~CollisionFilterCallback() {}
    virtual bool EvaluateBroad(CollisionBody*, CollisionBody*) = 0;
    virtual bool EvaluateNarrow(CollisionBody*, CollisionBody*) = 0;
    virtual void OnCollisionEvent(CollisionEvent&) = 0;
};

#endif
