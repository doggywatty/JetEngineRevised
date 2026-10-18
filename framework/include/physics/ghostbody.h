// framework/include/physics/ghostbody.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_GHOSTBODY_H
#define FRAMEWORK_INCLUDE_PHYSICS_GHOSTBODY_H

#include "physics/collisionbody.h"

// A body that has no response: it only collects what overlaps it
class GhostBody : public CollisionBody {  // sizeof 0x8
public:
    virtual int GetType() { return TypeDef::Ghost; }
};

#endif
