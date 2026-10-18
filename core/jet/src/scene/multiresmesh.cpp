// core/jet/src/scene/multiresmesh.cpp
#include "scene/multiresmesh.h"
#include "scene/meshinstance.h"
#include "scene/mesh.h"
#include "scene/model.h"
#include "scene/dynamicmeshinstance.h"

namespace jet::scene {

MultiResMesh::MultiResMesh() : m_model(0), m_node(0), m_nodeIdx(0), m_isVisible(true) {
    for (unsigned i = 0; i < ModelBase::k_maxLodCount; i++) m_loadedMeshInstances[i] = m_meshInstances[i] = 0;
}

MultiResMesh::~MultiResMesh() {
    for (unsigned i = 0; i < ModelBase::k_maxLodCount; i++) delete m_loadedMeshInstances[i];
}

// Load @0x703a64: skinned lods get a DynamicMeshInstance
void MultiResMesh::Load(const ModelBase::MultiResMeshData &d) {
    m_nodeIdx = d.nodeIdx;
    m_node = &m_model->GetNode(d.nodeIdx);
    for (unsigned i = 0; i < ModelBase::k_maxLodCount && i < d.lods.size(); i++) {
        if (!d.lods[i]) continue;
        m_loadedMeshInstances[i] = d.lods[i]->IsSkinned() ? new DynamicMeshInstance(this, d.lods[i])
                                                          : new MeshInstance(this, d.lods[i]);
    }
    BuildMeshArray();
}

// BuildMeshArray @0x703b68: every lod slot points at the nearest loaded instance, preferring finer
void MultiResMesh::BuildMeshArray() {
    for (int lod = 0; lod < (int)ModelBase::k_maxLodCount; lod++) {
        MeshInstance *mi = 0;
        for (int l = lod; l >= 0 && !mi; l--) mi = m_loadedMeshInstances[l];
        for (int l = lod + 1; l < (int)ModelBase::k_maxLodCount && !mi; l++) mi = m_loadedMeshInstances[l];
        m_meshInstances[lod] = mi;
    }
}

// Render @0x703cec
void MultiResMesh::Render() {
    if (!m_isVisible) return;
    MeshInstance *mi = GetMeshInstance(m_model->GetLod());
    if (mi && mi->IsVisible()) mi->Render();
}

}
