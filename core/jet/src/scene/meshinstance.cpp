// core/jet/src/scene/meshinstance.cpp
#include "scene/meshinstance.h"
#include "scene/multiresmesh.h"
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "scene/model.h"
#include "scene/node.h"
#include "video/driver.h"
#include "video/material.h"

namespace jet::scene {

MeshInstance::MeshInstance(MultiResMesh *mrm, boost::shared_ptr<Mesh> mesh)
    : m_multiResMesh(mrm), m_mesh(mesh), m_center(0, 0, 0), m_radius(0), m_isVisible(true) {
    m_subMeshData.resize(mesh->GetSubMeshCount());
    Model *model = mrm->GetModel();
    for (unsigned i = 0; i < m_subMeshData.size(); i++) {
        const SubMesh &sm = mesh->GetSubMesh(i);
        m_subMeshData[i].center = sm.GetCenter();
        int mi = model->FindMaterialIdxByName(sm.GetMaterialName());
        m_subMeshData[i].material = mi >= 0 ? &model->GetMaterial((unsigned)mi) : 0;
    }
    UpdateBoundingVolume();
}

MeshInstance::~MeshInstance() {}

unsigned MeshInstance::GetSubMeshCount() const { return m_mesh->GetSubMeshCount(); }
const SubMesh &MeshInstance::GetSubMesh(unsigned i) const { return m_mesh->GetSubMesh(i); }

// UpdateBoundingVolume @0x6fd5e8: the mesh box moved by the node's absolute transform
void MeshInstance::UpdateBoundingVolume() {
    const math::aabb<float> &b = m_mesh->GetBoundingBox();
    const math::mat4<float> &m = m_multiResMesh->GetNode()->GetAbsoluteTransform();
    math::vec3<float> c = (b.minPoint + b.maxPoint) * 0.5f;
    math::vec3<float> e = (b.maxPoint - b.minPoint) * 0.5f;
    math::vec3<float> wc(m.m[0] * c.x + m.m[4] * c.y + m.m[8] * c.z + m.m[12],
                         m.m[1] * c.x + m.m[5] * c.y + m.m[9] * c.z + m.m[13],
                         m.m[2] * c.x + m.m[6] * c.y + m.m[10] * c.z + m.m[14]);
    math::vec3<float> we(fabsf(m.m[0]) * e.x + fabsf(m.m[4]) * e.y + fabsf(m.m[8]) * e.z,
                         fabsf(m.m[1]) * e.x + fabsf(m.m[5]) * e.y + fabsf(m.m[9]) * e.z,
                         fabsf(m.m[2]) * e.x + fabsf(m.m[6]) * e.y + fabsf(m.m[10]) * e.z);
    m_boundingBox = math::aabb<float>(wc - we, wc + we);
    m_center = wc;
    m_radius = we.getLength();
}

// CreateRenderJob @0x6fd2e0
void MeshInstance::CreateRenderJob(unsigned i) {
    SubMeshData &d = m_subMeshData[i];
    const SubMesh &sm = m_mesh->GetSubMesh(i);
    d.job.SetDebugNodeName(m_multiResMesh->GetNode()->GetName());
    d.job.SetMaterial(d.material);
    d.job.SetGeometry(sm.GetGeometry());
    d.job.SetCenter(d.center);
    d.job.SetRadius(m_radius);
    d.jobCreated = true;
}

jet::video::RenderJob &MeshInstance::GetRenderJob(unsigned i) {
    if (!m_subMeshData[i].jobCreated) CreateRenderJob(i);
    return m_subMeshData[i].job;
}

// RenderSubMesh @0x6fd834: quantized positions get the dequantization folded into the transform
void MeshInstance::RenderSubMesh(unsigned i, const Node *node) {
    SubMeshData &d = m_subMeshData[i];
    if (!d.visible || !d.material) return;
    jet::video::RenderJob &job = GetRenderJob(i);
    job.SetMaterial(d.material);
    const SubMesh &sm = m_mesh->GetSubMesh(i);
    if (sm.GetPositionsQuantization())
        job.SetTransform(node->GetAbsoluteTransform(), sm.GetPositionsQuantizationOffset(), sm.GetPositionsQuantizationScale());
    else
        job.SetTransform(node->GetAbsoluteTransform(), false);
    job.SetLod(m_multiResMesh->GetModel()->GetLod());
    jet::video::Driver::GetCurrent()->DrawPrimitives(job);
}

// Render @0x6fda50: submeshes are submitted last to first
void MeshInstance::Render() {
    UpdateBoundingVolume();
    const Node *node = m_multiResMesh->GetNode();
    for (int i = (int)m_mesh->GetSubMeshCount() - 1; i >= 0; i--) RenderSubMesh((unsigned)i, node);
}

}
