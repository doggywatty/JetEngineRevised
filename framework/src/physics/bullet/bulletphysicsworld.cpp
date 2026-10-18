// framework/src/physics/bullet/bulletphysicsworld.cpp
#include "physics/bullet/bulletphysicsworld.h"
#include "physics/bullet/bulletrigidbody.h"
#include "physics/bullet/bulletghostbody.h"
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include <algorithm>

// the callback the game gave the world (s_collisionFilterCallback @0xd128ec)
static CollisionFilterCallback *s_collisionFilterCallback = 0;

// @0x66d8cc: the game's say, then Bullet's (the groups and masks)
bool BulletDispatcher::needsCollision(const btCollisionObject *a, const btCollisionObject *b) {
    if (m_callback && !m_callback->EvaluateNarrow(static_cast<CollisionBody*>(a->getUserPointer()), static_cast<CollisionBody*>(b->getUserPointer()))) return false;
    return btCollisionDispatcher::needsCollision(a, b);
}

// @0x66d8a4
bool BulletOverlapFilterCallback::needBroadphaseCollision(btBroadphaseProxy *a, btBroadphaseProxy *b) const {
    return m_callback->EvaluateBroad(static_cast<CollisionBody*>(static_cast<btCollisionObject*>(a->m_clientObject)->getUserPointer()),
                                     static_cast<CollisionBody*>(static_cast<btCollisionObject*>(b->m_clientObject)->getUserPointer()));
}

// @0x66db94: the material of what was hit (the triangle of a mesh) with the result
btScalar BulletClosestRayResultCallback::addSingleResult(btCollisionWorld::LocalRayResult &r, bool normalInWorldSpace) {
    if (r.m_collisionObject) {
        CollisionBody *body = static_cast<CollisionBody*>(r.m_collisionObject->getUserPointer());
        CollisionMaterialQuery q;
        if (r.m_localShapeInfo) {
            q.partId = r.m_localShapeInfo->m_shapePart;
            q.triangleIndex = r.m_localShapeInfo->m_triangleIndex;
        }
        m_materialId = body->GetCollisionShape()->GetMaterial(q);
    }
    return btCollisionWorld::ClosestRayResultCallback::addSingleResult(r, normalInWorldSpace);
}

// @0x66d91c (gContactAddedCallback): the game hears of each contact the narrow phase makes and may change how it plays out. A
// dynamic body is where the step began while it does. The edges of the triangles of a mesh are smoothed first.
static bool ContactAdded(btManifoldPoint &cp, const btCollisionObjectWrapper *wrap0, int partId0, int index0, const btCollisionObjectWrapper *wrap1, int partId1, int index1) {
    const btCollisionObject *obj0 = wrap0->getCollisionObject(), *obj1 = wrap1->getCollisionObject();
    CollisionBody *a = static_cast<CollisionBody*>(obj0->getUserPointer());
    CollisionBody *b = static_cast<CollisionBody*>(obj1->getUserPointer());
    BulletRigidBody *hackA = 0, *hackB = 0;
    if (a->GetType() != CollisionBody::TypeDef::Ghost) {
        BulletRigidBody *r = static_cast<BulletRigidBody*>(a);
        if (r->GetRole() == RigidBody::RoleDef::Dynamic) {
            hackA = r;
            r->UseCollisionHackTransform(true);
        }
    }
    if (b->GetType() != CollisionBody::TypeDef::Ghost) {
        BulletRigidBody *r = static_cast<BulletRigidBody*>(b);
        if (r->GetRole() == RigidBody::RoleDef::Dynamic) {
            hackB = r;
            r->UseCollisionHackTransform(true);
        }
    }
    btAdjustInternalEdgeContacts(cp, wrap1, wrap0, partId1, index1, 0);
    CollisionEvent ev;
    ev.m_bodyA = a;
    ev.m_bodyB = b;
    ev.m_pointA = math::vec3<float>(cp.m_positionWorldOnA.x(), cp.m_positionWorldOnA.y(), cp.m_positionWorldOnA.z());
    ev.m_pointB = math::vec3<float>(cp.m_positionWorldOnB.x(), cp.m_positionWorldOnB.y(), cp.m_positionWorldOnB.z());
    ev.m_normalB = math::vec3<float>(cp.m_normalWorldOnB.x(), cp.m_normalWorldOnB.y(), cp.m_normalWorldOnB.z());
    ev.m_restitution = cp.m_combinedRestitution;
    ev.m_friction = cp.m_combinedFriction;
    ev.m_materialIdA = a->GetCollisionShape()->GetMaterial(CollisionMaterialQuery(cp.m_partId0, cp.m_index0));
    ev.m_materialIdB = b->GetCollisionShape()->GetMaterial(CollisionMaterialQuery(cp.m_partId1, cp.m_index1));
    s_collisionFilterCallback->OnCollisionEvent(ev);
    cp.m_combinedFriction = ev.m_friction;
    cp.m_combinedRestitution = ev.m_restitution;
    if (hackA) hackA->UseCollisionHackTransform(false);
    if (hackB) hackB->UseCollisionHackTransform(false);
    return true;
}

