// core/jet/include/video/shadersampler.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERSAMPLER_H
#define CORE_JET_INCLUDE_VIDEO_SHADERSAMPLER_H

#include "jenum.h"
#include "core/pstring.h"
#include "video/texture.h"

namespace jet::video {
class ShaderSampler {  // sizeof 0x20
public:
    // SemanticDef verified against libdespicablemefree.so 0.0.3, ShaderSampler::ParseSemantic's
    // string table @0x61ee88 ("Diffuse".."Displacement"), Custom is the unmatched-but-nonempty case.
    struct SemanticDef {
        enum type { Diffuse, Ambient, Bump, SpecularCol, SpecularLvl, Glossiness, SelfIllum,
                    Opacity, FilterCol, Reflection, Refraction, Displacement, Custom };
    };

    ShaderSampler(const ShaderSampler&);
    ShaderSampler();
    ~ShaderSampler();
    ShaderSampler &operator=(const ShaderSampler&);
    bool operator==(const ShaderSampler&) const;
    bool operator!=(const ShaderSampler&) const;
    void SetServerData(unsigned);
    unsigned GetServerData() const;
    bool HasServerData() const;
    unsigned GetClientData() const;
    void SetClientData(unsigned);
    bool HasClientData() const;
    void RemoveClientData();
    safe_enum<Texture::WrapDef,unsigned char> GetWrapS() const;
    safe_enum<Texture::WrapDef,unsigned char> GetWrapT() const;
    safe_enum<Texture::FilterDef,unsigned char> GetFilter() const;
    unsigned GetAnisotropy() const;
    safe_enum<SemanticDef,SemanticDef::type> GetSemantic() const;
    int GetUVSet() const;
    jet::String GetBinding() const;
    void ParseBinding(const jet::String&);
    void ParseWrap(const jet::String&, const jet::String&);
    void ParseFilter(const jet::String&);
    void ParseSemantic(const jet::String&);
    void ParseUVSet(const jet::String&);
    void ParseAnisotropy(const jet::String&);
protected:
    bool m_hasClientData;
    bool m_hasServerData;
    unsigned m_clientData;
    unsigned m_serverData;
    safe_enum<Texture::WrapDef,unsigned char> m_wrapS;
    safe_enum<Texture::WrapDef,unsigned char> m_wrapT;
    safe_enum<Texture::FilterDef,unsigned char> m_filter;
    safe_enum<SemanticDef,SemanticDef::type> m_semantic;
    int m_uvSet;
    unsigned m_anisotropy;
    jet::String m_binding;
};
}

#endif
