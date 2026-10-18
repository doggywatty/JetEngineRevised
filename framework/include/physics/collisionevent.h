// framework/include/physics/collisionevent.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONEVENT_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONEVENT_H

#include "smath.h"
#include "physics/collisionbody.h"

// A contact as the physics resolves it; the filter callback may change the restitution and the friction
struct CollisionEvent {  // sizeof 0x3C
    CollisionEvent() : m_bodyA(0), m_bodyB(0), m_pointA(0, 0, 0), m_pointB(0, 0, 0), m_normalB(0, 0, 0), m_materialIdA(0), m_materialIdB(0), m_restitution(0), m_friction(0) {}
    const CollisionBody *m_bodyA;  // 0x0
    const CollisionBody *m_bodyB;  // 0x4
    math::vec3<float> m_pointA;  // 0x8
    math::vec3<float> m_pointB;  // 0x14
    math::vec3<float> m_normalB;  // 0x20
    unsigned m_materialIdA;  // 0x2C
    unsigned m_materialIdB;  // 0x30
    float m_restitution;  // 0x34
    float m_friction;  // 0x38
};

// The bodies that touch and where, at most 4 contacts (sizeof 0xD0, copied by @0x66f064)
struct CollisionPair {
    struct Contact {
        math::vec3<float> pointA, pointB, normal;  // 0x0, 0xC, 0x18
        float distance;                            // 0x24
        unsigned materialIdA, materialIdB;         // 0x28, 0x2C
        Contact() : pointA(0, 0, 0), pointB(0, 0, 0), normal(0, 0, 0), distance(0), materialIdA(0), materialIdB(0) {}
    };
    CollisionPair() : bodyA(0), bodyB(0), numContacts(0), tail(0) {}
    CollisionBody *bodyA, *bodyB;  // 0x0, 0x4
    int numContacts;               // 0x8
    Contact contacts[4];           // 0xC
    int tail;                      // 0xCC
};

#endif
