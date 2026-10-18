// framework/include/physics/rigidbody.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_RIGIDBODY_H
#define FRAMEWORK_INCLUDE_PHYSICS_RIGIDBODY_H

#include "physics/collisionbody.h"

// A body the physics moves: static (it never does), kinematic (the game moves it) or dynamic
class RigidBody : public CollisionBody {  // sizeof 0x8
public:
    struct RoleDef { enum type { Static = 0, Kinematic = 1, Dynamic = 2 }; };
    virtual int GetType() { return TypeDef::Rigid; }  // @0x66fb80
    virtual math::vec3<float> GetPositionOffset() = 0;
    virtual void SetPositionOffset(const math::vec3<float>&) = 0;
    virtual math::vec3<float> GetCOMPosition() = 0;
    virtual math::quat<float> GetRenderRotation() = 0;
    virtual math::vec3<float> GetRenderPosition() = 0;
    virtual void SetLinearVelocity(const math::vec3<float>&) = 0;
    virtual void SetAngularVelocity(const math::vec3<float>&) = 0;
    virtual math::vec3<float> GetLinearVelocity() = 0;
    virtual math::vec3<float> GetAngularVelocity() = 0;
    virtual math::vec3<float> GetInvInertiaDiagLocal() = 0;
    virtual math::vec3<float> GetVelocityInLocalPoint(const math::vec3<float>&) = 0;
    virtual void ApplyImpulse(const math::vec3<float>&, const math::vec3<float>&) = 0;
    virtual float ComputeImpulseDenominator(const math::vec3<float>&, const math::vec3<float>&) = 0;
    virtual void ApplyCentralForce(const math::vec3<float>&) = 0;
    virtual void SetDamping(float linear, float angular) = 0;
    virtual float GetLinearDamping() = 0;
    virtual float GetAngularDamping() = 0;
    virtual void SetLinearFactor(const math::vec3<float>&) = 0;
    virtual math::vec3<float> GetLinearFactor() = 0;
    virtual void SetAngularFactor(const math::vec3<float>&) = 0;
    virtual math::vec3<float> GetAngularFactor() = 0;
    virtual float GetFriction() = 0;
    virtual void SetFriction(float) = 0;
    virtual float GetRestitution() = 0;
    virtual void SetRestitution(float) = 0;
    virtual float GetMass() = 0;
    virtual void SetMass(float) = 0;
    virtual math::vec3<float> GetGravity() = 0;
    virtual void SetGravity(const math::vec3<float>&) = 0;
    virtual int GetRole() = 0;
    virtual void SetRole(int) = 0;
    virtual void SetActive(bool) = 0;
    virtual bool IsActive() = 0;
    virtual void SetAllowDeactivation(bool) = 0;
    virtual bool GetAllowDeactivation() = 0;
};

#endif
