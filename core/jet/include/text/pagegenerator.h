// core/jet/include/text/pagegenerator.h
#ifndef CORE_JET_INCLUDE_TEXT_PAGEGENERATOR_H
#define CORE_JET_INCLUDE_TEXT_PAGEGENERATOR_H

namespace jet::text {
class PageGenerator {  // sizeof 0xC0
public:
    PageGenerator();
    virtual ~PageGenerator();
    virtual bool Build(const jet::text::PageGenerationParams&, void*, unsigned) = 0;
protected:
    void Blit(int, int, jet::text::PageGenerator::Page*);
public:
    jet::text::PageGenerationParams m_params;  // 0x4
    std::vector<jet::text::Font::GlyphData,std::allocator<jet::text::Font::GlyphData> > m_glyphs;  // 0x28
    std::vector<jet::text::Font::KerningData,std::allocator<jet::text::Font::KerningData> > m_kernings;  // 0x3C
    std::vector<jet::text::PageGenerator::Page *,std::allocator<jet::text::PageGenerator::Page *> > m_pages;  // 0x50
protected:
    jet::text::FreetypeBitmap m_bitmap;  // 0x64
};
}

#endif
