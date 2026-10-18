// core/jet/src/scene/node.cpp
#include "scene/node.h"
#include "scene/model.h"

namespace jet::scene {

// Node() @0x703d80: identity, versions seeded so the first Get* recomputes everything
Node::Node()
    : m_model(0), m_flags(0), m_rot(0, 0, 0, 1), m_scale(1, 1, 1), m_pos(0, 0, 0), m_transformVersion(0x51),
      m_cachedTransformVersion(0x1fbb), m_cachedAbsoluteTransformVersion(0x13d52), m_absoluteRotation(0, 0, 0, 1),
      m_data(0), m_parent(0), m_hasRotation(false), m_hasScale(false) {}

Node::~Node() {}

// Load @0x703e8c
void Node::Load(const ModelBase::NodeData &d) {
    m_data = &d;
    m_flags = d.flags;
    m_parent = d.parentIdx < 0 ? 0 : &m_model->GetMutableNode((unsigned)d.parentIdx);
    SetTransform(d.position, d.rotation, d.scale);
}

void Node::AttachTo(Node *p) { m_parent = p; ++m_transformVersion; }

static bool IsIdentity(const math::quat<float> &q, float eps) {
    return fabsf(q.x) <= eps && fabsf(q.y) <= eps && fabsf(q.z) <= eps && fabsf(q.w - 1.0f) <= eps;
}
static bool IsOne(const math::vec3<float> &v, float eps) {
    return fabsf(v.x - 1.0f) <= eps && fabsf(v.y - 1.0f) <= eps && fabsf(v.z - 1.0f) <= eps;
}

void Node::SetPosition(const math::vec3<float> &p) { SetTransform(p, m_rot, m_scale); }
void Node::SetRotation(const math::quat<float> &q) { SetTransform(m_pos, q, m_scale); }
void Node::SetScale(const math::vec3<float> &s) { SetTransform(m_pos, m_rot, s); }
void Node::SetTransform(const math::vec3<float> &p, const math::quat<float> &q) { SetTransform(p, q, m_scale); }

// SetTransform @0x7043e4 (epsilon 0x34000000 = 2^-23)
void Node::SetTransform(const math::vec3<float> &p, const math::quat<float> &q, const math::vec3<float> &s) {
    if (p == m_pos && q.x == m_rot.x && q.y == m_rot.y && q.z == m_rot.z && q.w == m_rot.w && s == m_scale) return;
    const float eps = 1.1920929e-07f;
    m_pos = p;
    m_rot = q;
    m_hasRotation = !IsIdentity(q, eps);
    m_scale = s;
    m_hasScale = !IsOne(s, eps);
    ++m_transformVersion;
}

void Node::UpdateTransform() const {
    if (m_cachedTransformVersion != m_transformVersion) _UpdateTransform();
}

// _UpdateTransform @0x704560
void Node::_UpdateTransform() const {
    m_cachedTransformVersion = m_transformVersion;
    if (m_hasRotation) m_transform = math::mat4<float>(m_rot.getAsMat3());
    else m_transform.setIdentity();
    if (m_hasScale) {
        for (int r = 0; r < 3; r++) {
            m_transform.m[r] *= m_scale.x;
            m_transform.m[4 + r] *= m_scale.y;
            m_transform.m[8 + r] *= m_scale.z;
        }
    }
    m_transform.m[12] = m_pos.x;
    m_transform.m[13] = m_pos.y;
    m_transform.m[14] = m_pos.z;
}

// UpdateAbsoluteTransform @0x704a88: a node's absolute transform is current when its cached
// version equals its parent's cached version plus its own transform version.
void Node::UpdateAbsoluteTransform() const {
    UpdateTransform();
    if (m_parent) {
        m_parent->UpdateAbsoluteTransform();
        unsigned v = m_parent->m_cachedAbsoluteTransformVersion + m_transformVersion;
        if (v != m_cachedAbsoluteTransformVersion) _UpdateAbsoluteTransformFromParent(v);
        return;
    }
    if (m_transformVersion == m_cachedAbsoluteTransformVersion) return;
    m_cachedAbsoluteTransformVersion = m_transformVersion;
    m_absoluteTransform = m_transform;
    m_absoluteRotation = m_rot;
}

// _UpdateAbsoluteTransformFromParent @0x7046d0
void Node::_UpdateAbsoluteTransformFromParent(unsigned version) const {
    m_cachedAbsoluteTransformVersion = version;
    m_absoluteTransform = m_parent->m_absoluteTransform * m_transform;
    m_absoluteRotation = m_parent->m_absoluteRotation * m_rot;
}

const Node *Node::GetChild(unsigned i) const {
    return &m_model->GetNode(m_data->children[i]);
}

math::vec3<float> Node::GetAbsolutePosition() const {
    const math::mat4<float> &m = GetAbsoluteTransform();
    return math::vec3<float>(m.m[12], m.m[13], m.m[14]);
}

}
