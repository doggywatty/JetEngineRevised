// framework/include/physics/bullet/bulletphysicsworld.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETPHYSICSWORLD_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETPHYSICSWORLD_H

#include <vector>
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "physics/physicsworld.h"
#include "physics/rigidbody.h"
#include "physics/ghostbody.h"

class BulletRigidBody;

// The nearest hit of a ray, and the material of the triangle it hit
class BulletClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback {  // sizeof 0x70
public:
    BulletClosestRayResultCallback(const btVector3 &from, const btVector3 &to) : btCollisionWorld::ClosestRayResultCallback(from, to), m_materialId(0) {}
    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult&, bool normalInWorldSpace);  // @0x66db94
    unsigned m_materialId;
};

// The dispatcher asks the game whether two bodies are to meet before their shapes are tested
class BulletDispatcher : public btCollisionDispatcher {  // sizeof 0x1530
public:
    explicit BulletDispatcher(btCollisionConfiguration *c) : btCollisionDispatcher(c), m_callback(0) {}
    virtual bool needsCollision(const btCollisionObject*, const btCollisionObject*);  // @0x66d8cc
    CollisionFilterCallback *m_callback;  // 0x150C
};

// The broad phase asks the game whether two bodies may overlap
struct BulletOverlapFilterCallback : public btOverlapFilterCallback {  // sizeof 0x8
    explicit BulletOverlapFilterCallback(CollisionFilterCallback *c) : m_callback(c) {}
    virtual bool needBroadphaseCollision(btBroadphaseProxy*, btBroadphaseProxy*) const;  // @0x66d8a4
    CollisionFilterCallback *m_callback;  // 0x4
};

class BulletPhysicsWorld : public PhysicsWorld {  // sizeof 0x64
public:
    BulletPhysicsWorld();  // @0x66dc0c
    virtual ~BulletPhysicsWorld();  // @0x66de94
    virtual void AddBody(CollisionBody*);      // @0x66dff8
    virtual void RemoveBody(CollisionBody*);   // @0x66e168
    virtual math::vec3<float> GetGravity();    // @0x66e568
    virtual void SetGravity(const math::vec3<float>&);  // @0x66e5a8
    virtual void Update(unsigned ms);          // @0x66e5e8
    virtual void SetCollisionFilterCallback(CollisionFilterCallback*);  // @0x66e660
    virtual bool RayCast(const math::vec3<float>&, const math::vec3<float>&, unsigned short, unsigned short, RaycastResult&);  // @0x66e720
    btDiscreteDynamicsWorld *GetBulletObject() const { return m_world; }  // @0x66e658
protected:
    virtual void UpdateCollisionPairs();  // @0x66e93c
    static void PreStepCallbackTrampoline(btDynamicsWorld*, float);   // @0x66ed10
    static void PostStepCallbackTrampoline(btDynamicsWorld*, float);  // @0x66ed18
    void PreStepCallback(float);   // @0x66ed20
    void PostStepCallback(float) {}  // @0x66ed9c
    std::vector<BulletRigidBody*> m_rigidBodies;  // 0x38
private:
    btDiscreteDynamicsWorld *m_world;  // 0x14
    btBroadphaseInterface *m_broadphaseInterface;  // 0x1C
    btSequentialImpulseConstraintSolver *m_solver;  // 0x20
    btGhostPairCallback *m_ghostPairCallback;  // 0x24
    btDefaultCollisionConfiguration *m_collisionConfiguration;  // 0x28
    bool m_isSimulating;  // 0x48
    BulletDispatcher *m_dispatcher;  // 0x4C
    BulletOverlapFilterCallback *m_bulletOverlapFilterCallback;  // 0x50
};

// The world a level makes: it also counts what it holds
class BulletPhysicsWorldWithStatistics : public BulletPhysicsWorld {  // sizeof 0x98
public:
    struct PhysicsStatistics {
        unsigned numStaticBodies, numKinematicBodies, numDynamicBodies, numGhostBodies;  // 0x54, 0x58, 0x5C, 0x60
        PhysicsStatistics() : numStaticBodies(0), numKinematicBodies(0), numDynamicBodies(0), numGhostBodies(0) {}
    };
    BulletPhysicsWorldWithStatistics() {}  // @0x63b270
    virtual void AddBody(CollisionBody*);     // @0x63b39c
    virtual void RemoveBody(CollisionBody*);  // @0x63b4a4
    virtual void Update(unsigned ms);         // @0x63b66c
    const PhysicsStatistics &GetPhysicsStatistics() const { return m_physicStatistics; }  // @0x63b880
private:
    PhysicsStatistics m_physicStatistics;
    std::vector<RigidBody*> m_rigidBodiesForStats;
    std::vector<GhostBody*> m_ghostBodiesForStats;
};

#endif
