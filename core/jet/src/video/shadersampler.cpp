// core/jet/src/video/shadersampler.cpp
#include "video/shadersampler.h"
#include <cstdlib>

namespace jet::video {

ShaderSampler::ShaderSampler()
    : m_hasClientData(false), m_hasServerData(false), m_clientData(0), m_serverData(0),
      m_wrapS(Texture::WrapDef::Repeat), m_wrapT(Texture::WrapDef::Repeat),
      m_filter(Texture::FilterDef::Bilinear), m_semantic(SemanticDef::Custom), m_uvSet(0),
      m_anisotropy(0) {
}

ShaderSampler::ShaderSampler(const ShaderSampler &o)
    : m_hasClientData(o.m_hasClientData), m_hasServerData(o.m_hasServerData),
      m_clientData(o.m_clientData), m_serverData(o.m_serverData), m_wrapS(o.m_wrapS),
      m_wrapT(o.m_wrapT), m_filter(o.m_filter), m_semantic(o.m_semantic), m_uvSet(o.m_uvSet),
      m_anisotropy(o.m_anisotropy), m_binding(o.m_binding) {
}

ShaderSampler::~ShaderSampler() {}

ShaderSampler &ShaderSampler::operator=(const ShaderSampler &o) {
    m_hasClientData = o.m_hasClientData;
    m_hasServerData = o.m_hasServerData;
    m_clientData = o.m_clientData;
    m_serverData = o.m_serverData;
    m_wrapS = o.m_wrapS;
    m_wrapT = o.m_wrapT;
    m_filter = o.m_filter;
    m_semantic = o.m_semantic;
    m_uvSet = o.m_uvSet;
    m_anisotropy = o.m_anisotropy;
    m_binding = o.m_binding;
    return *this;
}

bool ShaderSampler::operator==(const ShaderSampler &o) const {
    return m_wrapS == o.m_wrapS && m_wrapT == o.m_wrapT && m_filter == o.m_filter &&
           m_semantic == o.m_semantic && m_uvSet == o.m_uvSet && m_anisotropy == o.m_anisotropy &&
           m_binding == o.m_binding;
}
bool ShaderSampler::operator!=(const ShaderSampler &o) const { return !(*this == o); }

void ShaderSampler::SetServerData(unsigned d) { m_hasServerData = true; m_serverData = d; }
unsigned ShaderSampler::GetServerData() const { return m_serverData; }
bool ShaderSampler::HasServerData() const { return m_hasServerData; }
unsigned ShaderSampler::GetClientData() const { return m_clientData; }
void ShaderSampler::SetClientData(unsigned d) { m_hasClientData = true; m_clientData = d; }
bool ShaderSampler::HasClientData() const { return m_hasClientData; }
void ShaderSampler::RemoveClientData() { m_hasClientData = false; m_clientData = 0; }

safe_enum<Texture::WrapDef,unsigned char> ShaderSampler::GetWrapS() const { return m_wrapS; }
safe_enum<Texture::WrapDef,unsigned char> ShaderSampler::GetWrapT() const { return m_wrapT; }
safe_enum<Texture::FilterDef,unsigned char> ShaderSampler::GetFilter() const { return m_filter; }
unsigned ShaderSampler::GetAnisotropy() const { return m_anisotropy; }
safe_enum<ShaderSampler::SemanticDef,ShaderSampler::SemanticDef::type> ShaderSampler::GetSemantic() const { return m_semantic; }
int ShaderSampler::GetUVSet() const { return m_uvSet; }
jet::String ShaderSampler::GetBinding() const { return m_binding; }

void ShaderSampler::ParseBinding(const jet::String &s) { m_binding = s; }

void ShaderSampler::ParseWrap(const jet::String &s, const jet::String &t) {
    if (s.Equals("repeat")) m_wrapS = Texture::WrapDef::Repeat;
    else if (s.Equals("clamp")) m_wrapS = Texture::WrapDef::Clamp;
    if (t.Equals("repeat")) m_wrapT = Texture::WrapDef::Repeat;
    else if (t.Equals("clamp")) m_wrapT = Texture::WrapDef::Clamp;
}

void ShaderSampler::ParseFilter(const jet::String &s) {
    if (s.Equals("nearest")) m_filter = Texture::FilterDef::Nearest;
    else if (s.Equals("bilinear")) m_filter = Texture::FilterDef::Bilinear;
    else if (s.Equals("trilinear")) m_filter = Texture::FilterDef::Trilinear;
}

void ShaderSampler::ParseSemantic(const jet::String &s) {
    using D = SemanticDef;
    if (s.Equals("Diffuse")) m_semantic = D::Diffuse;
    else if (s.Equals("Ambient")) m_semantic = D::Ambient;
    else if (s.Equals("Bump")) m_semantic = D::Bump;
    else if (s.Equals("SpecularCol")) m_semantic = D::SpecularCol;
    else if (s.Equals("SpecularLvl")) m_semantic = D::SpecularLvl;
    else if (s.Equals("Glossiness")) m_semantic = D::Glossiness;
    else if (s.Equals("SelfIllum")) m_semantic = D::SelfIllum;
    else if (s.Equals("Opacity")) m_semantic = D::Opacity;
    else if (s.Equals("FilterCol")) m_semantic = D::FilterCol;
    else if (s.Equals("Reflection")) m_semantic = D::Reflection;
    else if (s.Equals("Refraction")) m_semantic = D::Refraction;
    else if (s.Equals("Displacement")) m_semantic = D::Displacement;
    else if (!s.empty()) m_semantic = D::Custom;
}

void ShaderSampler::ParseUVSet(const jet::String &s) { m_uvSet = atoi(s.c_str()); }
void ShaderSampler::ParseAnisotropy(const jet::String &s) { m_anisotropy = atoi(s.c_str()); }

}
