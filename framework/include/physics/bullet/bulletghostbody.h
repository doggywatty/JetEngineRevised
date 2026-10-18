// framework/include/physics/bullet/bulletghostbody.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETGHOSTBODY_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETGHOSTBODY_H

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "physics/ghostbody.h"
#include "physics/bullet/bulletcollisionshape.h"

class BulletPhysicsWorld;

class BulletGhostBody : public GhostBody {  // sizeof 0x18
public:
    explicit BulletGhostBody(const CollisionShape*);  // @0x6759b0
    virtual ~BulletGhostBody();                        // @0x675a74
    virtual void SetTransform(const math::vec3<float>&, const math::quat<float>&);  // @0x675b04
    virtual void SetPosition(const math::vec3<float>&);   // @0x675b80
    virtual void SetRotation(const math::quat<float>&);   // @0x675be4
    virtual math::vec3<float> GetPosition();               // @0x675c60
    virtual math::quat<float> GetRotation();               // @0x675c80
    virtual void SetCollisionFilter(unsigned short group, unsigned short mask);  // @0x675cd0
    virtual unsigned short GetGroupFilter() { return m_collisionGroup; }  // @0x675d44
    virtual unsigned short GetMaskFilter() { return m_collisionMask; }    // @0x675d4c
    virtual const CollisionShape *GetCollisionShape() { return m_collisionShape; }  // @0x675d54
    btGhostObject *GetBulletObject() const { return m_ghostObject; }  // @0x675afc
private:
    friend class BulletPhysicsWorld;
    btGhostObject *m_ghostObject;  // 0x8
    unsigned short m_collisionGroup;  // 0xC
    unsigned short m_collisionMask;  // 0xE
    const BulletCollisionShape *m_collisionShape;  // 0x10
    BulletPhysicsWorld *m_world;  // 0x14
};

#endif
