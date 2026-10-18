// framework/src/physics/bullet/bulletcollisionshape.cpp
#include "physics/bullet/bulletcollisionshape.h"
#include "physics/shapedef.h"
#include "physics/collisionmesh.h"
#include "BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include <cmath>

// @0x6740e8: the shapes of a definition
BulletCollisionShape::BulletCollisionShape(const ShapeDef *def, unsigned materialId)
    : m_collisionShape(0), m_indexVertexArray(0), m_triangleInfoMap(0), m_materialId(materialId) {
    m_collisionShape = ConvertShapeDef(def);
    m_collisionShape->setUserPointer(this);
}

// @0x674168: the triangles of a mesh, each with its material (the id of it is what the contacts report); the mesh is scaled
// when asked to be, and the edges between its triangles are smoothed (btGenerateInternalEdgeInfo)
BulletCollisionShape::BulletCollisionShape(const CollisionMesh *mesh, const math::vec3<float> &scale)
    : m_collisionShape(0), m_indexVertexArray(0), m_triangleInfoMap(0), m_materialId(~0u) {
    const unsigned numVertices = (unsigned)mesh->GetVertices().size();
    const std::vector<unsigned> &ids = mesh->GetMaterials();
    m_materials.resize(ids.size());
    for (size_t i = 0; i < ids.size(); i++) {
        m_materials[i].pad[0] = (int)ids[i];
        m_materials[i].pad[1] = (int)numVertices;
    }
    m_indexVertexArray = new btTriangleIndexVertexMaterialArray(
        (int)mesh->GetNumTriangles(), const_cast<int*>(&mesh->GetIndexes()[0]), 12, (int)numVertices,
        const_cast<btScalar*>(&mesh->GetVertices()[0].x), 12, (int)m_materials.size(),
        m_materials.empty() ? 0 : reinterpret_cast<unsigned char*>(&m_materials[0]), (int)sizeof(btMaterial),
        reinterpret_cast<int*>(const_cast<unsigned char*>(&mesh->GetMaterialsIndexes()[0])), 1);
    btBvhTriangleMeshShape *bvh = new btMultimaterialTriangleMeshShape(m_indexVertexArray, true, true);
    if (scale.x != 1.0f || scale.y != 1.0f || scale.z != 1.0f) {
        bvh->setUserPointer(this);
        m_collisionShape = new btScaledBvhTriangleMeshShape(bvh, btVector3(scale.x, scale.y, scale.z));
    } else
        m_collisionShape = bvh;
    m_collisionShape->setUserPointer(this);
    m_triangleInfoMap = new btTriangleInfoMap();
    btGenerateInternalEdgeInfo(bvh, m_triangleInfoMap);
}

