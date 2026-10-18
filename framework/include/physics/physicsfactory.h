// framework/include/physics/physicsfactory.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_PHYSICSFACTORY_H
#define FRAMEWORK_INCLUDE_PHYSICS_PHYSICSFACTORY_H

#include "smath.h"
#include "physics/physicsworld.h"
#include "physics/rigidbody.h"
#include "physics/ghostbody.h"
#include "physics/collisionshape.h"

class ShapeDef;
class CollisionMesh;

// Makes and takes back the physics objects of a level
class PhysicsFactory {  // sizeof 0x4
public:
    virtual ~PhysicsFactory() {}
    virtual PhysicsWorld *CreateWorld() = 0;
    virtual void DestroyWorld() = 0;
    virtual RigidBody *CreateRigidBody(const CollisionShape*) = 0;
    virtual void Destroy(CollisionShape*) = 0;
    virtual void Destroy(GhostBody*) = 0;
    virtual void Destroy(RigidBody*) = 0;
    virtual GhostBody *CreateGhostBody(const CollisionShape*) = 0;
    virtual CollisionShape *CreateShape(const CollisionMesh*, const math::vec3<float> &scale) = 0;
    virtual CollisionShape *CreateShape(const ShapeDef*, unsigned materialId) = 0;
};

#endif
