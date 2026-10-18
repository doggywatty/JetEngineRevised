// framework/include/physics/physicsworld.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_PHYSICSWORLD_H
#define FRAMEWORK_INCLUDE_PHYSICS_PHYSICSWORLD_H

#include <vector>
#include "smath.h"
#include "physics/collisionbody.h"
#include "physics/collisionevent.h"
#include "physics/collisionfiltercallback.h"

// What a ray cast found (sizeof 0x24; BulletPhysicsWorld::RayCast @0x66e720 fills it): the user data of the
// body hit, where and the normal it was hit with, the material and how far from the start of the ray
struct RaycastResult {
    void *bodyId;                      // 0x0
    math::vec3<float> position;        // 0x4
    math::vec3<float> normal;          // 0x10
    unsigned materialId;               // 0x1C
    float distance;                    // 0x20
    RaycastResult() : bodyId(0), position(0, 0, 0), normal(0, 0, 0), materialId(0), distance(0) {}
};

class PhysicsWorld {  // sizeof 0x1C
public:
    virtual ~PhysicsWorld() {}
    virtual void AddBody(CollisionBody*) = 0;
    virtual void RemoveBody(CollisionBody*) = 0;
    virtual math::vec3<float> GetGravity() = 0;
    virtual void SetGravity(const math::vec3<float>&) = 0;
    // steps the simulation by the milliseconds and finds the pairs in contact
    virtual void Update(unsigned ms) = 0;
    unsigned GetNumberOfSimulationSteps() { return m_simulationSteps; }
    void SetNumberOfSimulationSteps(unsigned s) { m_simulationSteps = s; }
    virtual void SetCollisionFilterCallback(CollisionFilterCallback*) = 0;
    const std::vector<CollisionPair> &GetCollisionPairs() { return m_collisionPairs; }
    // the nearest body a ray from the first point to the second meets, among the bodies that the group and the mask let through
    virtual bool RayCast(const math::vec3<float> &from, const math::vec3<float> &to, unsigned short group, unsigned short mask, RaycastResult&) = 0;
protected:
    PhysicsWorld() : m_simulationSteps(0) {}
    std::vector<CollisionPair> m_collisionPairs;  // 0x4
    unsigned m_simulationSteps;  // 0x10
};

#endif
