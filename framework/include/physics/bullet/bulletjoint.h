// framework/include/physics/bullet/bulletjoint.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETJOINT_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETJOINT_H

class BulletHingeJoint : public HingeJoint {  // sizeof 0x18
protected:
    BulletHingeJoint(RigidBody*, RigidBody*, const math::mat4<float>&, const math::mat4<float>&);
    virtual ~BulletHingeJoint();
public:
    virtual void SetLimits(float, float);
    btHingeConstraint *GetBulletObject();
protected:
    float m_limits[2];  // 0xC
    btHingeConstraint *m_contraint;  // 0x14
};

#endif
