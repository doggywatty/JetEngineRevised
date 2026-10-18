// core/jet/include/scene/dynamicmeshinstance.h
#ifndef CORE_JET_INCLUDE_SCENE_DYNAMICMESHINSTANCE_H
#define CORE_JET_INCLUDE_SCENE_DYNAMICMESHINSTANCE_H

#include "scene/meshinstance.h"

namespace jet::scene {

// A skinned mesh lod (DynamicMeshInstance, RenderSubMesh @0x72b038). Each submesh lists the model
// nodes it is skinned to; hardware-skinned submeshes get node absolute transform * inverse bind
// matrix per node as the job's node transforms (ComputeSkinMatrices @0x733e20).
class DynamicMeshInstance : public MeshInstance {
public:
    DynamicMeshInstance(MultiResMesh*, boost::shared_ptr<Mesh>);
    virtual ~DynamicMeshInstance();
    virtual void Render();
protected:
    void RenderSkinnedSubMesh(unsigned);
    void RenderSWSkinnedSubMesh(unsigned);
    std::vector<std::vector<const Node*> > m_skinNodes;
};

void ComputeSkinMatrices(unsigned count, const math::mat4<float> *nodes, const math::mat4<float> *invBind, math::mat4<float> *out);
}

#endif
