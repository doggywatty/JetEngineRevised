// core/jet/src/video/shaderprogram.cpp
#include "video/shaderprogram.h"
#include "video/gles20/gles20shaderprogram.h"
#include "pugixml.hpp"

namespace jet::video {

ShaderProgram::ShaderProgram(RenderTechnique *tech) : m_technique(tech), m_isReady(false) {}
ShaderProgram::~ShaderProgram() {}

ShaderProgram *ShaderProgram::New(RenderTechnique *tech) {
    return new GLES20ShaderProgram(tech);
}

void ShaderProgram::SetDebugName(const jet::String &n) { m_debugName = n; }
const jet::String &ShaderProgram::GetDebugName() const { return m_debugName; }
bool ShaderProgram::IsReady() const { return m_isReady; }

// Load @0x6a8638: <shader version=> (absent or 100)
void ShaderProgram::Load(const pugi::xml_node &node) {
    pugi::xml_attribute v = node.attribute("version");
    if (!v || v.as_int() == 100) LoadV100(node);
}

}