// @0x66dc0c: Bullet's default collision configuration, the game's dispatcher, a dynamic bounding volume tree for the broad phase, the
// sequential impulse solver; a ghost pair callback so ghost bodies see what overlaps them; 2 simulation steps
BulletPhysicsWorld::BulletPhysicsWorld() : m_world(0), m_isSimulating(false), m_bulletOverlapFilterCallback(0) {
    m_simulationSteps = 2;
    btDefaultCollisionConstructionInfo info;
    info.m_defaultMaxPersistentManifoldPoolSize = 0x40;
    info.m_defaultMaxCollisionAlgorithmPoolSize = 0x40;
    m_collisionConfiguration = new btDefaultCollisionConfiguration(info);
    m_collisionConfiguration->setConvexConvexMultipointIterations(3, 3);
    m_dispatcher = new BulletDispatcher(m_collisionConfiguration);
    m_broadphaseInterface = new btDbvtBroadphase(0);
    m_solver = new btSequentialImpulseConstraintSolver();
    m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphaseInterface, m_solver, m_collisionConfiguration);
    m_world->setInternalTickCallback(PreStepCallbackTrampoline, this, true);
    m_world->setInternalTickCallback(PostStepCallbackTrampoline, this, false);
    m_world->setGravity(btVector3(0, 0, 0));
    m_world->setForceUpdateAllAabbs(false);
    m_ghostPairCallback = new btGhostPairCallback();
    m_world->getPairCache()->setInternalGhostPairCallback(m_ghostPairCallback);
}

// @0x66de94
BulletPhysicsWorld::~BulletPhysicsWorld() {
    if (s_collisionFilterCallback) s_collisionFilterCallback = 0;
    delete m_world;
    delete m_ghostPairCallback;
    delete m_solver;
    delete m_broadphaseInterface;
    delete m_dispatcher;
    delete m_collisionConfiguration;
    delete m_bulletOverlapFilterCallback;
}

// @0x66dff8: a rigid body enters the world with its transform, group and mask and is listed; a ghost goes in as a collision object
void BulletPhysicsWorld::AddBody(CollisionBody *body) {
    if (body->GetType() == CollisionBody::TypeDef::Rigid) {
        BulletRigidBody *r = static_cast<BulletRigidBody*>(body);
        btRigidBody *b = r->GetBulletObject();
        b->setInterpolationWorldTransform(b->getWorldTransform());
        m_world->addRigidBody(b, (short)r->GetGroupFilter(), (short)r->GetMaskFilter());
        r->m_world = this;
        m_rigidBodies.push_back(r);
    } else if (body->GetType() == CollisionBody::TypeDef::Ghost) {
        BulletGhostBody *g = static_cast<BulletGhostBody*>(body);
        m_world->addCollisionObject(g->GetBulletObject(), (short)g->GetGroupFilter(), (short)g->GetMaskFilter());
        g->m_world = this;
    }
}