// @0x674538: the shape, the compound's children, the mesh's data
BulletCollisionShape::~BulletCollisionShape() {
    if (m_collisionShape) {
        if (m_collisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
            btCompoundShape *c = static_cast<btCompoundShape*>(m_collisionShape);
            while (c->getNumChildShapes() > 0) {
                btCollisionShape *child = c->getChildShape(c->getNumChildShapes() - 1);
                c->removeChildShapeByIndex(c->getNumChildShapes() - 1);
                delete child;
            }
        } else if (m_collisionShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
            delete static_cast<btScaledBvhTriangleMeshShape*>(m_collisionShape)->getChildShape();
        delete m_collisionShape;
    }
    delete m_indexVertexArray;
    delete m_triangleInfoMap;
}

// @0x6746b0: the material a contact is on: of the triangle for a mesh (when the part and the triangle are known), else the
// shape's own
unsigned BulletCollisionShape::GetMaterial(const CollisionMaterialQuery &q) const {
    if (q.partId != -1 && q.triangleIndex != -1) {
        btCollisionShape *s = m_collisionShape;
        if (s->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE) s = static_cast<btScaledBvhTriangleMeshShape*>(s)->getChildShape();
        if (s->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
            return (unsigned)static_cast<btMultimaterialTriangleMeshShape*>(s)->getMaterialProperties(q.partId, q.triangleIndex)->pad[0];
    }
    return m_materialId;
}

namespace {
// a rotation as the angles about the three axes, as the game derives them (@0x674a54) to know whether a shape is turned
void QuatToAngles(float x, float y, float z, float w, float a[3]) {
    a[0] = a[1] = a[2] = 0;
    const float s = 2 * (x * z + y * w);
    if (s >= 0.99999f) {
        a[1] = 1.5707964f;
        a[0] = atan2f(2 * (y * x + w * z), 1 - 2 * (z * z + x * x));
    } else if (s <= -0.99999f) {
        a[1] = -1.5707964f;
        a[0] = -atan2f(2 * (y * x + w * z), 1 - 2 * (z * z + x * x));
    } else {
        a[1] = asinf(s);
        a[0] = atan2f(2 * (w * x - y * z), 1 - 2 * (y * y + x * x));
        a[2] = atan2f(2 * (w * z - x * y), 1 - 2 * (z * z + y * y));
    }
}
bool IsZero3(const float *v, float eps) { return fabsf(v[0]) < eps && fabsf(v[1]) < eps && fabsf(v[2]) < eps; }
}

// @0x674718: a primitive a Bullet shape, and the one alone a shape in itself when it is in place; else all of them the children
// of a compound at their places. The definition's rotations are inverted (the quaternion's vector part negated) as everywhere
// the game hands them to Bullet.
btCollisionShape *BulletCollisionShape::ConvertShapeDef(const ShapeDef *def) {
    std::vector<btCollisionShape*> shapes;
    for (unsigned i = 0; i < def->GetShapeCount(); i++) {
        const Shape *s = def->GetShape(i);
        btCollisionShape *b = 0;
        switch (s->GetType()) {
        case Shape::TypeDef::Box: {
            const BoxShape *x = static_cast<const BoxShape*>(s);
            b = new btBoxShape(btVector3(x->m_extents.x * 0.5f, x->m_extents.y * 0.5f, x->m_extents.z * 0.5f));
            break;
        }
        case Shape::TypeDef::Sphere: b = new btSphereShape(static_cast<const SphereShape*>(s)->m_radius); break;
        case Shape::TypeDef::Capsule: b = new btCapsuleShapeZ(static_cast<const CapsuleShape*>(s)->m_radius, static_cast<const CapsuleShape*>(s)->m_height); break;
        case Shape::TypeDef::Cylinder: {
            const CylinderShape *x = static_cast<const CylinderShape*>(s);
            b = new btCylinderShapeZ(btVector3(x->m_radius, x->m_radius, x->m_height * 0.5f));
            break;
        }
        case Shape::TypeDef::ConvexHull: {
            const ConvexHullShape *x = static_cast<const ConvexHullShape*>(s);
            b = new btConvexHullShape(x->m_vertices.empty() ? 0 : &x->m_vertices[0].x, (int)x->m_vertices.size(), 12);
            break;
        }
        }
        shapes.push_back(b);
    }
    bool compound;
    if (def->GetShapeCount() == 1) {
        const Shape *s = def->GetShape(0);
        float angles[3];
        QuatToAngles(s->m_rotation.x, s->m_rotation.y, s->m_rotation.z, s->m_rotation.w, angles);
        const float p[3] = { s->m_position.x, s->m_position.y, s->m_position.z };
        compound = !IsZero3(p, 0.01f) || !IsZero3(angles, 0.01f);
    } else
        compound = true;
    if (!compound) return shapes[0];
    btCompoundShape *c = new btCompoundShape(true);
    for (unsigned i = 0; i < def->GetShapeCount(); i++) {
        const Shape *s = def->GetShape(i);
        btTransform t(btQuaternion(-s->m_rotation.x, -s->m_rotation.y, -s->m_rotation.z, s->m_rotation.w).normalized(), btVector3(s->m_position.x, s->m_position.y, s->m_position.z));
        c->addChildShape(t, shapes[i]);
    }
    return c;
}
