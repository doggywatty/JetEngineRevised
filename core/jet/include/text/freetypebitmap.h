// core/jet/include/text/freetypebitmap.h
#ifndef CORE_JET_INCLUDE_TEXT_FREETYPEBITMAP_H
#define CORE_JET_INCLUDE_TEXT_FREETYPEBITMAP_H

namespace jet::text {
class FreetypeBitmap {  // sizeof 0x5C
public:
    FreetypeBitmap();
    ~FreetypeBitmap();
    bool LoadFont(const jet::String&, void*, unsigned);
    bool SupportsKerning();
    bool SetSize(unsigned);
    unsigned GetSize();
    unsigned GetHeight();
    unsigned ComputeRealHeight();
    bool Load(unsigned);
    void Render();
    int GetHorizontalKerningOffset(unsigned);
    jet::text::FreetypeBitmap::GlyphData m_data;  // 0x0
    std::vector<unsigned char,std::allocator<unsigned char> > m_image;  // 0x38
private:
    jet::String m_facename;  // 0x4C
    unsigned m_size;  // 0x50
    jet::text::FreetypeBitmap::FreetypeParams m_freetype;  // 0x54
};
}

#endif
