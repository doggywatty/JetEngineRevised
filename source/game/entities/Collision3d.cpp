// source/game/entities/Collision3d.cpp
#include "Collision3d.h"
#include "../GameLevel.h"
#include "Minion.h"
#include "physics/bullet/bulletphysicsfactory.h"
#include "physics/collisionmesh.h"
#include "physics/shapedef.h"
#include "physics/physicsworld.h"
#include "param.h"
#include <algorithm>

jet::String Collision3d::k_tmplName("Collision3d");
const jet::String Collision3d::k_effectGroupId("EffectGroup"), Collision3d::k_collisionShape("CollisionShape"), Collision3d::k_shapeDef("ShapeDef"),
    Collision3d::k_meshModel("MeshModel"), Collision3d::k_overrideMeshMaterials("OverrideMeshMaterials"), Collision3d::k_collisionGroup("CollisionGroup"),
    Collision3d::k_collisionMask("CollisionMask"), Collision3d::k_collisionRole("CollisionRole"), Collision3d::k_mass("Mass"), Collision3d::k_friction("Friction"),
    Collision3d::k_placedByCode("PlacedByCode");

// @0x55d69c: group 2 meets mask 5 unless the data says otherwise, a kinematic body
Collision3d::Collision3d(const clara::Template *t)
    : GameEntity(t), m_type(0), m_rigidBody(0), m_collisionShape(0), m_isAddedToWorld(false), m_collisionGroup(2), m_collisionMask(5), m_sizeExtent(0),
      m_isGroundCollision(true), m_effectGroupId(0), m_collisionListener(0), m_role(CollisionRoleDef::Kinematic), m_placedByCode(false) {}

// @0x55d708
Collision3d::Collision3d(GameEntity *e)
    : GameEntity(e->GetTemplate()), m_type(0), m_rigidBody(0), m_collisionShape(0), m_isAddedToWorld(false), m_collisionGroup(2), m_collisionMask(5), m_sizeExtent(0),
      m_isGroundCollision(true), m_effectGroupId(0), m_collisionListener(0), m_role(CollisionRoleDef::Kinematic), m_placedByCode(false) {
    ConstructFrom(static_cast<const GameEntity*>(e));
}

// @0x55d7a4: the body leaves the world and is destroyed with its shape
Collision3d::~Collision3d() {
    if (m_isAddedToWorld) RemoveFromWorld();
    if (GameLevel *l = GameLevel::GetInstance()) {
        PhysicsFactory *f = l->GetPhysicsFactory();
        if (m_rigidBody) f->Destroy(m_rigidBody);
        if (m_collisionShape) f->Destroy(m_collisionShape);
    }
}

// @0x55d94c: the entities of the libraries are only descriptions
void Collision3d::Init() {
    if (IsLibraryEntity()) return;
    GameEntity::Init();
}

