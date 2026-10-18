// framework/src/physics/bullet/bulletrigidbody.cpp
#include "physics/bullet/bulletrigidbody.h"
#include "physics/bullet/bulletphysicsworld.h"

namespace {
// the game's rotations are the conjugates of Bullet's (the vector part negated, @0x6703f0)
inline btQuaternion ToBullet(const math::quat<float> &q) { return btQuaternion(-q.x, -q.y, -q.z, q.w); }
inline btVector3 ToBullet(const math::vec3<float> &v) { return btVector3(v.x, v.y, v.z); }
inline math::vec3<float> FromBullet(const btVector3 &v) { return math::vec3<float>(v.x(), v.y(), v.z()); }
}

// @0x670030: a static body (no mass) of the shape, 0.5 friction, kept in a motion state of its own; the group and mask let
// everything meet it until the game says otherwise
BulletRigidBody::BulletRigidBody(const CollisionShape *shape)
    : m_useCollisionHackTransform(false), m_collisionHackPosition(0, 0, 0), m_collisionHackRotation(0, 0, 0, 1), m_role(0), m_motionState(0), m_rigidBody(0),
      m_collisionGroup(0xFFFF), m_collisionMask(0xFFFF), m_collisionShape(static_cast<const BulletCollisionShape*>(shape)), m_world(0), m_centerOfMassOffset(0, 0, 0),
      m_allowsDeactivation(true) {
    m_motionState = new MyMotionState(btTransform::getIdentity());
    btRigidBody::btRigidBodyConstructionInfo info(0, m_motionState, m_collisionShape->GetBulletObject());
    m_rigidBody = new btRigidBody(info);
    m_rigidBody->setSleepingThresholds(0.01f, 0.01f);
    m_rigidBody->setUserPointer(this);
}

// @0x6702f0
BulletRigidBody::~BulletRigidBody() {
    delete m_rigidBody;
    delete m_motionState;
}

// @0x6703a8: the body's position is where its centre of mass is less the offset, turned by it
void BulletRigidBody::SetTransform(const math::vec3<float> &p, const math::quat<float> &r) {
    btTransform t(btMatrix3x3(ToBullet(r)), btVector3(0, 0, 0));
    const btVector3 origin = ToBullet(p) + t.getBasis() * ToBullet(m_centerOfMassOffset);
    t.setOrigin(origin);
    m_rigidBody->setWorldTransform(t);
    m_motionState->setWorldTransform(t);
}

// @0x6704b4
void BulletRigidBody::SetPosition(const math::vec3<float> &p) {
    btTransform t = m_rigidBody->getWorldTransform();
    t.setOrigin(ToBullet(p) + t.getBasis() * ToBullet(m_centerOfMassOffset));
    m_rigidBody->setWorldTransform(t);
    m_motionState->setWorldTransform(t);
}

// @0x670568
void BulletRigidBody::SetRotation(const math::quat<float> &r) {
    btTransform t = m_rigidBody->getWorldTransform();
    t.setBasis(btMatrix3x3(ToBullet(r)));
    m_rigidBody->setWorldTransform(t);
    m_motionState->setWorldTransform(t);
}

// @0x6705e8: where the motion state has it (the one the render shows)
math::vec3<float> BulletRigidBody::GetRenderPosition() {
    btTransform t;
    m_motionState->getWorldTransform(t);
    return FromBullet(t.getOrigin() - t.getBasis() * ToBullet(m_centerOfMassOffset));
}

// @0x670678: the position of a dynamic body during a contact is the hack's
math::vec3<float> BulletRigidBody::GetPosition() {
    if (m_useCollisionHackTransform) return m_collisionHackPosition;
    const btTransform &t = m_rigidBody->getWorldTransform();
    return FromBullet(t.getOrigin() - t.getBasis() * ToBullet(m_centerOfMassOffset));
}

// @0x67072c
math::quat<float> BulletRigidBody::GetRenderRotation() {
    btTransform t;
    m_motionState->getWorldTransform(t);
    btQuaternion q;
    t.getBasis().getRotation(q);
    return math::quat<float>(-q.x(), -q.y(), -q.z(), q.w());
}

// @0x670790
math::quat<float> BulletRigidBody::GetRotation() {
    btQuaternion q;
    m_rigidBody->getWorldTransform().getBasis().getRotation(q);
    return math::quat<float>(-q.x(), -q.y(), -q.z(), q.w());
}

// @0x670854
math::vec3<float> BulletRigidBody::GetCOMPosition() { return FromBullet(m_rigidBody->getCenterOfMassPosition()); }

