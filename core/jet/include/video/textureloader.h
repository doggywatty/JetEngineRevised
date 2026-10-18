// core/jet/include/video/textureloader.h
#ifndef CORE_JET_INCLUDE_VIDEO_TEXTURELOADER_H
#define CORE_JET_INCLUDE_VIDEO_TEXTURELOADER_H

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "core/pstring.h"
#include "stream/istream.h"
#include "video/texture.h"

namespace jet::video {

// Loads textures by file name through StreamMgr and keeps one instance per name
// (TextureLoader::Load @0x6aabfc). Loading is synchronous here; the original's background
// streaming and memory budget (TextureLoadingJob, SwapStreamed) are not part of this path.
class TextureLoader : private boost::noncopyable {
public:
    struct Params {
        unsigned bestLod;   // largest levels dropped
        Params() : bestLod(0) {}
    };

    static TextureLoader *GetInstance();
    static bool HasInstance();
    static void FreeInstance();

    bool CanLoad(const jet::String&);
    boost::shared_ptr<Texture> Load(const jet::String&);
    boost::shared_ptr<Texture> Load(const jet::String&, const Params&);
    boost::shared_ptr<Texture> Load(const jet::String&, jet::stream::IStream*, const Params&);
    void UnloadAll();
    void FreeUnreferenced();
    void SetDefaultTextureFilter(safe_enum<Texture::FilterDef,unsigned char> f) { m_defaultFilter = f; }
    safe_enum<Texture::FilterDef,unsigned char> GetDefaultTextureFilter() const { return m_defaultFilter; }
    void SetBestLod(unsigned l) { m_bestLod = l; }

private:
    TextureLoader();
    ~TextureLoader();
    boost::shared_ptr<Texture> LoadFromStream(const jet::String&, jet::stream::IStream*, const Params&);

    static TextureLoader *s_instance;
    std::map<jet::String, boost::shared_ptr<Texture> > m_textures;
    safe_enum<Texture::FilterDef,unsigned char> m_defaultFilter;
    unsigned m_bestLod;
};
}

#endif
