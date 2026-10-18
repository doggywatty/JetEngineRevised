// core/jet/include/video/textureloadingjob.h
#ifndef CORE_JET_INCLUDE_VIDEO_TEXTURELOADINGJOB_H
#define CORE_JET_INCLUDE_VIDEO_TEXTURELOADINGJOB_H

namespace jet::video {
class TextureLoadingJob : public jet::thread::Task {  // sizeof 0x24
public:
    TextureLoadingJob(const boost::shared_ptr<jet::video::TextureLoader::TextureData>&, const jet::String&, int);
    virtual ~TextureLoadingJob();
    virtual bool Run();
    const boost::shared_ptr<jet::video::TextureLoader::TextureData> &GetOriginalTextureData();
    const boost::shared_ptr<jet::video::Texture> &GetNewTexture();
private:
    boost::shared_ptr<jet::video::TextureLoader::TextureData> m_originalTextureData;  // 0xC
    jet::String m_texName;  // 0x14
    int m_targetLOD;  // 0x18
    boost::shared_ptr<jet::video::Texture> m_texture;  // 0x1C
};
}

#endif
