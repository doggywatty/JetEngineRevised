// core/jet/include/video/shaderattribute.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERATTRIBUTE_H
#define CORE_JET_INCLUDE_VIDEO_SHADERATTRIBUTE_H

#include "jenum.h"
#include "core/pstring.h"
#include "video/geometry.h"

namespace jet::video {
class ShaderAttribute {  // sizeof 0x10
public:
    ShaderAttribute(const jet::video::ShaderAttribute&);
    explicit ShaderAttribute(safe_enum<jet::video::Geometry::SemanticDef,unsigned char>);
    ShaderAttribute();
    ~ShaderAttribute();
    jet::video::ShaderAttribute &operator=(const jet::video::ShaderAttribute&);
    bool operator==(const jet::video::ShaderAttribute&) const;
    bool operator!=(const jet::video::ShaderAttribute&) const;
    const jet::String &GetAttributeName() const;
    safe_enum<jet::video::Geometry::SemanticDef,unsigned char> GetType() const;
    void SetType(safe_enum<jet::video::Geometry::SemanticDef,unsigned char>);
    unsigned GetClientData() const;
    void SetClientData(unsigned);
    bool HasClientData() const;
    void RemoveClientData();
    // Reads a "mesh.<semantic>" name into m_type/m_attributeName; the original renames a bare
    // "mesh.tangents" attribute name to "mesh.tangents0" once it recognizes the Tangent semantic.
    void ParseValue(const jet::String&);
protected:
    void Clear();
    bool m_hasClientData;  // 0x0
    unsigned m_clientData;  // 0x4
    safe_enum<jet::video::Geometry::SemanticDef,unsigned char> m_type;  // 0x8
    jet::String m_attributeName;  // 0xC
};
}

#endif
