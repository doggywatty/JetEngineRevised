// core/jet/include/scene/multiresmesh.h
#ifndef CORE_JET_INCLUDE_SCENE_MULTIRESMESH_H
#define CORE_JET_INCLUDE_SCENE_MULTIRESMESH_H

#include <boost/noncopyable.hpp>
#include "scene/modelbase.h"

namespace jet::scene {
class Model;
class Node;
class MeshInstance;

// The lods of one mesh of a model instance, hung on a node (MultiResMesh, 0x40 bytes).
class MultiResMesh : private boost::noncopyable {
public:
    MultiResMesh();
    ~MultiResMesh();
    void SetModel(Model *m) { m_model = m; }
    void Load(const ModelBase::MultiResMeshData&);
    Model *GetModel() const { return m_model; }
    const Node *GetNode() const { return m_node; }
    // lods 1 (finest) .. 5; a missing lod resolves to the closest finer one, else coarser
    MeshInstance *GetMeshInstance(unsigned lod) const { return lod < ModelBase::k_maxLodCount ? m_meshInstances[lod] : 0; }
    MeshInstance *GetLoadedMeshInstance(unsigned lod) const { return lod < ModelBase::k_maxLodCount ? m_loadedMeshInstances[lod] : 0; }
    void Render();
    void SetVisible(bool v) { m_isVisible = v; }
    bool IsVisible() const { return m_isVisible; }
protected:
    void BuildMeshArray();
    Model *m_model;
    MeshInstance *m_loadedMeshInstances[ModelBase::k_maxLodCount];
    MeshInstance *m_meshInstances[ModelBase::k_maxLodCount];
    const Node *m_node;
    unsigned short m_nodeIdx;
    bool m_isVisible;
};
}

#endif