// @0x66e168: the body leaves the world and what Bullet holds of it, and the pairs it was in
void BulletPhysicsWorld::RemoveBody(CollisionBody *body) {
    if (body->GetType() == CollisionBody::TypeDef::Rigid) {
        BulletRigidBody *r = static_cast<BulletRigidBody*>(body);
        btRigidBody *b = r->GetBulletObject();
        m_world->removeRigidBody(b);
        r->m_world = 0;
        m_world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(b->getBroadphaseHandle(), m_dispatcher);
        std::vector<BulletRigidBody*>::iterator it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), r);
        if (it != m_rigidBodies.end()) m_rigidBodies.erase(it);
    } else if (body->GetType() == CollisionBody::TypeDef::Ghost) {
        BulletGhostBody *g = static_cast<BulletGhostBody*>(body);
        m_world->removeCollisionObject(g->GetBulletObject());
        g->m_world = 0;
        m_world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(g->GetBulletObject()->getBroadphaseHandle(), m_dispatcher);
    }
    for (size_t i = 0; i < m_collisionPairs.size();) {
        if (m_collisionPairs[i].bodyA == body || m_collisionPairs[i].bodyB == body) m_collisionPairs.erase(m_collisionPairs.begin() + i);
        else i++;
    }
}

// @0x66e568, 0x66e5a8
math::vec3<float> BulletPhysicsWorld::GetGravity() {
    btVector3 g = m_world->getGravity();
    return math::vec3<float>(g.x(), g.y(), g.z());
}
void BulletPhysicsWorld::SetGravity(const math::vec3<float> &g) { m_world->setGravity(btVector3(g.x, g.y, g.z)); }

// @0x66e5e8: the milliseconds in seconds, in at most m_simulationSteps steps of 1/60 s shared among them
void BulletPhysicsWorld::Update(unsigned ms) {
    m_isSimulating = true;
    m_world->stepSimulation((float)ms * 0.001f, (int)m_simulationSteps, (1.0f / 60.0f) / (float)m_simulationSteps);
    UpdateCollisionPairs();
    m_isSimulating = false;
}

// @0x66e660: the game decides who meets whom, in the broad phase, in the dispatcher and with every contact made
void BulletPhysicsWorld::SetCollisionFilterCallback(CollisionFilterCallback *c) {
    BulletOverlapFilterCallback *old = m_bulletOverlapFilterCallback;
    m_bulletOverlapFilterCallback = c ? new BulletOverlapFilterCallback(c) : 0;
    m_world->getPairCache()->setOverlapFilterCallback(m_bulletOverlapFilterCallback);
    m_dispatcher->m_callback = c;
    s_collisionFilterCallback = c;
    gContactAddedCallback = c ? ContactAdded : 0;
    delete old;
}

// @0x66e720: the closest body the ray meets among the groups and masks asked
bool BulletPhysicsWorld::RayCast(const math::vec3<float> &from, const math::vec3<float> &to, unsigned short group, unsigned short mask, RaycastResult &out) {
    const btVector3 f(from.x, from.y, from.z), t(to.x, to.y, to.z);
    BulletClosestRayResultCallback cb(f, t);
    cb.m_collisionFilterGroup = (short)group;
    cb.m_collisionFilterMask = (short)mask;
    m_world->rayTest(f, t, cb);
    if (!cb.m_collisionObject) return false;
    out.bodyId = cb.m_collisionObject->getUserPointer();
    out.position = math::vec3<float>(cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(), cb.m_hitPointWorld.z());
    out.normal = math::vec3<float>(cb.m_hitNormalWorld.x(), cb.m_hitNormalWorld.y(), cb.m_hitNormalWorld.z());
    out.materialId = cb.m_materialId;
    out.distance = (from - to).getLength() * cb.m_closestHitFraction;
    return true;
}

