// core/jet/include/video/texturecomposer.h
#ifndef CORE_JET_INCLUDE_VIDEO_TEXTURECOMPOSER_H
#define CORE_JET_INCLUDE_VIDEO_TEXTURECOMPOSER_H

namespace jet::video {
class TextureComposer {  // sizeof 0x20
public:
    TextureComposer();
    bool Create(unsigned, unsigned);
    bool CopyRect(const boost::shared_ptr<jet::video::Texture>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&);
    boost::shared_ptr<jet::video::Texture> GenerateTexture();
protected:
    bool CopyBlockCompressed(const boost::shared_ptr<jet::video::Texture>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&);
    bool CopyUncompressed(const boost::shared_ptr<jet::video::Texture>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, const math::vec2<unsigned int>&);
private:
    unsigned m_width;  // 0x0
    unsigned m_height;  // 0x4
    safe_enum<jet::video::Texture::FormatDef,unsigned char> m_format;  // 0x8
    std::vector<unsigned char,std::allocator<unsigned char> > m_data;  // 0xC
};
}

#endif
