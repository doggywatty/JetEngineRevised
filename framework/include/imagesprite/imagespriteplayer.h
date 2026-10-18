// framework/include/imagesprite/imagespriteplayer.h
#ifndef FRAMEWORK_INCLUDE_IMAGESPRITE_IMAGESPRITEPLAYER_H
#define FRAMEWORK_INCLUDE_IMAGESPRITE_IMAGESPRITEPLAYER_H

class ImageSpritePlayer {  // sizeof 0x80
public:
    ImageSpritePlayer(const unsigned char*, int, int, int, int, const jet::String&, const jet::String&);
    virtual ~ImageSpritePlayer();
    void Update(int);
    void Render(jet::video::Painter&);
    int GetImageWidth();
    int GetImageHeight();
    void PutIntoRect(const math::rect<float>&);
    const boost::shared_ptr<jet::video::Texture> &GetTexturePtr();
private:
    math::vec2<float> m_quadCoords[4];  // 0x4
    math::vec2<float> m_texCoords[4];  // 0x24
    math::mat3<float> m_transform;  // 0x44
    boost::shared_ptr<jet::video::Texture> m_texture;  // 0x68
    jet::stream::MemoryStream *m_memStream;  // 0x70
    int m_imageWidth;  // 0x74
    int m_imageHeight;  // 0x78
    jet::String m_uniqueImageName;  // 0x7C
    static unsigned s_imageCount;
};

#endif