// @0x66e93c: one pair for each manifold that has contacts; a ghost's has none to give
void BulletPhysicsWorld::UpdateCollisionPairs() {
    const int manifolds = m_dispatcher->getNumManifolds();
    m_collisionPairs.clear();
    m_collisionPairs.reserve((size_t)manifolds);
    for (int m = 0; m < manifolds; m++) {
        btPersistentManifold *pm = m_dispatcher->getManifoldByIndexInternal(m);
        int count = pm->getNumContacts();
        if (!count) continue;
        m_collisionPairs.push_back(CollisionPair());
        CollisionPair &p = m_collisionPairs.back();
        p.bodyA = static_cast<CollisionBody*>(pm->getBody0()->getUserPointer());
        p.bodyB = static_cast<CollisionBody*>(pm->getBody1()->getUserPointer());
        if (p.bodyA->GetType() == CollisionBody::TypeDef::Ghost || p.bodyB->GetType() == CollisionBody::TypeDef::Ghost) count = 0;
        if (count > 4) count = 4;
        p.numContacts = count;
        for (int c = 0; c < count; c++) {
            const btManifoldPoint &cp = pm->getContactPoint(c);
            CollisionPair::Contact &k = p.contacts[c];
            k.pointA = math::vec3<float>(cp.m_positionWorldOnA.x(), cp.m_positionWorldOnA.y(), cp.m_positionWorldOnA.z());
            k.pointB = math::vec3<float>(cp.m_positionWorldOnB.x(), cp.m_positionWorldOnB.y(), cp.m_positionWorldOnB.z());
            k.normal = math::vec3<float>(cp.m_normalWorldOnB.x(), cp.m_normalWorldOnB.y(), cp.m_normalWorldOnB.z());
            k.distance = cp.m_distance1;
            k.materialIdA = p.bodyA->GetCollisionShape()->GetMaterial(CollisionMaterialQuery(cp.m_partId0, cp.m_index0));
            k.materialIdB = p.bodyB->GetCollisionShape()->GetMaterial(CollisionMaterialQuery(cp.m_partId1, cp.m_index1));
        }
    }
}

// @0x66ed10
void BulletPhysicsWorld::PreStepCallbackTrampoline(btDynamicsWorld *w, float dt) { static_cast<BulletPhysicsWorld*>(w->getWorldUserInfo())->PreStepCallback(dt); }
void BulletPhysicsWorld::PostStepCallbackTrampoline(btDynamicsWorld *w, float dt) { static_cast<BulletPhysicsWorld*>(w->getWorldUserInfo())->PostStepCallback(dt); }

// @0x66ed20: where the dynamic bodies stand as a step begins is what a contact of theirs reports
void BulletPhysicsWorld::PreStepCallback(float) {
    for (BulletRigidBody *r : m_rigidBodies)
        if (r->GetRole() == RigidBody::RoleDef::Dynamic) r->SetCollisionHackTransform(r->GetPosition(), r->GetRotation());
}

// @0x63b39c: bodies are counted by what they are
void BulletPhysicsWorldWithStatistics::AddBody(CollisionBody *body) {
    BulletPhysicsWorld::AddBody(body);
    if (body->GetType() == CollisionBody::TypeDef::Rigid) m_rigidBodiesForStats.push_back(static_cast<RigidBody*>(body));
    else if (body->GetType() == CollisionBody::TypeDef::Ghost) m_ghostBodiesForStats.push_back(static_cast<GhostBody*>(body));
}

// @0x63b4a4
void BulletPhysicsWorldWithStatistics::RemoveBody(CollisionBody *body) {
    BulletPhysicsWorld::RemoveBody(body);
    if (body->GetType() == CollisionBody::TypeDef::Rigid) {
        std::vector<RigidBody*>::iterator it = std::find(m_rigidBodiesForStats.begin(), m_rigidBodiesForStats.end(), static_cast<RigidBody*>(body));
        if (it != m_rigidBodiesForStats.end()) m_rigidBodiesForStats.erase(it);
    } else if (body->GetType() == CollisionBody::TypeDef::Ghost) {
        std::vector<GhostBody*>::iterator it = std::find(m_ghostBodiesForStats.begin(), m_ghostBodiesForStats.end(), static_cast<GhostBody*>(body));
        if (it != m_ghostBodiesForStats.end()) m_ghostBodiesForStats.erase(it);
    }
}

// @0x63b66c: the simulation, then the count of the bodies by role
void BulletPhysicsWorldWithStatistics::Update(unsigned ms) {
    BulletPhysicsWorld::Update(ms);
    unsigned stat = 0, kin = 0, dyn = 0;
    for (RigidBody *r : m_rigidBodiesForStats) {
        if (r->GetRole() == RigidBody::RoleDef::Kinematic) kin++;
        else if (r->GetRole() == RigidBody::RoleDef::Dynamic) dyn++;
        else stat++;
    }
    m_physicStatistics.numDynamicBodies = dyn;
    m_physicStatistics.numKinematicBodies = kin;
    m_physicStatistics.numStaticBodies = stat;
    m_physicStatistics.numGhostBodies = (unsigned)m_ghostBodiesForStats.size();
}
