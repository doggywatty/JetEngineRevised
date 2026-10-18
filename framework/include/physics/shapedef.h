// framework/include/physics/shapedef.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_SHAPEDEF_H
#define FRAMEWORK_INCLUDE_PHYSICS_SHAPEDEF_H

#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "res/resource.h"

// A primitive of a collision definition (.shapedef): where it sits relative to the body and what it is
class Shape {  // sizeof 0x30
public:
    struct TypeDef { enum type { Box = 0, Sphere = 1, Capsule = 2, Cylinder = 3, ConvexHull = 4 }; };
    Shape();  // @0x653b44
    virtual ~Shape() {}
    virtual int GetType() const = 0;
    virtual Shape *Clone() const = 0;
    jet::String m_name;                 // 0x4
    math::vec3<float> m_position;       // 0x8
    math::quat<float> m_rotation;       // 0x14
    math::vec3<float> m_scale;          // 0x24
};

class BoxShape : public Shape {  // sizeof 0x3C
public:
    BoxShape();  // @0x6547ec
    virtual int GetType() const { return TypeDef::Box; }
    virtual Shape *Clone() const;  // @0x654838
    math::vec3<float> m_extents;  // 0x30: width, length, height
};

class SphereShape : public Shape {  // sizeof 0x34
public:
    SphereShape();  // @0x654b50
    virtual int GetType() const { return TypeDef::Sphere; }
    virtual Shape *Clone() const;  // @0x654b94
    float m_radius;  // 0x30
};

class CapsuleShape : public Shape {  // sizeof 0x38
public:
    CapsuleShape();  // @0x654d9c
    virtual int GetType() const { return TypeDef::Capsule; }
    virtual Shape *Clone() const;  // @0x654de4
    float m_radius;  // 0x30
    float m_height;  // 0x34
};

class CylinderShape : public Shape {  // sizeof 0x38
public:
    CylinderShape();  // @0x654fbc
    virtual int GetType() const { return TypeDef::Cylinder; }
    virtual Shape *Clone() const;  // @0x655004
    float m_radius;  // 0x30
    float m_height;  // 0x34
};

class ConvexHullShape : public Shape {  // sizeof 0x50
public:
    ConvexHullShape();  // @0x6551dc
    virtual int GetType() const { return TypeDef::ConvexHull; }
    virtual Shape *Clone() const;  // @0x655248
    std::vector<math::vec3<float> > m_vertices;  // 0x30
    std::vector<unsigned short> m_triangles;     // 0x40: 3 indices a triangle
};

// A set of primitives and the volume that holds them
class ShapeDef : public jet::res::Resource {  // sizeof 0x38
public:
    ShapeDef();   // @0x653ba8
    virtual ~ShapeDef();
    virtual bool Init(jet::stream::IStream*);  // @0x653d54
    virtual bool Load();                       // @0x653d60: the xml of the file
    virtual void Unload() {}                   // @0x654584
    unsigned GetShapeCount() const { return (unsigned)m_shapes.size(); }
    const Shape *GetShape(unsigned i) const { return m_shapes[i]; }
    Shape *GetShape(unsigned i) { return m_shapes[i]; }
    const Shape *GetShapeByName(const jet::String&) const;
    const math::aabb<float> &GetVolume() const { return m_volume; }
    boost::shared_ptr<ShapeDef> CreateSubDef(const jet::String &name) const;
    boost::shared_ptr<ShapeDef> Clone() const;
    void AddCopy(const Shape*);
private:
    void Clear();
    void ComputeVolume();
    std::vector<Shape*> m_shapes;  // 0x10
    math::aabb<float> m_volume;    // 0x20
};

// Loads and keeps the .shapedef files by name
class ShapeDefLoader {  // sizeof 0x34
public:
    static ShapeDefLoader *GetInstance();
    static void FreeInstance();
    ShapeDefLoader();
    ~ShapeDefLoader();
    const ShapeDef *Load(const jet::String&);
    void Free(const ShapeDef*);
    void FreeAll();
    void CleanUp() {}
private:
    boost::unordered_map<jet::String, boost::shared_ptr<ShapeDef> > m_map;  // 0x4
    std::vector<ShapeDef*> m_array;  // 0x20
    static ShapeDefLoader *s_instance;
};

#endif
