// core/jet/include/scene/mesh.h
#ifndef CORE_JET_INCLUDE_SCENE_MESH_H
#define CORE_JET_INCLUDE_SCENE_MESH_H

#include <boost/noncopyable.hpp>
#include "smath.h"
#include "stream/istream.h"

namespace jet::scene {
class ModelBase;
class SubMesh;

// One LOD of a multires mesh: bounding box and submeshes (Mesh::LoadV100 @0x72cbdc).
class Mesh : private boost::noncopyable {
public:
    explicit Mesh(ModelBase*);
    ~Mesh();
    int Load(jet::stream::IStream&);
    const math::aabb<float> &GetBoundingBox() const { return m_boundingBox; }
    unsigned GetSubMeshCount() const { return m_subMeshCount; }
    SubMesh &GetSubMesh(unsigned i) { return *m_subMeshes[i]; }
    const SubMesh &GetSubMesh(unsigned i) const { return *m_subMeshes[i]; }
    bool IsSkinned() const { return m_isSkinned; }
    ModelBase &GetModelBase() const { return *m_base; }
protected:
    int LoadV100(jet::stream::IStream&);
    ModelBase *m_base;
    bool m_isSkinned;
    unsigned m_subMeshCount;
    SubMesh **m_subMeshes;
    math::aabb<float> m_boundingBox;
};
}

#endif
