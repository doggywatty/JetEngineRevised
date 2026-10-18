// framework/src/physics/bullet/bulletghostbody.cpp
#include "physics/bullet/bulletghostbody.h"
#include "physics/bullet/bulletphysicsworld.h"

// @0x6759b0: a ghost object of the shape that has no contact response
BulletGhostBody::BulletGhostBody(const CollisionShape *shape)
    : m_ghostObject(0), m_collisionGroup(0xFFFF), m_collisionMask(0xFFFF), m_collisionShape(static_cast<const BulletCollisionShape*>(shape)), m_world(0) {
    m_ghostObject = new btGhostObject();
    m_ghostObject->setCollisionFlags(m_ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    m_ghostObject->setCollisionShape(m_collisionShape->GetBulletObject());
    m_ghostObject->setUserPointer(this);
}

// @0x675a74
BulletGhostBody::~BulletGhostBody() { delete m_ghostObject; }

// @0x675b04: the rotations of the game are the conjugates of Bullet's
void BulletGhostBody::SetTransform(const math::vec3<float> &p, const math::quat<float> &r) {
    btTransform t(btMatrix3x3(btQuaternion(-r.x, -r.y, -r.z, r.w)), btVector3(p.x, p.y, p.z));
    m_ghostObject->setWorldTransform(t);
}

// @0x675b80
void BulletGhostBody::SetPosition(const math::vec3<float> &p) {
    btTransform t = m_ghostObject->getWorldTransform();
    t.setOrigin(btVector3(p.x, p.y, p.z));
    m_ghostObject->setWorldTransform(t);
}

// @0x675be4
void BulletGhostBody::SetRotation(const math::quat<float> &r) {
    btTransform t = m_ghostObject->getWorldTransform();
    t.setBasis(btMatrix3x3(btQuaternion(-r.x, -r.y, -r.z, r.w)));
    m_ghostObject->setWorldTransform(t);
}

// @0x675c60
math::vec3<float> BulletGhostBody::GetPosition() {
    const btVector3 &o = m_ghostObject->getWorldTransform().getOrigin();
    return math::vec3<float>(o.x(), o.y(), o.z());
}

// @0x675c80
math::quat<float> BulletGhostBody::GetRotation() {
    btQuaternion q;
    m_ghostObject->getWorldTransform().getBasis().getRotation(q);
    return math::quat<float>(-q.x(), -q.y(), -q.z(), q.w());
}

// @0x675cd0: in a world the body leaves it and comes back with the new filter
void BulletGhostBody::SetCollisionFilter(unsigned short group, unsigned short mask) {
    if (m_collisionGroup == group && m_collisionMask == mask) return;
    m_collisionGroup = group;
    m_collisionMask = mask;
    if (!m_world) return;
    btDiscreteDynamicsWorld *w = m_world->GetBulletObject();
    w->removeCollisionObject(m_ghostObject);
    w->addCollisionObject(m_ghostObject, (short)m_collisionGroup, (short)m_collisionMask);
}
