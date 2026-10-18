// framework/include/physics/bullet/bulletrigidbody.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETRIGIDBODY_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETRIGIDBODY_H

#include "btBulletDynamicsCommon.h"
#include "physics/rigidbody.h"
#include "physics/bullet/bulletcollisionshape.h"

class BulletPhysicsWorld;

// Holds the transform Bullet moves a body with (the body's world transform is the game's)
class MyMotionState : public btMotionState {  // sizeof 0x50
public:
    explicit MyMotionState(const btTransform &t) : m_worldTrans(t) {}
    virtual void getWorldTransform(btTransform &t) const { t = m_worldTrans; }
    virtual void setWorldTransform(const btTransform &t) { m_worldTrans = t; }
    btTransform m_worldTrans;  // 0x10 (0x4 in the .so)
};

class BulletRigidBody : public RigidBody {  // sizeof 0x50
public:
    explicit BulletRigidBody(const CollisionShape*);  // @0x670030
    virtual ~BulletRigidBody();                        // @0x6702f0
    virtual void SetTransform(const math::vec3<float>&, const math::quat<float>&);  // @0x6703a8
    virtual void SetPosition(const math::vec3<float>&);   // @0x6704b4
    virtual void SetRotation(const math::quat<float>&);   // @0x670568
    virtual math::vec3<float> GetPosition();               // @0x670678
    virtual math::quat<float> GetRotation();               // @0x670790
    virtual math::quat<float> GetRenderRotation();         // @0x67072c
    virtual math::vec3<float> GetRenderPosition();         // @0x6705e8
    virtual math::vec3<float> GetPositionOffset() { return m_centerOfMassOffset; }  // @0x67081c
    virtual void SetPositionOffset(const math::vec3<float> &o) { m_centerOfMassOffset = o; }  // @0x670838
    virtual math::vec3<float> GetCOMPosition();            // @0x670854
    virtual void SetLinearVelocity(const math::vec3<float>&);   // @0x670894
    virtual void SetAngularVelocity(const math::vec3<float>&);  // @0x6708d0
    virtual math::vec3<float> GetLinearVelocity();         // @0x67090c
    virtual math::vec3<float> GetAngularVelocity();        // @0x67092c
    virtual math::vec3<float> GetInvInertiaDiagLocal();    // @0x670bf4
    virtual math::vec3<float> GetVelocityInLocalPoint(const math::vec3<float>&);  // @0x670c14
    virtual void ApplyImpulse(const math::vec3<float>&, const math::vec3<float>&);  // @0x670c90
    virtual float ComputeImpulseDenominator(const math::vec3<float>&, const math::vec3<float>&);  // @0x670db0
    virtual void ApplyCentralForce(const math::vec3<float>&);  // @0x670e0c
    virtual void SetDamping(float linear, float angular) { m_rigidBody->setDamping(linear, angular); }  // @0x670e60
    virtual float GetLinearDamping() { return m_rigidBody->getLinearDamping(); }  // @0x670e68
    virtual float GetAngularDamping() { return m_rigidBody->getAngularDamping(); }  // @0x670e74
    virtual void SetLinearFactor(const math::vec3<float>&);   // @0x670e80
    virtual math::vec3<float> GetLinearFactor();             // @0x670efc
    virtual void SetAngularFactor(const math::vec3<float>&);  // @0x670f1c
    virtual math::vec3<float> GetAngularFactor();            // @0x670f58
    virtual float GetFriction() { return m_rigidBody->getFriction(); }  // @0x6709d4
    virtual void SetFriction(float f) { m_rigidBody->setFriction(f); }  // @0x6709e0
    virtual float GetRestitution() { return m_rigidBody->getRestitution(); }  // @0x6709ec
    virtual void SetRestitution(float r) { m_rigidBody->setRestitution(r); }  // @0x6709f8
    virtual float GetMass();                                  // @0x670a04
    virtual void SetMass(float);                              // @0x670a2c
    virtual math::vec3<float> GetGravity();                   // @0x670a78
    virtual void SetGravity(const math::vec3<float>&);        // @0x670a98
    virtual int GetRole() { return m_role; }                  // @0x67094c
    virtual void SetRole(int);                                // @0x670954
    virtual void SetActive(bool);                             // @0x670b6c
    virtual bool IsActive();                                  // @0x670b8c
    virtual void SetAllowDeactivation(bool);                  // @0x670ba0
    virtual bool GetAllowDeactivation() { return m_allowsDeactivation; }  // @0x670bec
    virtual void SetCollisionFilter(unsigned short group, unsigned short mask);  // @0x670ae0
    virtual unsigned short GetGroupFilter() { return m_collisionGroup; }  // @0x670b54
    virtual unsigned short GetMaskFilter() { return m_collisionMask; }    // @0x670b5c
    virtual const CollisionShape *GetCollisionShape() { return m_collisionShape; }  // @0x670b64
    btRigidBody *GetBulletObject() const { return m_rigidBody; }  // @0x670398
    BulletPhysicsWorld *GetWorld() const { return m_world; }      // @0x6703a0
    // while a contact is reported the position of a dynamic body is the one the step began with
    void UseCollisionHackTransform(bool b) { m_useCollisionHackTransform = b; }  // @0x670f78
    void SetCollisionHackTransform(const math::vec3<float> &p, const math::quat<float> &r) { m_collisionHackPosition = p; m_collisionHackRotation = r; }  // @0x670f80
private:
    friend class BulletPhysicsWorld;
    bool m_useCollisionHackTransform;  // 0x8
    math::vec3<float> m_collisionHackPosition;  // 0xC
    math::quat<float> m_collisionHackRotation;  // 0x18
    int m_role;  // 0x28
    MyMotionState *m_motionState;  // 0x2C
    btRigidBody *m_rigidBody;  // 0x30
    unsigned short m_collisionGroup;  // 0x34
    unsigned short m_collisionMask;  // 0x36
    const BulletCollisionShape *m_collisionShape;  // 0x38
    BulletPhysicsWorld *m_world;  // 0x3C
    math::vec3<float> m_centerOfMassOffset;  // 0x40
    bool m_allowsDeactivation;  // 0x4C
};

#endif
