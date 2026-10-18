// core/jet/include/text2/font2.h
#ifndef CORE_JET_INCLUDE_TEXT2_FONT2_H
#define CORE_JET_INCLUDE_TEXT2_FONT2_H

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "res/resource.h"
#include "video/texture.h"
#include "video/atlaser.h"

typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_ *FT_Face;

namespace jet::text2 {
// TrueType font rendered with FreeType into 256x256 alpha pages, one page set shared by every
// pixel height. Glyph metrics are in pixels at the requested height.
class Font : public jet::res::Resource {  // sizeof 0xBC
public:
    struct Glyph {
        unsigned codepoint;      // 0x0
        unsigned glyphIndex;     // 0x4
        float advance;           // 0x8
        float width;             // 0xc
        float height;            // 0x10
        unsigned short x, y;     // 0x14 position in the page (inside a 1 pixel border)
        unsigned short w, h;     // 0x18 bitmap size
        short left, top;         // 0x1c bitmap offset from the pen (top is positive down)
        unsigned short page;     // 0x20 0xffff when not cached
        Glyph();
    };
    struct GlyphCachedData { unsigned codepoint, height; };
    struct PageData { boost::shared_ptr<jet::video::Texture> texture; };

    Font();
    virtual ~Font();
    math::vec2<unsigned int> GetPageSize() const;
    unsigned GetPageCount() const;
    boost::shared_ptr<jet::video::Texture> GetPage(unsigned) const;
    float GetSpaceWidth(unsigned) const;
    float GetCenterHeight(unsigned) const;
    float GetAscender(unsigned) const;
    float GetDescender(unsigned) const;
    float GetLineGap(unsigned) const;
    // metrics only, scaled from the font's em square
    void GetGlyphs(std::vector<Glyph>&, unsigned height, const char *utf8) const;
    // rendered glyphs plus the page textures they reference
    void GetGlyphs(std::vector<Glyph>&, std::vector<boost::shared_ptr<jet::video::Texture> >&, unsigned height, const char *utf8) const;
    float GetKerning(unsigned height, const Glyph&, const Glyph&) const;
    virtual void Unload();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();

private:
    struct HeightData {
        float ascender, descender, height, lineGap, centerHeight, spaceWidth;
        std::unordered_map<unsigned, unsigned> index;   // codepoint -> glyphs[]
        std::vector<Glyph> glyphs;
    };
    void FreeData();
    bool SetHeight(unsigned) const;
    HeightData &GetHeightData(unsigned) const;
    void ResetAtlas() const;
    bool UploadPage() const;
    bool RenderGlyph(Glyph&, unsigned codepoint, bool cache) const;
    bool ComputeGlyphMetrics(Glyph&, unsigned codepoint) const;
    bool CacheGlyphs(unsigned height, const std::vector<GlyphCachedData>&) const;
    void CollectAllGlyphsToCache(std::vector<GlyphCachedData>&) const;
    void Decode(const char *utf8, std::vector<unsigned>&) const;

    FT_Library mLibrary;                                   // 0x10
    FT_Face mFace;                                         // 0x14
    unsigned mSize;                                        // 0x1c
    mutable std::unordered_map<unsigned long long, float> mKernings;   // 0x24 (a codepoint, b codepoint) -> em fraction
    mutable std::unordered_map<unsigned, Glyph> mGlyphMetrics;         // 0x3c em-normalized metrics
    mutable unsigned mCrtHeight;                           // 0x54
    mutable std::unordered_map<unsigned, HeightData> mFontHeightData;  // 0x58
    mutable std::vector<PageData> mPages;                  // 0x70
    mutable bool mAtlasClean;                              // 0x7c
    mutable jet::video::Atlaser mAtlas;                    // 0x80
    mutable std::vector<unsigned char> mImageData;         // 0x94
};
}

#endif