// @0x670894, 0x6708d0, 0x67090c, 0x67092c
void BulletRigidBody::SetLinearVelocity(const math::vec3<float> &v) { m_rigidBody->setLinearVelocity(ToBullet(v)); }
void BulletRigidBody::SetAngularVelocity(const math::vec3<float> &v) { m_rigidBody->setAngularVelocity(ToBullet(v)); }
math::vec3<float> BulletRigidBody::GetLinearVelocity() { return FromBullet(m_rigidBody->getLinearVelocity()); }
math::vec3<float> BulletRigidBody::GetAngularVelocity() { return FromBullet(m_rigidBody->getAngularVelocity()); }
math::vec3<float> BulletRigidBody::GetInvInertiaDiagLocal() { return FromBullet(m_rigidBody->getInvInertiaDiagLocal()); }
math::vec3<float> BulletRigidBody::GetVelocityInLocalPoint(const math::vec3<float> &p) { return FromBullet(m_rigidBody->getVelocityInLocalPoint(ToBullet(p))); }
void BulletRigidBody::ApplyImpulse(const math::vec3<float> &impulse, const math::vec3<float> &relPos) { m_rigidBody->applyImpulse(ToBullet(impulse), ToBullet(relPos)); }
float BulletRigidBody::ComputeImpulseDenominator(const math::vec3<float> &pos, const math::vec3<float> &normal) { return m_rigidBody->computeImpulseDenominator(ToBullet(pos), ToBullet(normal)); }
void BulletRigidBody::ApplyCentralForce(const math::vec3<float> &f) { m_rigidBody->applyCentralForce(ToBullet(f)); }
void BulletRigidBody::SetLinearFactor(const math::vec3<float> &f) { m_rigidBody->setLinearFactor(ToBullet(f)); }
math::vec3<float> BulletRigidBody::GetLinearFactor() { return FromBullet(m_rigidBody->getLinearFactor()); }
void BulletRigidBody::SetAngularFactor(const math::vec3<float> &f) { m_rigidBody->setAngularFactor(ToBullet(f)); }
math::vec3<float> BulletRigidBody::GetAngularFactor() { return FromBullet(m_rigidBody->getAngularFactor()); }

// @0x670a04: Bullet holds the inverse
float BulletRigidBody::GetMass() { return m_rigidBody->getInvMass() != 0 ? 1.0f / m_rigidBody->getInvMass() : 0.0f; }

// @0x670a2c
void BulletRigidBody::SetMass(float mass) {
    btVector3 inertia;
    m_collisionShape->GetBulletObject()->calculateLocalInertia(mass, inertia);
    m_rigidBody->setMassProps(mass, inertia);
    m_rigidBody->updateInertiaTensor();
}

// @0x670a78
math::vec3<float> BulletRigidBody::GetGravity() { return FromBullet(m_rigidBody->getGravity()); }

// @0x670a98: the body's own gravity, not the world's
void BulletRigidBody::SetGravity(const math::vec3<float> &g) {
    m_rigidBody->setGravity(ToBullet(g));
    m_rigidBody->setFlags(m_rigidBody->getFlags() | BT_DISABLE_WORLD_GRAVITY);
}

// @0x670954: a dynamic body is simulated (and sleeps), a kinematic one never sleeps, a static one is not simulated
void BulletRigidBody::SetRole(int role) {
    m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    int flags = m_rigidBody->getCollisionFlags();
    if (role == RoleDef::Dynamic) {
        m_rigidBody->setCollisionFlags(flags & ~(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_KINEMATIC_OBJECT));
        m_rigidBody->setActivationState(WANTS_DEACTIVATION);
        m_rigidBody->activate(false);
    } else if (role == RoleDef::Kinematic) {
        m_rigidBody->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
        m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
    } else {
        m_rigidBody->setCollisionFlags(flags | btCollisionObject::CF_STATIC_OBJECT);
        m_rigidBody->setActivationState(DISABLE_SIMULATION);
    }
    m_role = role;
}

// @0x670b6c: only a dynamic body can be put to sleep
void BulletRigidBody::SetActive(bool active) {
    if (m_role != RoleDef::Dynamic) return;
    m_rigidBody->setActivationState(active ? ACTIVE_TAG : ISLAND_SLEEPING);
}

// @0x670b8c
bool BulletRigidBody::IsActive() { return m_rigidBody->getActivationState() != ISLAND_SLEEPING; }

// @0x670ba0
void BulletRigidBody::SetAllowDeactivation(bool allow) {
    if (!allow)
        m_rigidBody->forceActivationState(DISABLE_DEACTIVATION);
    else
        m_rigidBody->setActivationState(m_role == RoleDef::Dynamic ? ACTIVE_TAG : m_role == RoleDef::Kinematic ? DISABLE_DEACTIVATION : DISABLE_SIMULATION);
    m_allowsDeactivation = allow;
}

// @0x670ae0: in a world the body leaves it and comes back with the new filter
void BulletRigidBody::SetCollisionFilter(unsigned short group, unsigned short mask) {
    if (m_collisionGroup == group && m_collisionMask == mask) return;
    m_collisionGroup = group;
    m_collisionMask = mask;
    if (!m_world) return;
    btDiscreteDynamicsWorld *w = m_world->GetBulletObject();
    w->removeRigidBody(m_rigidBody);
    w->addRigidBody(m_rigidBody, (short)m_collisionGroup, (short)m_collisionMask);
}
