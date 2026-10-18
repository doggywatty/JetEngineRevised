// core/jet/include/scene/dummy.h
#ifndef CORE_JET_INCLUDE_SCENE_DUMMY_H
#define CORE_JET_INCLUDE_SCENE_DUMMY_H

#include "smath.h"
#include "core/pstring.h"
#include "scene/node.h"

namespace jet::scene {
// A named helper node of a model (Model::Model @0x686494): name, node and its initial local transform
class Dummy {  // sizeof 0x28
public:
    Dummy() : m_node(0), m_initPos(0, 0, 0), m_initRot(0, 0, 0, 1), m_index(0) {}
    Dummy(Node *n, const jet::String &name, unsigned index)
        : m_name(name), m_node(n), m_initPos(n->GetPosition()), m_initRot(n->GetRotation()), m_index(index) {}
    const jet::String &GetName() const { return m_name; }
    Node *GetNode() const { return m_node; }
    unsigned GetIndex() const { return m_index; }
    math::vec3<float> GetAbsolutePosition() const { return m_node->GetAbsolutePosition(); }
    const math::vec3<float> &GetPosition() const { return m_node->GetPosition(); }
    const math::quat<float> &GetRotation() const { return m_node->GetRotation(); }
    const math::quat<float> &GetAbsoluteRotation() const { return m_node->GetAbsoluteRotation(); }
    const math::mat4<float> &GetAbsoluteTransform() const { return m_node->GetAbsoluteTransform(); }
    // back to the pose the model was loaded with (ResetDummies @0x6878c8)
    void Reset() { m_node->SetTransform(m_initPos, m_initRot); }
private:
    jet::String m_name;        // 0x0
    Node *m_node;              // 0x4
    math::vec3<float> m_initPos;  // 0x8
    math::quat<float> m_initRot;  // 0x14
    unsigned m_index;          // 0x24
};
}

#endif
