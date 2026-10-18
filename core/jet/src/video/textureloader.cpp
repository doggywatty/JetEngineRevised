// core/jet/src/video/textureloader.cpp
#include "video/textureloader.h"
#include "stream/streammgr.h"

namespace jet::video {

TextureLoader *TextureLoader::s_instance = 0;

TextureLoader::TextureLoader() : m_defaultFilter(Texture::FilterDef::Trilinear), m_bestLod(0) {}
TextureLoader::~TextureLoader() { UnloadAll(); }

TextureLoader *TextureLoader::GetInstance() {
    if (!s_instance) s_instance = new TextureLoader();
    return s_instance;
}
bool TextureLoader::HasInstance() { return s_instance != 0; }
void TextureLoader::FreeInstance() { delete s_instance; s_instance = 0; }

// CanLoad @0x6aaa7c: already loaded, or a stream of that name exists
bool TextureLoader::CanLoad(const jet::String &name) {
    if (name.empty()) return false;
    if (m_textures.find(name) != m_textures.end()) return true;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) return false;
    delete s;
    return true;
}

boost::shared_ptr<Texture> TextureLoader::Load(const jet::String &name) { return Load(name, Params()); }

boost::shared_ptr<Texture> TextureLoader::Load(const jet::String &name, const Params &p) {
    std::map<jet::String, boost::shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it != m_textures.end()) return it->second;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) return boost::shared_ptr<Texture>();
    return LoadFromStream(name, s, p);
}

boost::shared_ptr<Texture> TextureLoader::Load(const jet::String &name, jet::stream::IStream *s, const Params &p) {
    std::map<jet::String, boost::shared_ptr<Texture> >::iterator it = m_textures.find(name);
    if (it != m_textures.end()) return it->second;
    return LoadFromStream(name, s, p);
}

// LoadFromStream @0x6ab... : a texture of the driver's kind, initialized from the header, then
// its levels uploaded (skipping the best-lod ones)
boost::shared_ptr<Texture> TextureLoader::LoadFromStream(const jet::String &name, jet::stream::IStream *s, const Params &p) {
    boost::shared_ptr<Texture> t = Texture::New();
    t->SetResourceName(name);
    t->SetLod(p.bestLod > m_bestLod ? p.bestLod : m_bestLod);
    if (!t->Init(s) || !t->Load()) return boost::shared_ptr<Texture>();
    t->SetFiltering(m_defaultFilter);
    m_textures[name] = t;
    return t;
}

void TextureLoader::UnloadAll() { m_textures.clear(); }

void TextureLoader::FreeUnreferenced() {
    for (std::map<jet::String, boost::shared_ptr<Texture> >::iterator it = m_textures.begin(); it != m_textures.end();)
        if (it->second.use_count() == 1) m_textures.erase(it++);
        else ++it;
}

}
