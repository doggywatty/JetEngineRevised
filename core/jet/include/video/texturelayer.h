// core/jet/include/video/texturelayer.h
#ifndef CORE_JET_INCLUDE_VIDEO_TEXTURELAYER_H
#define CORE_JET_INCLUDE_VIDEO_TEXTURELAYER_H

#include <memory>
#include <boost/shared_ptr.hpp>
#include "jenum.h"
#include "smath.h"
#include "video/texture.h"

namespace jet::video {
class TextureLayer {  // sizeof 0x1C
public:
    TextureLayer(const jet::video::TextureLayer&);
    TextureLayer();
    TextureLayer &operator=(const TextureLayer&);
    ~TextureLayer();
    bool operator==(const jet::video::TextureLayer&) const;
    bool operator!=(const jet::video::TextureLayer&) const;
    const boost::shared_ptr<jet::video::Texture> &GetTexture() const;
    void SetTexture(const boost::shared_ptr<jet::video::Texture>&);
    const boost::shared_ptr<jet::video::Texture> &GetAlphaTexture() const;
    void SetAlphaTexture(const boost::shared_ptr<jet::video::Texture>&);
    bool HasTransform();
    const math::mat4<float> &GetTransform();
    void SetTransform(const math::mat4<float>&);
    void ClearTransform() { m_hasTransform = false; }
    safe_enum<jet::video::Texture::WrapDef,unsigned char> GetWrapS();
    void SetWrapS(safe_enum<jet::video::Texture::WrapDef,unsigned char>);
    safe_enum<jet::video::Texture::WrapDef,unsigned char> GetWrapT();
    void SetWrapT(safe_enum<jet::video::Texture::WrapDef,unsigned char>);
    safe_enum<jet::video::Texture::FilterDef,unsigned char> GetFilter();
    void SetFilter(safe_enum<jet::video::Texture::FilterDef,unsigned char>);
    unsigned GetAnisotropy();
    void SetAnisotropy(unsigned);
    bool GetMipmap();
    void SetMipmap(bool);
protected:
    boost::shared_ptr<jet::video::Texture> m_texture;  // 0x0
    boost::shared_ptr<jet::video::Texture> m_alphaTexture;  // 0x8
    std::unique_ptr<math::mat4<float> > m_transform;  // 0x10
    bool m_hasTransform;  // 0x14
    unsigned short m_anisotropy;  // 0x16
    safe_enum<jet::video::Texture::WrapDef,unsigned char> m_wrapS;  // 0x18
    safe_enum<jet::video::Texture::WrapDef,unsigned char> m_wrapT;  // 0x19
    safe_enum<jet::video::Texture::FilterDef,unsigned char> m_filter;  // 0x1A
    bool m_useMipmap;  // 0x1B
};
}

#endif
