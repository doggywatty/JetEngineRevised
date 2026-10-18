// framework/include/physics/collisionshape.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONSHAPE_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONSHAPE_H

// Which part of a shape a contact is on (a mesh's part and triangle): -1 for the shape as a whole
struct CollisionMaterialQuery {
    int partId, triangleIndex;
    CollisionMaterialQuery() : partId(-1), triangleIndex(-1) {}
    CollisionMaterialQuery(int p, int t) : partId(p), triangleIndex(t) {}
};

class CollisionShape {  // sizeof 0x4
public:
    virtual ~CollisionShape() {}
    // the material id of the part
    virtual unsigned GetMaterial(const CollisionMaterialQuery&) const = 0;
};

#endif
