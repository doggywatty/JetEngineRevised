// core/jet/src/video/shaderdata.cpp
// No decompiled reference for this file was found in the 0.0.3 corpus (likely fully inlined at
// its call sites); the mechanics below (name->index maps, packed uniform data offsets) follow
// directly and unambiguously from the header's public contract.
#include "video/shaderdata.h"

namespace jet::video {

ShaderData::ShaderData() : m_totalUniformDataSize(0) {}
ShaderData::~ShaderData() {}

ShaderData &ShaderData::operator=(const ShaderData &o) {
    m_uniformMap = o.m_uniformMap; m_uniformNames = o.m_uniformNames; m_uniforms = o.m_uniforms;
    m_uniformMins = o.m_uniformMins; m_uniformMaxs = o.m_uniformMaxs;
    m_uniformDataOffsets = o.m_uniformDataOffsets; m_uniformDataSizes = o.m_uniformDataSizes;
    m_totalUniformDataSize = o.m_totalUniformDataSize;
    m_attributeMap = o.m_attributeMap; m_attributeNames = o.m_attributeNames; m_attributes = o.m_attributes;
    m_samplerMap = o.m_samplerMap; m_samplerNames = o.m_samplerNames; m_samplers = o.m_samplers;
    m_alphaSamplerMap = o.m_alphaSamplerMap; m_alphaSamplerNames = o.m_alphaSamplerNames;
    m_alphaSamplers = o.m_alphaSamplers;
    return *this;
}

bool ShaderData::operator==(const ShaderData &o) const {
    return m_uniformNames == o.m_uniformNames && m_uniforms == o.m_uniforms &&
           m_attributeNames == o.m_attributeNames && m_samplerNames == o.m_samplerNames;
}
bool ShaderData::operator!=(const ShaderData &o) const { return !(*this == o); }

int ShaderData::GetId() const { return (int)(size_t)this; }

ShaderUniform &ShaderData::AddUniform(const jet::String &name, const ShaderUniform &value, const ShaderUniform &lo, const ShaderUniform &hi) {
    unsigned idx = (unsigned)m_uniforms.size();
    m_uniformMap[name] = idx;
    m_uniformNames.push_back(name);
    m_uniforms.push_back(value);
    m_uniformMins.push_back(lo);
    m_uniformMaxs.push_back(hi);
    m_uniformDataOffsets.push_back(m_totalUniformDataSize);
    m_uniformDataSizes.push_back(value.GetDataSize());
    m_totalUniformDataSize += value.GetDataSize();
    return m_uniforms.back();
}
ShaderUniform &ShaderData::AddUniform(const jet::String &name, const ShaderUniform &value) {
    return AddUniform(name, value, value, value);
}

unsigned ShaderData::GetUniformCount() const { return (unsigned)m_uniforms.size(); }
const jet::String &ShaderData::GetUniformName(unsigned i) const { return m_uniformNames[i]; }
ShaderUniform &ShaderData::GetUniform(unsigned i) { return m_uniforms[i]; }
const ShaderUniform &ShaderData::GetUniform(unsigned i) const { return m_uniforms[i]; }
const ShaderUniform &ShaderData::GetUniformMin(unsigned i) const { return m_uniformMins[i]; }
const ShaderUniform &ShaderData::GetUniformMax(unsigned i) const { return m_uniformMaxs[i]; }

int ShaderData::FindUniformIdxByName(const jet::String &name) const {
    boost::unordered_map<jet::String,unsigned>::const_iterator it = m_uniformMap.find(name);
    return it == m_uniformMap.end() ? -1 : (int)it->second;
}
const ShaderUniform *ShaderData::FindUniformByName(const jet::String &name) const {
    int idx = FindUniformIdxByName(name);
    return idx < 0 ? 0 : &m_uniforms[idx];
}

unsigned ShaderData::GetUniformDataSize(unsigned i) const { return m_uniformDataSizes[i]; }
unsigned ShaderData::GetUniformDataOffset(unsigned i) const { return m_uniformDataOffsets[i]; }
unsigned ShaderData::GetTotalUniformDataSize() const { return m_totalUniformDataSize; }

void ShaderData::AddAttribute(const jet::String &name, const ShaderAttribute &attr) {
    m_attributeMap[name] = (unsigned)m_attributes.size();
    m_attributeNames.push_back(name);
    m_attributes.push_back(attr);
}
unsigned ShaderData::GetAttributeCount() const { return (unsigned)m_attributes.size(); }
const jet::String &ShaderData::GetAttributeName(unsigned i) const { return m_attributeNames[i]; }
unsigned ShaderData::GetUsedAttributes() const {
    unsigned mask = 0;
    for (unsigned i = 0; i < m_attributes.size(); i++)
        mask |= 1u << (unsigned)(Geometry::SemanticDef::type)m_attributes[i].GetType();
    return mask;
}
ShaderAttribute &ShaderData::GetAttribute(unsigned i) { return m_attributes[i]; }
const ShaderAttribute &ShaderData::GetAttribute(unsigned i) const { return m_attributes[i]; }
const ShaderAttribute *ShaderData::FindAttributeByName(const jet::String &name) const {
    boost::unordered_map<jet::String,unsigned>::const_iterator it = m_attributeMap.find(name);
    return it == m_attributeMap.end() ? 0 : &m_attributes[it->second];
}

void ShaderData::AddSampler(const jet::String &name, const ShaderSampler &s) {
    m_samplerMap[name] = (unsigned)m_samplers.size();
    m_samplerNames.push_back(name);
    m_samplers.push_back(s);
}
unsigned ShaderData::GetSamplerCount() const { return (unsigned)m_samplers.size(); }
const jet::String &ShaderData::GetSamplerName(unsigned i) const { return m_samplerNames[i]; }
ShaderSampler &ShaderData::GetSampler(unsigned i) { return m_samplers[i]; }
const ShaderSampler &ShaderData::GetSampler(unsigned i) const { return m_samplers[i]; }
int ShaderData::FindSamplerIdxByName(const jet::String &name) const {
    boost::unordered_map<jet::String,unsigned>::const_iterator it = m_samplerMap.find(name);
    return it == m_samplerMap.end() ? -1 : (int)it->second;
}
ShaderSampler *ShaderData::FindSamplerByName(const jet::String &name) {
    int idx = FindSamplerIdxByName(name);
    return idx < 0 ? 0 : &m_samplers[idx];
}
const ShaderSampler *ShaderData::FindSamplerByName(const jet::String &name) const {
    int idx = FindSamplerIdxByName(name);
    return idx < 0 ? 0 : &m_samplers[idx];
}

void ShaderData::SetAlphaSampler(unsigned i, const jet::String &name, const ShaderSampler &s) {
    if (i >= m_alphaSamplers.size()) {
        m_alphaSamplerNames.resize(i + 1);
        m_alphaSamplers.resize(i + 1);
    }
    m_alphaSamplerMap[name] = i;
    m_alphaSamplerNames[i] = name;
    m_alphaSamplers[i] = s;
}
unsigned ShaderData::GetAlphaSamplerCount() const { return (unsigned)m_alphaSamplers.size(); }
const jet::String &ShaderData::GetAlphaSamplerName(unsigned i) const { return m_alphaSamplerNames[i]; }
ShaderSampler &ShaderData::GetAlphaSampler(unsigned i) { return m_alphaSamplers[i]; }
const ShaderSampler &ShaderData::GetAlphaSampler(unsigned i) const { return m_alphaSamplers[i]; }
const ShaderSampler *ShaderData::FindAlphaSamplerByName(const jet::String &name) const {
    boost::unordered_map<jet::String,unsigned>::const_iterator it = m_alphaSamplerMap.find(name);
    return it == m_alphaSamplerMap.end() ? 0 : &m_alphaSamplers[it->second];
}

}
