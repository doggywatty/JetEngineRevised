// core/jet/src/video/shaderattribute.cpp
#include "video/shaderattribute.h"

namespace jet::video {

typedef safe_enum<Geometry::SemanticDef, unsigned char> Semantic;

ShaderAttribute::ShaderAttribute() : m_hasClientData(false), m_clientData(0), m_type(Geometry::SemanticDef::Custom) {}
ShaderAttribute::ShaderAttribute(const ShaderAttribute &o)
    : m_hasClientData(o.m_hasClientData), m_clientData(o.m_clientData), m_type(o.m_type), m_attributeName(o.m_attributeName) {}
ShaderAttribute::ShaderAttribute(Semantic type) : m_hasClientData(false), m_clientData(0), m_type(type) {}
ShaderAttribute::~ShaderAttribute() {}

ShaderAttribute &ShaderAttribute::operator=(const ShaderAttribute &o) {
    m_hasClientData = o.m_hasClientData;
    m_clientData = o.m_clientData;
    m_type = o.m_type;
    m_attributeName = o.m_attributeName;
    return *this;
}

bool ShaderAttribute::operator==(const ShaderAttribute &o) const {
    return m_type == o.m_type && m_attributeName == o.m_attributeName;
}
bool ShaderAttribute::operator!=(const ShaderAttribute &o) const { return !(*this == o); }

const jet::String &ShaderAttribute::GetAttributeName() const { return m_attributeName; }
Semantic ShaderAttribute::GetType() const { return m_type; }
void ShaderAttribute::SetType(Semantic t) { m_type = t; }

unsigned ShaderAttribute::GetClientData() const { return m_clientData; }
void ShaderAttribute::SetClientData(unsigned d) { m_hasClientData = true; m_clientData = d; }
bool ShaderAttribute::HasClientData() const { return m_hasClientData; }
void ShaderAttribute::RemoveClientData() { m_hasClientData = false; m_clientData = 0; }

void ShaderAttribute::Clear() { m_hasClientData = false; m_clientData = 0; m_attributeName = jet::String(); }

void ShaderAttribute::ParseValue(const jet::String &name) {
    using S = Geometry::SemanticDef;
    S::type t;
    if (name.Equals("mesh.positions")) t = S::Position;
    else if (name.Equals("mesh.normals")) t = S::Normal;
    else if (name.Equals("mesh.tangents") || name.Equals("mesh.tangents0")) t = S::Tangent;
    else if (name.Equals("mesh.tangents1")) t = S::Tangent1;
    else if (name.Equals("mesh.tangents2")) t = S::Tangent2;
    else if (name.Equals("mesh.tangents3")) t = S::Tangent3;
    else if (name.Equals("mesh.uv0")) t = S::TexCoord0;
    else if (name.Equals("mesh.uv1")) t = S::TexCoord1;
    else if (name.Equals("mesh.uv2")) t = S::TexCoord2;
    else if (name.Equals("mesh.uv3")) t = S::TexCoord3;
    else if (name.Equals("mesh.colors")) t = S::Color;
    else if (name.Equals("mesh.nodeindices")) t = S::NodeIndices;
    else if (name.Equals("mesh.nodeweights")) t = S::NodeWeights;
    else t = S::Custom;
    m_type = Semantic(t);

    if (name.find("mesh.") == 0) {
        m_attributeName = name.substr(5);
        // "tangents" without an index is renamed to "tangents0" for the Tangent semantic.
        if (m_attributeName.Equals("tangents")) m_attributeName = jet::String("tangents0");
    } else {
        m_attributeName = name;
    }
}

}
