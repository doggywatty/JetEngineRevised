// core/jet/include/scene/model.h
#ifndef CORE_JET_INCLUDE_SCENE_MODEL_H
#define CORE_JET_INCLUDE_SCENE_MODEL_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "scene/modelbase.h"
#include "scene/node.h"
#include "scene/dummy.h"
#include "scene/renderable.h"
#include "video/material.h"
#include "anim/animcontroller.h"

namespace jet::anim { class Animation; }

namespace jet::scene {
class Node;
class MultiResMesh;
class Camera;
class ModelAnimator;

// An instance of a ModelBase: its own nodes (transforms), materials, lods and cameras
// (Model::Model @0x6fde9c).
class Model : public Renderable {
public:
    explicit Model(boost::shared_ptr<const ModelBase>);
    virtual ~Model();

    const boost::shared_ptr<const ModelBase> &GetBase() const { return m_base; }
    unsigned GetNodeCount() const { return m_base->GetNodeCount(); }
    const Node &GetNode(unsigned i) const { return m_nodes[i]; }
    Node &GetMutableNode(unsigned i) { return m_nodes[i]; }
    const Node *FindNodeByName(const jet::String&) const;
    int FindNodeIdxByName(const jet::String &n) const { return m_base->FindNodeIdxByName(n); }
    Node &GetRootNode() { return m_nodes[0]; }

    unsigned GetCameraCount() const { return m_base->GetCameraCount(); }
    Camera &GetCamera(unsigned i) const { return *m_cameras[i]; }
    int FindCameraIdxByName(const jet::String &n) const { return m_base->FindCameraIdxByName(n); }
    Node &GetCameraNode(unsigned i) { return m_cameraNodes[i]; }

    unsigned GetMultiResMeshCount() const { return m_base->GetMultiResMeshCount(); }
    MultiResMesh &GetMultiResMesh(unsigned i) const { return *m_multiResMeshes[i]; }

    unsigned GetMaterialCount() const { return m_base->GetMaterialCount(); }
    const jet::video::Material &GetMaterial(unsigned i) const { return m_materials[i]; }
    jet::video::Material &GetMutableMaterial(unsigned i) { return m_materials[i]; }
    const jet::String &GetMaterialName(unsigned i) const { return m_base->GetMaterialData(i).name; }
    int FindMaterialIdxByName(const jet::String &n) const { return m_base->FindMaterialIdxByName(n); }
    void SetMaterial(unsigned i, const jet::video::Material &m) { m_materials[i] = m; }
    void SetAllMaterials(const jet::video::Material&);

    unsigned GetLod() const;
    void SetLod(unsigned l) { m_lod = l; }
    void SetLodOffset(int o) { m_lodOffset = o; }

    void SetVisible(bool v) { m_isVisible = v; }
    bool IsVisible() const { return m_isVisible; }

    // Root placement of the instance in the world
    void SetTransform(const math::vec3<float>&, const math::quat<float>&, const math::vec3<float>&);

    void SetAnimator(ModelAnimator *a) { m_animator = a; }
    ModelAnimator *GetAnimator() const { return m_animator; }

    // Animation (SetAnimation @0x6884fc, SetTextureAnimation @0x688620, AnimateNodes @0x68892c): model nodes
    // are bound to the animation's channels by name. The node controller (CC) and the texture one (CTC)
    // are made on first use. When to sample nodes: 0 in Update, 1 in PreCull, 2 only when asked (Animate).
    struct AnimData {  // per node (0x2c)
        bool animated, hasPos, hasScale;
        math::vec3<float> oldPos;
        math::quat<float> oldRot;
        int cache[3];
        AnimData() : animated(false), hasPos(false), hasScale(false), oldPos(0, 0, 0), oldRot(0, 0, 0, 1) { cache[0] = cache[1] = cache[2] = -1; }
    };
    typedef jet::anim::AnimController<Model> Controller;
    Controller &CC() const;
    Controller &CTC() const;
    Controller &GetAnimController() { return CC(); }
    bool SetAnimation(jet::anim::Animation*, short interpolationMs = 0);
    jet::anim::Animation *GetAnimation() const { return m_animation; }
    bool SetTextureAnimation(jet::anim::Animation*);
    jet::anim::Animation *GetTextureAnimation() const { return m_texAnimation; }
    void SetUpdateMode(int m) { m_updateMode = m; }
    int GetUpdateMode() const { return m_updateMode; }
    void Animate(bool force) { if (m_animation) AnimateNodes(force); }
    void AnimateNodes(bool force);
    void Update(int dt);   // _Update @0x688870
    int GetLastAnimTime() const { return m_lastAnimTime; }
    unsigned char &AnimFlags() { return m_animFlags; }
    short &InterpolationTotal() { return m_interpTotal; }
    void RefreshMovementDelta(bool) const;
    const math::vec3<float> &GetMovementDelta() const { return m_moveDelta; }
    void ClearMovementDelta() { m_moveDelta = math::vec3<float>(0, 0, 0); }