// @0x55d95c: the body in the shape of the data, in its group and mask, in the world unless something else puts it there
void Collision3d::PostInit() {
    if (IsLibraryEntity()) return;
    GameLevel *level = GameLevel::GetInstance();
    int type = 0;
    GetParam(k_collisionShape, type);
    m_type = type;
    if (HasParam(k_effectGroupId)) {
        int id = 0;
        GetParam(k_effectGroupId, id);
        m_effectGroupId = (unsigned)id;
    }
    int role = 0;
    GetParam(k_collisionRole, role);
    m_role = role;
    PhysicsFactory *factory = level->GetPhysicsFactory();
    if (m_type == CollisionShapeTypeDef::Box) {
        // a box the size of the entity
        ShapeDef def;
        BoxShape box;
        box.m_extents = GetScale();
        def.AddCopy(&box);
        m_collisionShape = factory->CreateShape(&def, 0);
        m_sizeExtent = std::max(std::max(box.m_extents.x, box.m_extents.y), box.m_extents.z);
    } else if (m_type == CollisionShapeTypeDef::ShapeDef) {
        jet::String name;
        GetParam(k_shapeDef, name);
        ShapeDefLoader *loader = ShapeDefLoader::GetInstance();
        const ShapeDef *def = loader->Load(name);
        m_sizeExtent = def->GetVolume().getRadius();
        m_collisionShape = factory->CreateShape(def, 0);
        loader->Free(def);
    } else if (m_type == CollisionShapeTypeDef::Mesh) {
        jet::String name;
        GetParam(k_meshModel, name);
        CollisionMeshLoader *loader = CollisionMeshLoader::GetInstance();
        CollisionMesh *mesh = loader->Load(name);
        bool override = false;
        GetParam(k_overrideMeshMaterials, override);
        const math::vec3<float> scale = GetScale();
        m_collisionShape = factory->CreateShape(mesh, scale);
        const float r = mesh->GetBoundingVolume().getRadius();
        m_sizeExtent = math::vec3<float>(r * scale.x, r * scale.y, r * scale.z).getLength();
        loader->Free(mesh);
    }
    if (HasParam(k_collisionGroup)) {
        int g = 0;
        GetParam(k_collisionGroup, g);
        m_collisionGroup = (unsigned short)(1 << g);
    }
    if (HasParam(k_collisionMask)) {
        m_collisionMask = 0;
        for (unsigned i = 0; i < FindParamByName(k_collisionMask)->GetComponentCount(); i++) {
            int m = 0;
            GetParam(k_collisionMask, m, i);
            m_collisionMask |= (unsigned short)(1 << m);
        }
    }
    m_rigidBody = factory->CreateRigidBody(m_collisionShape);
    m_rigidBody->SetRole(m_role);
    m_rigidBody->SetCollisionFilter(m_collisionGroup, m_collisionMask);
    m_rigidBody->SetUserData(this);
    m_placedByCode = false;
    if (m_role == CollisionRoleDef::Dynamic) {
        float mass = 0, friction = 0;
        GetParam(k_mass, mass);
        m_rigidBody->SetMass(mass);
        GetParam(k_friction, friction);
        m_rigidBody->SetFriction(friction);
    } else if (m_role == CollisionRoleDef::Static)
        GetParam(k_placedByCode, m_placedByCode);
    GameEntity::PostInit();
    if (m_role != CollisionRoleDef::Static || !m_placedByCode) AddToWorld();
    RegisterForUpdate(true);
}

// @0x55d8dc, 0x55d914: the body enters the world and is as enabled as the entity
void Collision3d::AddToWorld() { AddToWorld(GameLevel::GetInstance()->GetPhysicsWorld()); }
void Collision3d::AddToWorld(PhysicsWorld *w) {
    w->AddBody(m_rigidBody);
    m_isAddedToWorld = true;
    RefreshBodyState(IsEnabled());
}

// @0x55d8a4, 0x55d87c
void Collision3d::RemoveFromWorld() { RemoveFromWorld(GameLevel::GetInstance()->GetPhysicsWorld()); }
void Collision3d::RemoveFromWorld(PhysicsWorld *w) {
    w->RemoveBody(m_rigidBody);
    m_isAddedToWorld = false;
}

// @0x55df1c: in the world exactly while enabled
void Collision3d::RefreshBodyState(bool enabled) {
    if (m_isAddedToWorld == enabled) return;
    if (enabled) AddToWorld();
    else RemoveFromWorld();
}

// @0x55defc
void Collision3d::SetEnabled(bool b) {
    GameEntity::SetEnabled(b);
    RefreshBodyState(b);
}

// @0x55df38: the body goes where the entity does; a static one is out of the world while it moves
void Collision3d::NotifyTransformChanged(bool b) {
    bool back = false;
    if (m_rigidBody->GetRole() == RigidBody::RoleDef::Static) {
        back = m_isAddedToWorld;
        if (back) RemoveFromWorld();
    }
    GameEntity::NotifyTransformChanged(b);
    m_rigidBody->SetTransform(GetPosition(), GetRotation());
    if (back) AddToWorld();
}

// @0x55dfd4
void Collision3d::CollidesWith(Collision3d *other, const CollisionPair &pair) {
    if (m_collisionListener) m_collisionListener->CollidesWith(other, pair);
}

// @0x55dffc: pending to delete, and behind the player's run (the minion's own is never)
bool Collision3d::IsOutOfScene() const {
    if (!GetPendingToDelete()) return false;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (minion->GetGroundCollision() == this) return false;
    return minion->GetTraveledDistance() >= m_distanceToDelete;
}

// @0x55e064: what a code places enters the world only now
void Collision3d::OnPositionedInWorld() {
    if (m_role == CollisionRoleDef::Static && m_placedByCode) AddToWorld();
}
