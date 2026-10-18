// core/jet/src/video/renderjob.cpp
#include "video/renderjob.h"

namespace jet::video {

RenderJob::RenderJob()
    : m_lod(0), m_isTransformIdentity(true), m_isCenteredOnCamera(false), m_isValid(false),
      m_shaderFlavorType(ShaderProgramFlavor::TypeDef::Normal), m_rangeStart(0), m_rangeCount(0),
      m_material(0), m_radius(0), m_debugColor(0xffffffff), m_id(0), m_isWireframe(false), m_isLocked(false) {
}

bool RenderJob::IsValid() const { return m_isValid && m_material && m_geometry; }

void RenderJob::Reset() {
    m_material = 0;
    m_geometry.reset();
    m_isTransformIdentity = true;
    m_isCenteredOnCamera = false;
    m_isValid = false;
    m_rangeStart = 0;
    m_rangeCount = 0;
    m_nodeTransforms.clear();
    m_isWireframe = false;
    m_isLocked = false;
}

void RenderJob::SetShaderFlavorType(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> t) { m_shaderFlavorType = t; }
safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> RenderJob::GetShaderFlavorType() const { return m_shaderFlavorType; }

void RenderJob::SetRange(unsigned start, unsigned count) { m_rangeStart = start; m_rangeCount = count; }
unsigned RenderJob::GetRangeStart() const { return m_rangeStart; }
unsigned RenderJob::GetRangeCount() const { return m_rangeCount; }

void RenderJob::SetRadius(float r) { m_radius = r; }
float RenderJob::GetRadius() const { return m_radius; }
void RenderJob::SetLod(unsigned l) { m_lod = (unsigned char)l; }
unsigned RenderJob::GetLod() const { return m_lod; }

void RenderJob::SetCenter(const math::vec3<float> &c) { m_center = c; }
const math::vec3<float> &RenderJob::GetCenter() const { return m_center; }
void RenderJob::SetCenteredOnCamera(bool v) { m_isCenteredOnCamera = v; }
bool RenderJob::IsCenteredOnCamera() const { return m_isCenteredOnCamera; }

void RenderJob::SetTransform(const math::mat4<float> *m) {
    if (m) { m_transform = *m; m_isTransformIdentity = false; }
    else { m_transform = math::mat4<float>(); m_isTransformIdentity = true; }
}
void RenderJob::SetTransform(const math::mat4<float> &m, bool isIdentity) { m_transform = m; m_isTransformIdentity = isIdentity; }
// SetTransform @0x6efdc8
void RenderJob::SetTransform(const math::mat4<float> &m, const math::vec3<float> &o, const math::vec3<float> &sc) {
    m_transform = m;
    float *t = m_transform.m;
    t[12] += t[0] * o.x + t[4] * o.y + t[8] * o.z;
    t[13] += t[1] * o.x + t[5] * o.y + t[9] * o.z;
    t[14] += t[2] * o.x + t[6] * o.y + t[10] * o.z;
    for (int r = 0; r < 3; r++) { t[r] *= sc.x; t[4 + r] *= sc.y; t[8 + r] *= sc.z; }
    m_isTransformIdentity = false;
}
const math::mat4<float> &RenderJob::GetTransform() const { return m_transform; }
bool RenderJob::IsTransformIdentity() const { return m_isTransformIdentity; }

void RenderJob::SetNodeTransformCount(unsigned c) { m_nodeTransforms.resize(c); }
unsigned RenderJob::GetNodeTransformCount() const { return (unsigned)m_nodeTransforms.size(); }
void RenderJob::SetNodeTransform(unsigned i, const math::mat4<float> &m) { m_nodeTransforms[i] = m; }
const math::mat4<float> &RenderJob::GetNodeTransform(unsigned i) const { return m_nodeTransforms[i]; }
math::mat4<float> *RenderJob::GetNodeTransformsPtr() { return m_nodeTransforms.empty() ? 0 : &m_nodeTransforms[0]; }

void RenderJob::SetMaterial(const Material *m) { m_material = m; m_isValid = m != 0; }
const Material *RenderJob::GetMaterial() const { return m_material; }
void RenderJob::SetGeometry(const boost::shared_ptr<Geometry> &g) { m_geometry = g; m_isValid = m_isValid && g.get() != 0; }
const boost::shared_ptr<Geometry> &RenderJob::GetGeometry() const { return m_geometry; }

void RenderJob::SetDebugNodeName(const jet::String &n) { m_debugNodeName = n; }
const jet::String &RenderJob::GetDebugNodeName() const { return m_debugNodeName; }
void RenderJob::SetDebugModelName(const jet::String &n) { m_debugModelName = n; }
const jet::String &RenderJob::GetDebugModelName() const { return m_debugModelName; }
void RenderJob::SetDebugColor(unsigned c) { m_debugColor = c; }
unsigned RenderJob::GetDebugColor() const { return m_debugColor; }

void RenderJob::SetLocked(bool v) { m_isLocked = v; }
void RenderJob::SetId(unsigned i) { m_id = i; }
unsigned RenderJob::GetId() const { return m_id; }
void RenderJob::SetWireframe(bool v) { m_isWireframe = v; }
bool RenderJob::IsWireframe() const { return m_isWireframe; }

}
