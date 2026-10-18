// core/jet/include/video/renderjob.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERJOB_H
#define CORE_JET_INCLUDE_VIDEO_RENDERJOB_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "jenum.h"
#include "smath.h"
#include "core/pstring.h"
#include "video/geometry.h"
#include "video/shaderprogramflavor.h"

namespace jet::video {
class Material;

class RenderJob {  // sizeof 0x94
public:
    RenderJob();
    bool IsValid() const;
    void Reset();
    void SetShaderFlavorType(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>);
    safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> GetShaderFlavorType() const;
    void SetRange(unsigned, unsigned);
    unsigned GetRangeStart() const;
    unsigned GetRangeCount() const;
    void SetRadius(float);
    float GetRadius() const;
    void SetLod(unsigned);
    unsigned GetLod() const;
    void SetCenter(const math::vec3<float>&);
    const math::vec3<float> &GetCenter() const;
    void SetCenteredOnCamera(bool);
    bool IsCenteredOnCamera() const;
    void SetTransform(const math::mat4<float>*);
    void SetTransform(const math::mat4<float>&, bool isIdentity);
    // with position dequantization folded in: m * translate(offset) * scale(scale)
    void SetTransform(const math::mat4<float>&, const math::vec3<float> &offset, const math::vec3<float> &scale);
    const math::mat4<float> &GetTransform() const;
    bool IsTransformIdentity() const;
    // node weights per vertex of a hardware skinned job (0: not skinned); picks the flavor
    void SetSkinWeightCount(unsigned n) { m_skinWeightCount = n; }
    unsigned GetSkinWeightCount() const { return m_skinWeightCount; }
    void SetNodeTransformCount(unsigned);
    unsigned GetNodeTransformCount() const;
    void SetNodeTransform(unsigned, const math::mat4<float>&);
    const math::mat4<float> &GetNodeTransform(unsigned) const;
    math::mat4<float> *GetNodeTransformsPtr();
    void SetMaterial(const Material*);
    const Material *GetMaterial() const;
    void SetGeometry(const boost::shared_ptr<Geometry>&);
    const boost::shared_ptr<Geometry> &GetGeometry() const;
    void SetDebugNodeName(const jet::String&);
    const jet::String &GetDebugNodeName() const;
    void SetDebugModelName(const jet::String&);
    const jet::String &GetDebugModelName() const;
    void SetDebugColor(unsigned);
    unsigned GetDebugColor() const;
    void SetLocked(bool);
    void SetId(unsigned);
    unsigned GetId() const;
    void SetWireframe(bool);
    bool IsWireframe() const;
protected:
    unsigned char m_lod;
    bool m_isTransformIdentity;
    bool m_isCenteredOnCamera;
    bool m_isValid;
    safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> m_shaderFlavorType;
    unsigned m_rangeStart;
    unsigned m_rangeCount;
    const Material *m_material;
    math::mat4<float> m_transform;
    boost::shared_ptr<Geometry> m_geometry;
    math::vec3<float> m_center;
    float m_radius;
    std::vector<math::mat4<float> > m_nodeTransforms;
    unsigned m_skinWeightCount = 0;
    unsigned m_debugColor;
    jet::String m_debugNodeName;
    jet::String m_debugModelName;
    unsigned m_id;
    bool m_isWireframe;
    bool m_isLocked;
};
}

#endif
