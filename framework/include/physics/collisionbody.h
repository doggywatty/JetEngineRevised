// framework/include/physics/collisionbody.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONBODY_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONBODY_H

#include "smath.h"

class CollisionShape;

// Something a physics world holds: a body that is moved by the physics (rigid) or one that only senses what enters it (ghost)
class CollisionBody {  // sizeof 0x8
public:
    struct TypeDef { enum type { Rigid = 0, Ghost = 1 }; };
    virtual ~CollisionBody() {}
    virtual int GetType() = 0;
    virtual void SetTransform(const math::vec3<float>&, const math::quat<float>&) = 0;
    virtual void SetPosition(const math::vec3<float>&) = 0;
    virtual void SetRotation(const math::quat<float>&) = 0;
    virtual math::vec3<float> GetPosition() = 0;
    virtual math::quat<float> GetRotation() = 0;
    virtual void SetCollisionFilter(unsigned short group, unsigned short mask) = 0;
    virtual unsigned short GetGroupFilter() = 0;
    virtual unsigned short GetMaskFilter() = 0;
    virtual const CollisionShape *GetCollisionShape() = 0;
    void SetUserData(void *d) { m_userDataPointer = d; }
    void *GetUserData() { return m_userDataPointer; }
protected:
    CollisionBody() : m_userDataPointer(0) {}
private:
    void *m_userDataPointer;  // 0x4
};

#endif
