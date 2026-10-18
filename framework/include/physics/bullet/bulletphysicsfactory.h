// framework/include/physics/bullet/bulletphysicsfactory.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETPHYSICSFACTORY_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETPHYSICSFACTORY_H

#include <vector>
#include "physics/physicsfactory.h"

// The Bullet physics objects of a level; what it makes it takes back with itself
class BulletPhysicsFactory : public PhysicsFactory {  // sizeof 0x58
public:
    BulletPhysicsFactory();  // @0x66c054
    virtual ~BulletPhysicsFactory();  // @0x66c0b0
    virtual PhysicsWorld *CreateWorld();  // @0x66c130: the one world (with the statistics)
    virtual void DestroyWorld();  // @0x66c170
    virtual RigidBody *CreateRigidBody(const CollisionShape*);  // @0x66c19c
    virtual void Destroy(RigidBody*);  // @0x66c20c
    virtual GhostBody *CreateGhostBody(const CollisionShape*);  // @0x66c310
    virtual void Destroy(GhostBody*);  // @0x66c380
    virtual CollisionShape *CreateShape(const ShapeDef*, unsigned materialId);  // @0x66c484
    virtual CollisionShape *CreateShape(const CollisionMesh*, const math::vec3<float> &scale);  // @0x66c4d4
    virtual void Destroy(CollisionShape*);  // @0x66c524
private:
    PhysicsWorld *m_world;  // 0x4
    std::vector<RigidBody*> m_rigidBodies;  // 0x8
    std::vector<GhostBody*> m_ghostBodies;  // 0x1C
    std::vector<CollisionShape*> m_shapes;  // 0x30
};

#endif
