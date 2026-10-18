// core/jet/include/video/shaderdata.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERDATA_H
#define CORE_JET_INCLUDE_VIDEO_SHADERDATA_H

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include "core/pstring.h"
#include "video/shaderuniform.h"
#include "video/shaderattribute.h"
#include "video/shadersampler.h"

namespace jet::video {
class ShaderData : public boost::noncopyable {
public:
    ShaderData();
    ~ShaderData();
    ShaderData &operator=(const ShaderData&);
    bool operator==(const ShaderData&) const;
    bool operator!=(const ShaderData&) const;
    int GetId() const;
    ShaderUniform &AddUniform(const jet::String&, const ShaderUniform&, const ShaderUniform&, const ShaderUniform&);
    ShaderUniform &AddUniform(const jet::String&, const ShaderUniform&);
    unsigned GetUniformCount() const;
    const jet::String &GetUniformName(unsigned) const;
    ShaderUniform &GetUniform(unsigned);
    const ShaderUniform &GetUniform(unsigned) const;
    const ShaderUniform &GetUniformMin(unsigned) const;
    const ShaderUniform &GetUniformMax(unsigned) const;
    const ShaderUniform *FindUniformByName(const jet::String&) const;
    int FindUniformIdxByName(const jet::String&) const;
    unsigned GetUniformDataSize(unsigned) const;
    unsigned GetUniformDataOffset(unsigned) const;
    unsigned GetTotalUniformDataSize() const;
    void AddAttribute(const jet::String&, const ShaderAttribute&);
    unsigned GetAttributeCount() const;
    const jet::String &GetAttributeName(unsigned) const;
    unsigned GetUsedAttributes() const;
    ShaderAttribute &GetAttribute(unsigned);
    const ShaderAttribute &GetAttribute(unsigned) const;
    const ShaderAttribute *FindAttributeByName(const jet::String&) const;
    void AddSampler(const jet::String&, const ShaderSampler&);
    unsigned GetSamplerCount() const;
    const jet::String &GetSamplerName(unsigned) const;
    ShaderSampler &GetSampler(unsigned);
    const ShaderSampler &GetSampler(unsigned) const;
    ShaderSampler *FindSamplerByName(const jet::String&);
    const ShaderSampler *FindSamplerByName(const jet::String&) const;
    int FindSamplerIdxByName(const jet::String&) const;
    void SetAlphaSampler(unsigned, const jet::String&, const ShaderSampler&);
    unsigned GetAlphaSamplerCount() const;
    const jet::String &GetAlphaSamplerName(unsigned) const;
    ShaderSampler &GetAlphaSampler(unsigned);
    const ShaderSampler &GetAlphaSampler(unsigned) const;
    const ShaderSampler *FindAlphaSamplerByName(const jet::String&) const;
protected:
    boost::unordered_map<jet::String,unsigned> m_uniformMap;
    std::vector<jet::String> m_uniformNames;
    std::vector<ShaderUniform> m_uniforms;
    std::vector<ShaderUniform> m_uniformMins;
    std::vector<ShaderUniform> m_uniformMaxs;
    std::vector<unsigned> m_uniformDataOffsets;
    std::vector<unsigned> m_uniformDataSizes;
    unsigned m_totalUniformDataSize;
    boost::unordered_map<jet::String,unsigned> m_attributeMap;
    std::vector<jet::String> m_attributeNames;
    std::vector<ShaderAttribute> m_attributes;
    boost::unordered_map<jet::String,unsigned> m_samplerMap;
    std::vector<jet::String> m_samplerNames;
    std::vector<ShaderSampler> m_samplers;
    boost::unordered_map<jet::String,unsigned> m_alphaSamplerMap;
    std::vector<jet::String> m_alphaSamplerNames;
    std::vector<ShaderSampler> m_alphaSamplers;
};
}

#endif
