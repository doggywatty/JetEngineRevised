// framework/include/physics/joint.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_JOINT_H
#define FRAMEWORK_INCLUDE_PHYSICS_JOINT_H

class Joint {  // sizeof 0xC
protected:
    Joint();
    virtual ~Joint();
public:
    virtual safe_enum<Joint::TypeDef,enum Joint::TypeDef::type> GetType() = 0;
    RigidBody *GetBodyA();
    RigidBody *GetBodyB();
protected:
    RigidBody *m_bodyA;  // 0x4
    RigidBody *m_bodyB;  // 0x8
};

#endif
