// core/jet/include/scene/meshinstance.h
#ifndef CORE_JET_INCLUDE_SCENE_MESHINSTANCE_H
#define CORE_JET_INCLUDE_SCENE_MESHINSTANCE_H

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "video/renderjob.h"

namespace jet::video { class Material; }

namespace jet::scene {
class Mesh;
class SubMesh;
class MultiResMesh;
class Node;

// One lod of a mesh in a model instance: a render job and material per submesh
// (MeshInstance, Render @0x6fda50, RenderSubMesh @0x6fd834).
class MeshInstance : private boost::noncopyable {
public:
    struct SubMeshData {  // 0xa4 in the original
        bool visible;
        math::vec3<float> center;
        const jet::video::Material *material;
        jet::video::RenderJob job;
        bool jobCreated;
        SubMeshData() : visible(true), center(0, 0, 0), material(0), jobCreated(false) {}
    };

    MeshInstance(MultiResMesh*, boost::shared_ptr<Mesh>);
    virtual ~MeshInstance();
    MultiResMesh &GetMultiResMesh() const { return *m_multiResMesh; }
    const Mesh &GetMesh() const { return *m_mesh; }
    unsigned GetSubMeshCount() const;
    const SubMesh &GetSubMesh(unsigned) const;
    void SetSubMeshMaterial(unsigned i, const jet::video::Material *m) { m_subMeshData[i].material = m; }
    const jet::video::Material *GetSubMeshMaterial(unsigned i) const { return m_subMeshData[i].material; }
    void SetSubMeshVisible(unsigned i, bool v) { m_subMeshData[i].visible = v; }
    jet::video::RenderJob &GetRenderJob(unsigned);
    const math::aabb<float> &GetBoundingBox() const { return m_boundingBox; }
    void UpdateBoundingVolume();
    virtual void Render();
    void SetVisible(bool v) { m_isVisible = v; }
    bool IsVisible() const { return m_isVisible; }
protected:
    void CreateRenderJob(unsigned);
    void RenderSubMesh(unsigned, const Node*);
    MultiResMesh *m_multiResMesh;
    boost::shared_ptr<Mesh> m_mesh;
    math::aabb<float> m_boundingBox;
    math::vec3<float> m_center;
    float m_radius;
    std::vector<SubMeshData> m_subMeshData;
    bool m_isVisible;
};
}

#endif
