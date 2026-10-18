// core/jet/src/video/texturelayer.cpp
#include "video/texturelayer.h"

namespace jet::video {

TextureLayer::TextureLayer()
    : m_hasTransform(false), m_anisotropy(0), m_wrapS(Texture::WrapDef::Repeat),
      m_wrapT(Texture::WrapDef::Repeat), m_filter(Texture::FilterDef::Bilinear), m_useMipmap(true) {
}

TextureLayer::TextureLayer(const TextureLayer &o)
    : m_texture(o.m_texture), m_alphaTexture(o.m_alphaTexture), m_hasTransform(o.m_hasTransform),
      m_anisotropy(o.m_anisotropy), m_wrapS(o.m_wrapS), m_wrapT(o.m_wrapT), m_filter(o.m_filter),
      m_useMipmap(o.m_useMipmap) {
    if (o.m_transform.get()) m_transform.reset(new math::mat4<float>(*o.m_transform));
}

TextureLayer::~TextureLayer() {}

TextureLayer &TextureLayer::operator=(const TextureLayer &o) {
    m_texture = o.m_texture;
    m_alphaTexture = o.m_alphaTexture;
    m_hasTransform = o.m_hasTransform;
    m_transform.reset(o.m_transform.get() ? new math::mat4<float>(*o.m_transform) : 0);
    m_anisotropy = o.m_anisotropy;
    m_wrapS = o.m_wrapS;
    m_wrapT = o.m_wrapT;
    m_filter = o.m_filter;
    m_useMipmap = o.m_useMipmap;
    return *this;
}

bool TextureLayer::operator==(const TextureLayer &o) const {
    return m_texture == o.m_texture && m_alphaTexture == o.m_alphaTexture &&
           m_hasTransform == o.m_hasTransform && m_wrapS == o.m_wrapS && m_wrapT == o.m_wrapT &&
           m_filter == o.m_filter && m_anisotropy == o.m_anisotropy && m_useMipmap == o.m_useMipmap &&
           (!m_hasTransform || *m_transform == *o.m_transform);
}
bool TextureLayer::operator!=(const TextureLayer &o) const { return !(*this == o); }

const boost::shared_ptr<Texture> &TextureLayer::GetTexture() const { return m_texture; }
void TextureLayer::SetTexture(const boost::shared_ptr<Texture> &t) { m_texture = t; }
const boost::shared_ptr<Texture> &TextureLayer::GetAlphaTexture() const { return m_alphaTexture; }
void TextureLayer::SetAlphaTexture(const boost::shared_ptr<Texture> &t) { m_alphaTexture = t; }

bool TextureLayer::HasTransform() { return m_hasTransform; }
const math::mat4<float> &TextureLayer::GetTransform() {
    static math::mat4<float> identity;
    return m_transform.get() ? *m_transform : identity;
}
void TextureLayer::SetTransform(const math::mat4<float> &t) {
    m_hasTransform = true;
    m_transform.reset(new math::mat4<float>(t));
}

safe_enum<Texture::WrapDef,unsigned char> TextureLayer::GetWrapS() { return m_wrapS; }
void TextureLayer::SetWrapS(safe_enum<Texture::WrapDef,unsigned char> v) { m_wrapS = v; }
safe_enum<Texture::WrapDef,unsigned char> TextureLayer::GetWrapT() { return m_wrapT; }
void TextureLayer::SetWrapT(safe_enum<Texture::WrapDef,unsigned char> v) { m_wrapT = v; }
safe_enum<Texture::FilterDef,unsigned char> TextureLayer::GetFilter() { return m_filter; }
void TextureLayer::SetFilter(safe_enum<Texture::FilterDef,unsigned char> v) { m_filter = v; }

unsigned TextureLayer::GetAnisotropy() { return m_anisotropy; }
void TextureLayer::SetAnisotropy(unsigned v) { m_anisotropy = (unsigned short)v; }
bool TextureLayer::GetMipmap() { return m_useMipmap; }
void TextureLayer::SetMipmap(bool v) { m_useMipmap = v; }

}
