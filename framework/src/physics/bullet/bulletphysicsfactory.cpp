// framework/src/physics/bullet/bulletphysicsfactory.cpp
#include "physics/bullet/bulletphysicsfactory.h"
#include "physics/bullet/bulletphysicsworld.h"
#include "physics/bullet/bulletrigidbody.h"
#include "physics/bullet/bulletghostbody.h"
#include "physics/bullet/bulletcollisionshape.h"
#include <algorithm>

BulletPhysicsFactory::BulletPhysicsFactory() : m_world(0) {}

// @0x66c0b0: what the factory made is destroyed with it (the world last)
BulletPhysicsFactory::~BulletPhysicsFactory() {
    while (!m_rigidBodies.empty()) Destroy(m_rigidBodies.back());
    while (!m_ghostBodies.empty()) Destroy(m_ghostBodies.back());
    while (!m_shapes.empty()) Destroy(m_shapes.back());
    DestroyWorld();
}

PhysicsWorld *BulletPhysicsFactory::CreateWorld() {
    if (!m_world) m_world = new BulletPhysicsWorldWithStatistics;
    return m_world;
}

void BulletPhysicsFactory::DestroyWorld() {
    delete m_world;
    m_world = 0;
}

RigidBody *BulletPhysicsFactory::CreateRigidBody(const CollisionShape *shape) {
    RigidBody *b = new BulletRigidBody(shape);
    m_rigidBodies.push_back(b);
    return b;
}

// @0x66c20c: a body in the world leaves it first
void BulletPhysicsFactory::Destroy(RigidBody *b) {
    std::vector<RigidBody*>::iterator it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), b);
    if (it == m_rigidBodies.end()) return;
    m_rigidBodies.erase(it);
    if (static_cast<BulletRigidBody*>(b)->GetWorld()) m_world->RemoveBody(b);
    delete static_cast<BulletRigidBody*>(b);
}

GhostBody *BulletPhysicsFactory::CreateGhostBody(const CollisionShape *shape) {
    GhostBody *b = new BulletGhostBody(shape);
    m_ghostBodies.push_back(b);
    return b;
}

void BulletPhysicsFactory::Destroy(GhostBody *b) {
    std::vector<GhostBody*>::iterator it = std::find(m_ghostBodies.begin(), m_ghostBodies.end(), b);
    if (it == m_ghostBodies.end()) return;
    m_ghostBodies.erase(it);
    m_world->RemoveBody(b);
    delete static_cast<BulletGhostBody*>(b);
}

CollisionShape *BulletPhysicsFactory::CreateShape(const ShapeDef *def, unsigned materialId) {
    CollisionShape *s = new BulletCollisionShape(def, materialId);
    m_shapes.push_back(s);
    return s;
}

CollisionShape *BulletPhysicsFactory::CreateShape(const CollisionMesh *mesh, const math::vec3<float> &scale) {
    CollisionShape *s = new BulletCollisionShape(mesh, scale);
    m_shapes.push_back(s);
    return s;
}

void BulletPhysicsFactory::Destroy(CollisionShape *s) {
    std::vector<CollisionShape*>::iterator it = std::find(m_shapes.begin(), m_shapes.end(), s);
    if (it == m_shapes.end()) return;
    m_shapes.erase(it);
    delete s;
}