    unsigned GetDummyCount() const { return (unsigned)m_dummies.size(); }
    Dummy *GetDummy(unsigned i) { return &m_dummies[i]; }
    Dummy *FindDummyByName(const jet::String&, unsigned n = 0);
    // the n-th dummy of that name is also found as FindDummyByNameAndIndex
    Dummy *FindDummyByNameAndIndex(const jet::String &name, unsigned n) { return FindDummyByName(name, n); }
    unsigned GetDummyCountByName(const jet::String&) const;
    void ResetDummies();

    // LOD thresholds in pixels of projected radius (s_lodScreenSize1-4: 140, 80, 20, 5)
    void SetLodScreenSize(unsigned lod, float s) { if (lod >= 1 && lod <= 4) m_lodScreenSize[lod - 1] = s; }
    float GetLodScreenSize(unsigned lod) const { return lod >= 1 && lod <= 4 ? m_lodScreenSize[lod - 1] : 0.f; }
    static void SetDefaultLodScreenSize(unsigned lod, float s) { if (lod >= 1 && lod <= 4) s_lodScreenSize[lod - 1] = s; }
    void SetAutoLod(bool b) { m_autoLod = b; }

    // the box of the meshes in world space at the current pose (ComputeAbsoluteAccurateBoundingBox @0x6879e0)
    math::aabb<float> ComputeAbsoluteAccurateBoundingBox();
    // the box of every lod of every mesh as the meshes hold it (ComputeAccurateBoundingBox @0x70114c)
    math::aabb<float> ComputeAccurateBoundingBox() const;
    unsigned GetRenderTagMask() override;
    bool PreCull() override;
    void PreRender() override;
    void Render() override;

protected:
    void UpdateBoundingVolume();
    static float s_lodScreenSize[4];
    float m_lodScreenSize[4];
    bool m_autoLod;
    boost::shared_ptr<const ModelBase> m_base;
    std::vector<jet::video::Material> m_materials;
    std::vector<Node> m_nodes;
    std::vector<Node> m_cameraNodes;
    std::vector<MultiResMesh*> m_multiResMeshes;
    std::vector<Camera*> m_cameras;
    jet::anim::Animation *m_animation, *m_texAnimation;
    unsigned m_lod;
    int m_lodOffset;
    bool m_isVisible;
    ModelAnimator *m_animator;
    std::vector<Dummy> m_dummies;
    void RefreshAnimatedItems();
    void RefreshAnimatedMaterials();
    void ResetCaches();
    void SaveNodeOldData(unsigned i);
    void AnimateMaterials(bool force);
    mutable Controller *m_cc, *m_ctc;
    std::vector<AnimData> m_animData;
    std::vector<int> m_animItems;        // anim node | model node << 16, nodes
    std::vector<int> m_cameraItems;      // anim node | camera << 16
    std::vector<int> m_materialItems;    // anim material | model material << 16
    int m_updateMode;                    // 0x114
    unsigned char m_animFlags;           // 0xa7: 1 absolute blend, 2 rotation blend, 4 position blend, 0x80 interpolate
    bool m_wasAnimated;                  // 0xa6
    int m_lastAnimTime, m_lastTexTime;
    short m_interpCur, m_interpTotal;
    int m_movementNode, m_moveLastTime;
    math::vec3<float> m_moveLastPos, m_moveDelta, m_moveAccum;
    bool m_moveWrapped;
};
}

#endif
