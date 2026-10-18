// core/jet/src/video/texture.cpp
#include "video/texture.h"
#include "video/gles20/gles20texture.h"
#include <algorithm>

namespace jet::video {

static int g_currentFrameIdx = 0;  // bumped by the driver once per frame; wired up with GLES2Driver.

Texture::Texture()
    : m_canBeStreamed(true), m_canBeUnloaded(true), m_hasMipMaps(false), m_isCubeMap(false),
      m_hasAlphaChannel(true), m_useMipMap(true), m_wrapS(WrapDef::Repeat), m_wrapT(WrapDef::Repeat),
      m_filtering(FilterDef::Bilinear), m_format(FormatDef::RGBA8888), m_anisotropy(0),
      m_lastUsedFrameIdx(0), m_crtLod(0), m_minViewDistance(0), m_streamViewDistance(0) {
    SetLoaded(true);
}

Texture::~Texture() {}

boost::shared_ptr<Texture> Texture::New() {
    return boost::shared_ptr<Texture>(new GLES20Texture());
}

safe_enum<Texture::FormatDef,unsigned char> Texture::GetFormat() { return m_format; }
void Texture::SetFormat(safe_enum<FormatDef,unsigned char> f) { m_format = f; }

void Texture::SwapContent(const boost::shared_ptr<Texture> &o) {
    jet::res::Resource::SwapContent(o.get());
    if (!o) return;
    std::swap(m_isCubeMap, o->m_isCubeMap);
    std::swap(m_hasMipMaps, o->m_hasMipMaps);
    std::swap(m_useMipMap, o->m_useMipMap);
    std::swap(m_wrapS, o->m_wrapS);
    std::swap(m_wrapT, o->m_wrapT);
    std::swap(m_filtering, o->m_filtering);
    std::swap(m_hasAlphaChannel, o->m_hasAlphaChannel);
    std::swap(m_canBeUnloaded, o->m_canBeUnloaded);
    std::swap(m_format, o->m_format);
    std::swap(m_anisotropy, o->m_anisotropy);
    std::swap(m_lastUsedFrameIdx, o->m_lastUsedFrameIdx);
    std::swap(m_crtLod, o->m_crtLod);
    std::swap(m_size, o->m_size);
    std::swap(m_lod0Size, o->m_lod0Size);
}

bool Texture::CanBeStreamed() { return m_canBeStreamed; }
void Texture::SetCanBeStreamed(bool v) { m_canBeStreamed = v; }
bool Texture::CanBeUnloaded() { return m_canBeUnloaded; }
void Texture::SetCanBeUnloaded(bool v) { m_canBeUnloaded = v; }
bool Texture::IsCubeMap() { return m_isCubeMap; }
void Texture::SetIsCubeMap(bool v) { m_isCubeMap = v; }
bool Texture::HasAlphaChannel() { return m_hasAlphaChannel; }
void Texture::SetHasAlphaChannel(bool v) { m_hasAlphaChannel = v; }
bool Texture::HasMipMaps() { return m_hasMipMaps; }
void Texture::SetHasMipMaps(bool v) { m_hasMipMaps = v; }
bool Texture::GetMipMap() { return m_useMipMap; }
void Texture::SetMipMap(bool v) { m_useMipMap = v; }

const math::vec2<unsigned int> &Texture::GetLod0Size() { return m_lod0Size; }
void Texture::SetLod0Size(const math::vec2<unsigned int> &s) { m_lod0Size = s; }
const math::vec2<unsigned int> &Texture::GetSize() { return m_size; }
void Texture::SetSize(const math::vec2<unsigned int> &s) { m_size = s; }

float Texture::GetMinViewDistance() { return m_minViewDistance; }
void Texture::SetMinViewDistance(float v) { m_minViewDistance = v; }
float Texture::ComputeStreamViewDistance() { return m_streamViewDistance; }

void Texture::SetWrapS(safe_enum<WrapDef,unsigned char> v) { m_wrapS = v; }
safe_enum<Texture::WrapDef,unsigned char> Texture::GetWrapS() { return m_wrapS; }
void Texture::SetWrapT(safe_enum<WrapDef,unsigned char> v) { m_wrapT = v; }
safe_enum<Texture::WrapDef,unsigned char> Texture::GetWrapT() { return m_wrapT; }
void Texture::SetFiltering(safe_enum<FilterDef,unsigned char> v) { m_filtering = v; }
safe_enum<Texture::FilterDef,unsigned char> Texture::GetFiltering() { return m_filtering; }
void Texture::SetAnisotropy(unsigned v) { m_anisotropy = v; }
unsigned Texture::GetAnisotropy() { return m_anisotropy; }

int Texture::GetLastUsedFrameIdx() { return m_lastUsedFrameIdx; }
void Texture::SetLastUsedFrameIdx(int v) { m_lastUsedFrameIdx = v; }
unsigned Texture::GetLod() { return m_crtLod; }
void Texture::SetLod(unsigned v) { m_crtLod = v; }

// The original resets the streamed-in view distance to "unknown" whenever the texture is used
// in a different frame than last time, so the streaming system re-evaluates it.
void Texture::Bind(int) {
    int last = m_lastUsedFrameIdx;
    m_lastUsedFrameIdx = g_currentFrameIdx;
    if (last != g_currentFrameIdx) m_streamViewDistance = -1.0f;
}

bool Texture::GetData(std::vector<unsigned char>&, unsigned) { return false; }

}
