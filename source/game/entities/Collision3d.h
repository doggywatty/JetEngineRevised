// source/game/entities/Collision3d.h
#ifndef SOURCE_GAME_ENTITIES_COLLISION3D_H
#define SOURCE_GAME_ENTITIES_COLLISION3D_H

#include "GameEntity.h"
#include "collision3dlistener.h"
#include "physics/collisionevent.h"

class RigidBody;
class CollisionShape;
class PhysicsWorld;

// The collision of an object: a rigid body of the physics world in the shape the data gives (a box the size of the entity, a
// collision mesh, a shape definition) that follows the entity. What touches it is told to its listener (the object it is of).
class Collision3d : public GameEntity {  // sizeof 0xB8
public:
    // what shape the body is made in
    struct CollisionShapeTypeDef { enum type { Box = 0, Mesh = 1, ShapeDef = 3 }; };
    // how the physics treats the body
    struct CollisionRoleDef { enum type { Static = 0, Kinematic = 1, Dynamic = 2 }; };
    explicit Collision3d(GameEntity*);  // @0x55d708: a copy of the entity
    explicit Collision3d(const clara::Template*);  // @0x55d69c
    virtual ~Collision3d();  // @0x55d7a4
    virtual void Init();  // @0x55d94c
    virtual void PostInit();  // @0x55d95c
    virtual void Update() {}  // @0x55def8
    virtual void SetEnabled(bool);  // @0x55defc
    virtual void NotifyTransformChanged(bool);  // @0x55df38
    virtual bool IsOutOfScene() const;  // @0x55dffc
    virtual void OnPositionedInWorld();  // @0x55e064
    RigidBody *GetCollisionBody() const { return m_rigidBody; }
    // tells the listener that the other collision touches this one
    void CollidesWith(Collision3d *other, const CollisionPair &pair);  // @0x55dfd4
    void SetCollisionListener(Collision3dListener *l) { m_collisionListener = l; }  // @0x55dff4
    Collision3dListener *GetCollisionListener() const { return m_collisionListener; }
    unsigned short GetCollisionGroup() const { return m_collisionGroup; }
    unsigned short GetCollisionMask() const { return m_collisionMask; }
    bool IsGroundCollision() const { return m_isGroundCollision; }
    void SetIsGroundCollision(bool b) { m_isGroundCollision = b; }
    unsigned GetEffectGroupId() const { return m_effectGroupId; }
    void AddToWorld();  // @0x55d8dc
    void RemoveFromWorld();  // @0x55d8a4
    void RemoveFromWorld(PhysicsWorld*);  // @0x55d87c
    static jet::String k_tmplName;
private:
    void AddToWorld(PhysicsWorld*);  // @0x55d914
    void RefreshBodyState(bool enabled);  // @0x55df1c
    int m_type;  // 0x8C
    RigidBody *m_rigidBody;  // 0x90
    CollisionShape *m_collisionShape;  // 0x94
    bool m_isAddedToWorld;  // 0x98
    unsigned short m_collisionGroup;  // 0x9A
    unsigned short m_collisionMask;  // 0x9C
    float m_sizeExtent;  // 0xA0: how far from the entity the collision reaches
    bool m_isGroundCollision;  // 0xA4
    unsigned m_effectGroupId;  // 0xA8
    Collision3dListener *m_collisionListener;  // 0xAC
    int m_role;  // 0xB0
    bool m_placedByCode;  // 0xB4: it enters the world when its owner places it
    static const jet::String k_effectGroupId, k_collisionShape, k_shapeDef, k_meshModel, k_overrideMeshMaterials, k_collisionGroup, k_collisionMask,
        k_collisionRole, k_mass, k_friction, k_placedByCode;
};

#endif
