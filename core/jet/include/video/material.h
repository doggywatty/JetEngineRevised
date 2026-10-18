// core/jet/include/video/material.h
#ifndef CORE_JET_INCLUDE_VIDEO_MATERIAL_H
#define CORE_JET_INCLUDE_VIDEO_MATERIAL_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "core/pstring.h"
#include "video/rendertechnique.h"
#include "video/texturelayer.h"
#include "video/shaderuniform.h"

namespace pugi { class xml_node; }

namespace jet::video {
class Material {
public:
    Material(const Material&);
    Material();
    ~Material();
    Material &operator=(const Material&);
    bool operator==(const Material&) const;
    bool operator!=(const Material&) const;
    bool IsValid() const;
    bool EqualsPass(const Material&, unsigned) const;
    unsigned GetPassHash(unsigned) const;
    unsigned GetHash() const;
    void Reset();
    unsigned GetColor() const;
    void SetColor(unsigned);
    void SetDrawLayer(int);
    int GetDrawLayer() const;
    void SetRenderTechnique(const boost::shared_ptr<RenderTechnique>&);
    void SetRenderTechnique(const jet::String&);
    const boost::shared_ptr<RenderTechnique> &GetRenderTechnique() const;
    unsigned GetRenderPassCount() const;
    const jet::String &GetRenderPassName(unsigned) const;
    RenderState &GetRenderState(unsigned);
    const RenderState &GetRenderState(unsigned) const;
    unsigned GetUniformCount(unsigned) const;
    ShaderUniform GetUniform(unsigned, unsigned) const;
    bool SetUniform(unsigned, const jet::String&, const ShaderUniform&);
    void SetUniform(unsigned, unsigned, const ShaderUniform&);
    const jet::String &GetUniformName(unsigned, unsigned) const;
    int FindUniformIdxByName(unsigned, const jet::String&) const;
    const ShaderUniform &GetUniformDef(unsigned, unsigned) const;
    const unsigned char *GetUniformData(unsigned, unsigned) const;
    unsigned GetTextureLayerCount(unsigned) const;
    TextureLayer &GetTextureLayer(unsigned, unsigned);
    const TextureLayer &GetTextureLayer(unsigned, unsigned) const;
    const jet::String &GetTextureLayerName(unsigned, unsigned) const;
    TextureLayer *FindTextureLayerByName(unsigned, const jet::String&);
    const TextureLayer *FindTextureLayerByName(unsigned, const jet::String&) const;
    void SetTextureFiltering(safe_enum<Texture::FilterDef,unsigned char>);
    void SetTextureMipmapping(bool);
    void SetTextureWrapping(safe_enum<Texture::WrapDef,unsigned char>);
    void Load(jet::stream::IStream&);
    unsigned GetUsedAttributes() const;
    unsigned GetAllTagMasks() const;
    unsigned GetTagMask(unsigned) const;
    void SetTagActive(unsigned, unsigned, bool);
    bool IsTagActive(unsigned, unsigned) const;
protected:
    void LoadV100(const pugi::xml_node&);
    void ResetHash();

    struct PassData {
        unsigned tagMask;
        RenderState renderState;
        unsigned uniformDataOffset;
        unsigned uniformDataSize;
        unsigned textureLayerStart;
        unsigned textureLayerCount;
    };

    unsigned m_color;
    int m_drawLayer;
    boost::shared_ptr<RenderTechnique> m_technique;
    std::vector<unsigned char> m_uniformData;
    std::vector<TextureLayer> m_textureLayers;
    std::vector<PassData> m_passes;
    unsigned m_usedAttributes;
    unsigned m_allTagMasks;
    unsigned m_hash;
};
}

#endif
