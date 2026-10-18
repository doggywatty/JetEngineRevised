// core/jet/include/scene/node.h
#ifndef CORE_JET_INCLUDE_SCENE_NODE_H
#define CORE_JET_INCLUDE_SCENE_NODE_H

#include <vector>
#include "smath.h"
#include "core/pstring.h"
#include "scene/modelbase.h"

namespace jet::scene {
class Model;

// A model instance's transform hierarchy node (jet::scene::Node, 0xd8 bytes in the 0.0.3 .so).
// The local transform is rotation (quat) and per-axis scale in the 3x3 part and the position in
// the translation column (_UpdateTransform @0x704560); the absolute one is parent absolute *
// local and the absolute rotation parent rotation * local rotation
// (_UpdateAbsoluteTransformFromParent @0x7046d0). Both are refreshed lazily from version counters.
class Node {
public:
    Node();
    ~Node();
    void Load(const ModelBase::NodeData&);

    const jet::String &GetName() const { return m_data->name; }
    const ModelBase::NodeData *GetData() const { return m_data; }
    unsigned GetFlags() const { return m_flags; }
    bool IsDummy() const { return (m_flags & ModelBase::NodeFlags::Dummy) != 0; }
    bool IsCamera() const { return (m_flags & ModelBase::NodeFlags::Camera) != 0; }
    Node *GetParent() const { return m_parent; }
    // @0x703f3c: the model's node of the i-th child of this node's data
    const Node *GetChild(unsigned i) const;
    void AttachTo(Node*);
    Model *GetModel() const { return m_model; }
    void SetModel(Model *m) { m_model = m; }

    const math::vec3<float> &GetPosition() const { return m_pos; }
    void SetPosition(const math::vec3<float>&);
    const math::quat<float> &GetRotation() const { return m_rot; }
    void SetRotation(const math::quat<float>&);
    const math::vec3<float> &GetScale() const { return m_scale; }
    void SetScale(const math::vec3<float>&);
    void SetTransform(const math::vec3<float>&, const math::quat<float>&);
    void SetTransform(const math::vec3<float>&, const math::quat<float>&, const math::vec3<float>&);

    const math::mat4<float> &GetTransform() const { UpdateTransform(); return m_transform; }
    const math::mat4<float> &GetAbsoluteTransform() const { UpdateAbsoluteTransform(); return m_absoluteTransform; }
    const math::quat<float> &GetAbsoluteRotation() const { UpdateAbsoluteTransform(); return m_absoluteRotation; }
    math::vec3<float> GetAbsolutePosition() const;

    void UpdateTransform() const;
    void UpdateAbsoluteTransform() const;

private:
    void _UpdateTransform() const;
    void _UpdateAbsoluteTransformFromParent(unsigned version) const;

    Model *m_model;
    unsigned m_flags;
    math::quat<float> m_rot;
    math::vec3<float> m_scale;
    math::vec3<float> m_pos;
    unsigned m_transformVersion;
    mutable unsigned m_cachedTransformVersion;
    mutable unsigned m_cachedAbsoluteTransformVersion;
    mutable math::mat4<float> m_transform;
    mutable math::mat4<float> m_absoluteTransform;
    mutable math::quat<float> m_absoluteRotation;
    const ModelBase::NodeData *m_data;
    Node *m_parent;
    bool m_hasRotation;
    bool m_hasScale;
};
}

#endif
