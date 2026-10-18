// framework/include/physics/bullet/bulletcollisionshape.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETCOLLISIONSHAPE_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETCOLLISIONSHAPE_H

#include <vector>
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionShapes/btMaterial.h"
#include "BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.h"
#include "physics/collisionshape.h"
#include "smath.h"

class ShapeDef;
class CollisionMesh;

// A Bullet shape: the primitives of a shape definition (one, or a compound of them) or the triangles of a collision mesh
class BulletCollisionShape : public CollisionShape {  // sizeof 0x28
public:
    BulletCollisionShape(const CollisionMesh*, const math::vec3<float> &scale);  // @0x674168
    BulletCollisionShape(const ShapeDef*, unsigned materialId);                  // @0x6740e8
    virtual ~BulletCollisionShape();                                              // @0x674538
    virtual unsigned GetMaterial(const CollisionMaterialQuery&) const;            // @0x6746b0
    btCollisionShape *GetBulletObject() const { return m_collisionShape; }        // @0x674710
private:
    btCollisionShape *ConvertShapeDef(const ShapeDef*);  // @0x674718
    btCollisionShape *m_collisionShape;  // 0x4
    // the id of each material of a mesh, in the second word of its btMaterial (@0x674210)
    std::vector<btMaterial> m_materials;  // 0x8
    btTriangleIndexVertexMaterialArray *m_indexVertexArray;  // 0x14 (0x1C in the PDB layout)
    btTriangleInfoMap *m_triangleInfoMap;  // 0x18
    unsigned m_materialId;  // 0x1C: of a shape definition, -1 for a mesh
};

#endif
