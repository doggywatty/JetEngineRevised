// core/jet/src/scene/dynamicmeshinstance.cpp
#include "scene/dynamicmeshinstance.h"
#include "scene/multiresmesh.h"
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "scene/model.h"
#include "scene/node.h"
#include "video/driver.h"
#include <cstring>

namespace jet::scene {

// SubMesh skin node records (0x1c): node-local bounds, then the model node index
static unsigned SkinNodeIdx(const SubMesh::NodeInfo &ni) {
    unsigned short idx;
    memcpy(&idx, ni.data + 0x18, 2);
    return idx;
}

DynamicMeshInstance::DynamicMeshInstance(MultiResMesh *mrm, boost::shared_ptr<Mesh> mesh) : MeshInstance(mrm, mesh) {
    Model *model = mrm->GetModel();
    m_skinNodes.resize(mesh->GetSubMeshCount());
    for (unsigned i = 0; i < mesh->GetSubMeshCount(); i++) {
        const SubMesh &sm = mesh->GetSubMesh(i);
        for (unsigned n = 0; n < sm.GetSkinNodeCount(); n++)
            m_skinNodes[i].push_back(&model->GetNode(SkinNodeIdx(sm.GetSkinNodeInfos()[n])));
    }
}

DynamicMeshInstance::~DynamicMeshInstance() {}

void ComputeSkinMatrices(unsigned count, const math::mat4<float> *nodes, const math::mat4<float> *invBind, math::mat4<float> *out) {
    for (unsigned i = 0; i < count; i++) out[i] = nodes[i] * invBind[i];
}

void DynamicMeshInstance::RenderSkinnedSubMesh(unsigned i) {
    SubMeshData &d = m_subMeshData[i];
    if (!d.visible || !d.material) return;
    const SubMesh &sm = m_mesh->GetSubMesh(i);
    jet::video::RenderJob &job = GetRenderJob(i);
    job.SetMaterial(d.material);
    const std::vector<const Node*> &nodes = m_skinNodes[i];
    std::vector<math::mat4<float> > abs(nodes.size());
    for (unsigned n = 0; n < nodes.size(); n++) abs[n] = nodes[n]->GetAbsoluteTransform();
    job.SetNodeTransformCount((unsigned)nodes.size());
    const math::mat4<float> *invBind = reinterpret_cast<const math::mat4<float>*>(
        sm.GetSkinData() + sizeof(SubMesh::NodeInfo) * sm.GetSkinNodeCount());
    if (!nodes.empty()) ComputeSkinMatrices((unsigned)nodes.size(), &abs[0], invBind, job.GetNodeTransformsPtr());
    // RenderSubMesh @0x72b2dc: quantized positions get T(offset) * S(scale) folded into every
    // skin matrix; the skinned vertices are then in world space
    if (sm.GetPositionsQuantization()) {
        const math::vec3<float> &o = sm.GetPositionsQuantizationOffset(), &sc = sm.GetPositionsQuantizationScale();
        math::mat4<float> *m = job.GetNodeTransformsPtr();
        for (unsigned n = 0; n < nodes.size(); n++) {
            float *t = m[n].m;
            t[12] += t[0] * o.x + t[4] * o.y + t[8] * o.z;
            t[13] += t[1] * o.x + t[5] * o.y + t[9] * o.z;
            t[14] += t[2] * o.x + t[6] * o.y + t[10] * o.z;
            for (int r = 0; r < 3; r++) { t[r] *= sc.x; t[4 + r] *= sc.y; t[8 + r] *= sc.z; }
        }
    }
    job.SetTransform(math::mat4<float>(), true);
    job.SetSkinWeightCount(sm.GetWeightCount() ? sm.GetWeightCount() : 1);
    job.SetLod(m_multiResMesh->GetModel()->GetLod());
    jet::video::Driver::GetCurrent()->DrawPrimitives(job);
}

// RenderSubMesh @0x72b3e0 (software path): the nodes' absolute transforms go to a SkinTask that
// skins the submesh's streams; the job draws them untransformed
void DynamicMeshInstance::RenderSWSkinnedSubMesh(unsigned i) {
    SubMeshData &d = m_subMeshData[i];
    if (!d.visible || !d.material) return;
    const SubMesh &sm = m_mesh->GetSubMesh(i);
    jet::video::RenderJob &job = GetRenderJob(i);
    job.SetMaterial(d.material);
    const std::vector<const Node*> &nodes = m_skinNodes[i];
    std::vector<math::mat4<float> > abs(nodes.size());
    for (unsigned n = 0; n < nodes.size(); n++) abs[n] = nodes[n]->GetAbsoluteTransform();
    sm.GetGeometry();
    if (!nodes.empty()) sm.Skin(&abs[0]);
    job.SetTransform(math::mat4<float>(), true);
    job.SetSkinWeightCount(0);
    job.SetLod(m_multiResMesh->GetModel()->GetLod());
    jet::video::Driver::GetCurrent()->DrawPrimitives(job);
}

void DynamicMeshInstance::Render() {
    UpdateBoundingVolume();
    for (int i = (int)m_mesh->GetSubMeshCount() - 1; i >= 0; i--) {
        const SubMesh &sm = m_mesh->GetSubMesh((unsigned)i);
        if (sm.IsHWSkinned()) RenderSkinnedSubMesh((unsigned)i);
        else if (sm.IsSWSkinned()) RenderSWSkinnedSubMesh((unsigned)i);
        else RenderSubMesh((unsigned)i, m_multiResMesh->GetNode());
    }
}

}
