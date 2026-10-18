// framework/include/physics/collisionmesh.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONMESH_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONMESH_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "res/resource.h"

// The triangles of a collision file (.coll): vertices, the triangles by their vertex indices and the material of each
class CollisionMesh : public jet::res::Resource {  // sizeof 0x78
public:
    CollisionMesh();  // @0x652be8
    virtual ~CollisionMesh() {}
    virtual bool Init(jet::stream::IStream*);  // @0x652d70
    virtual bool Load();                       // @0x652d7c
    virtual void Unload() {}                   // @0x6531b8
    unsigned GetNumTriangles() const { return m_numTriangles; }
    const std::vector<math::vec3<float> > &GetVertices() const { return m_vertices; }
    const std::vector<int> &GetIndexes() const { return m_indexes; }
    const std::vector<jet::String> &GetMaterialNames() const { return m_materialNames; }
    const std::vector<unsigned> &GetMaterials() const { return m_materials; }
    const std::vector<unsigned char> &GetMaterialsIndexes() const { return m_materialIndexes; }
    const math::aabb<float> &GetBoundingVolume() const { return m_volume; }  // @0x6531bc
    // gives the material of that name another id
    void OverrideMaterial(const jet::String&, unsigned);  // @0x6531c4
private:
    bool LoadV100(jet::stream::IStream&);  // @0x652e08
    unsigned m_numTriangles;                     // 0x10
    std::vector<math::vec3<float> > m_vertices;  // 0x14
    std::vector<int> m_indexes;                  // 0x24: three vertex indices a triangle
    std::vector<jet::String> m_materialNames;    // 0x34
    std::vector<unsigned> m_materials;           // 0x40: the id of each material
    std::vector<unsigned char> m_materialIndexes;  // 0x50: the material of each triangle
    math::aabb<float> m_volume;                  // 0x60
};

// Loads and keeps the .coll files by name
class CollisionMeshLoader {  // sizeof 0x34
public:
    static CollisionMeshLoader *GetInstance();
    static void FreeInstance();
    CollisionMeshLoader();
    ~CollisionMeshLoader();
    CollisionMesh *Load(const jet::String&);  // @0x65358c
    void Free(CollisionMesh*);                // @0x653484
    void FreeAll();                           // @0x653570
    void CleanUp() {}
private:
    boost::unordered_map<jet::String, boost::shared_ptr<CollisionMesh> > m_map;  // 0x4
    std::vector<CollisionMesh*> m_array;  // 0x1c
    static CollisionMeshLoader *s_instance;
};

#endif
