// core/jet/include/text/font.h
#ifndef CORE_JET_INCLUDE_TEXT_FONT_H
#define CORE_JET_INCLUDE_TEXT_FONT_H

namespace jet::text2 {
class Font : public jet::res::Resource {  // sizeof 0xBC
public:
    Font();
    virtual ~Font();
    math::vec2<unsigned int> GetPageSize();
    unsigned GetPageCount();
    boost::shared_ptr<jet::video::Texture> GetPage(unsigned);
    float GetSpaceWidth(unsigned);
    float GetCenterHeight(unsigned);
    float GetAscender(unsigned);
    float GetDescender(unsigned);
    float GetLineGap(unsigned);
    void GetGlyphs(std::vector<jet::text2::Font::Glyph,std::allocator<jet::text2::Font::Glyph> >&, std::vector<boost::shared_ptr<jet::video::Texture>,std::allocator<boost::shared_ptr<jet::video::Texture> > >&, unsigned, const char*);
    float GetKerning(unsigned, const jet::text2::Font::Glyph&, const jet::text2::Font::Glyph&);
    virtual void Unload();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
private:
    void FreeData();
    bool SetHeight(unsigned);
    jet::text2::Font::HeightData &GetHeightData(unsigned);
    void CreateAtlas();
    bool CreatePageFromAtlas();
    bool RenderGlyph(jet::text2::Font::Glyph&, unsigned);
    void ConvertToDistanceMap(const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, const unsigned char*, unsigned);
    jet::text2::Font::FreeTypeData mFreeTypeData;  // 0x10
    unsigned char* mData;  // 0x18
    unsigned mSize;  // 0x1C
    jet::thread::Mutex mRenderMutex;  // 0x20
    boost::unordered::unordered_map<unsigned __int64,float,boost::hash<unsigned __int64>,std::equal_to<unsigned __int64>,std::allocator<std::pair<unsigned __int64 const ,float> > > mKernings;  // 0x3C
    unsigned mCrtHeight;  // 0x58
    boost::unordered::unordered_map<unsigned int,jet::text2::Font::HeightData,boost::hash<unsigned int>,std::equal_to<unsigned int>,std::allocator<std::pair<unsigned int const ,jet::text2::Font::HeightData> > > mFontHeightData;  // 0x5C
    std::vector<boost::shared_ptr<jet::video::Texture>,std::allocator<boost::shared_ptr<jet::video::Texture> > > mPages;  // 0x78
    jet::video::Atlaser mAtlas;  // 0x8C
    std::vector<unsigned char,std::allocator<unsigned char> > mImageData;  // 0xA8
};
}

namespace jet::text {
class Font : public jet::res::Resource {  // sizeof 0x7C
protected:
    Font(safe_enum<jet::text::Font::TypeDef,enum jet::text::Font::TypeDef::type>);
public:
    virtual ~Font();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
    virtual void Unload();
    unsigned GetHeight();
    safe_enum<jet::text::Font::TypeDef,enum jet::text::Font::TypeDef::type> GetType();
    bool Prepare(const char*, unsigned);
protected:
    jet::text::Font::FreetypeData GetFreetypeData();
    bool LoadBMFont();
    bool LoadAuroraFont();
    bool LoadFreetypeFont();
    bool IsValidForRendering(const jet::text::Font::GlyphData*);
    const jet::text::Font::GlyphData *FindGlyphDataByCodepoint(unsigned);
    int FindKerningDataByCodepoints(unsigned, unsigned);
    unsigned GetCharWidthA(unsigned);
    bool NeedsTextures(const unsigned*, unsigned);
    void GenerateTextures(const unsigned*, unsigned);
    void UnloadPages();
    boost::shared_ptr<jet::text::Font::ComputeRealHeightTask> m_computeRealHeightTask;  // 0x10
private:
    void* m_mappedData;  // 0x18
    unsigned m_mappedSize;  // 0x1C
    safe_enum<jet::text::Font::TypeDef,enum jet::text::Font::TypeDef::type> m_type;  // 0x20
    std::vector<jet::text::Font::GlyphData,std::allocator<jet::text::Font::GlyphData> > m_glyphs;  // 0x24
    std::vector<jet::text::Font::KerningData,std::allocator<jet::text::Font::KerningData> > m_kernings;  // 0x38
    std::vector<boost::shared_ptr<jet::video::Texture>,std::allocator<boost::shared_ptr<jet::video::Texture> > > m_pages;  // 0x4C
    unsigned m_height;  // 0x60
    jet::text::Font::FreetypeData m_freetype;  // 0x64
    jet::text::Font::AuroraData m_aurora;  // 0x78
};
}

#endif
