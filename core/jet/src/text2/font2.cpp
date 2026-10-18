// core/jet/src/text2/font2.cpp
#include "text2/font2.h"
#include "text2/utf8/utf8/unchecked.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cstdio>
#include <cstring>

namespace jet::text2 {

static const unsigned k_pageSize = 256;
static const float k_26_6 = 1.0f / 64.0f;
static const unsigned k_maxPages = 33;

Font::Glyph::Glyph() : codepoint(0), glyphIndex(0), advance(0), width(0), height(0), x(0), y(0), w(0), h(0), left(0), top(0), page(0) {
    width = height = 0; x = y = w = h = 0; left = top = 0;
}

// @0x6af538
Font::Font() : mLibrary(0), mFace(0), mSize(0), mCrtHeight(0), mAtlasClean(true), mAtlas(math::vec2<unsigned int>(k_pageSize, k_pageSize)) {}

Font::~Font() { FreeData(); }

// @0x6af77c
void Font::FreeData() {
    if (!mLibrary) return;
    FT_Done_FreeType(mLibrary);
    mLibrary = 0;
    mFace = 0;
    if (jet::stream::IStream *st = GetStream()) {
        st->Unmap();
        st->Finish();
    }
}

void Font::Unload() {}

// @0x6af7d0
bool Font::Init(jet::stream::IStream *s) {
    if (!s) return false;
    SetStream(s);
    return true;
}

// @0x6af7e4: the stream is mapped for the lifetime of the face
bool Font::Load() {
    Unload();
    jet::stream::IStream *st = GetStream();
    if (!st || mLibrary) return false;
    if (FT_Init_FreeType(&mLibrary)) return false;
    st->Start();
    bool started = st->IsStarted();
    if (!started) return false;
    mSize = st->Size();
    if (!mSize) { st->Finish(); return false; }
    void *data = st->Map(mSize);
    if (!data) { st->Finish(); return false; }
    if (FT_New_Memory_Face(mLibrary, (const FT_Byte*)data, (FT_Long)mSize, 0, &mFace)) {
        st->Unmap();
        st->Finish();
        return false;
    }
    SetLoaded(true);
    return true;
}

// @0x6af9d0: FT_Set_Char_Size at 72 dpi, i.e. one point per pixel
bool Font::SetHeight(unsigned h) const {
    if (mCrtHeight == h) return true;
    mCrtHeight = h;
    FT_Error err = FT_Set_Char_Size(mFace, (FT_F26Dot6)(h << 6), 0, 0x48, 0x48);
    if (err) {
        fprintf(stderr, "FT_Error code 0x%02x\n", err);
        return false;
    }
    return true;
}

// @0x6afa54
void Font::ResetAtlas() const {
    std::fill(mImageData.begin(), mImageData.end(), 0);
    mAtlas.Reset();
    mAtlasClean = true;
}

// @0x6afa88: the page being filled is always the last one; this publishes its current image
bool Font::UploadPage() const {
    boost::shared_ptr<jet::video::Texture> tex = jet::video::Texture::New();
    jet::String name = jet::String::Format("page_%x", (unsigned)(uintptr_t)this);
    if (!tex->Create(name, jet::String("8"), math::vec2<unsigned int>(k_pageSize, k_pageSize))) return false;
    tex->UploadData(0, mImageData.data());
    if (mPages.empty()) {
        PageData pd; pd.texture = tex;
        mPages.push_back(pd);
    } else
        mPages.back().texture = tex;
    return true;
}

// @0x6afbe0: renders a glyph with FreeType; with cache set it is packed into the current page
bool Font::RenderGlyph(Glyph &g, unsigned codepoint, bool cache) const {
    FT_UInt idx = FT_Get_Char_Index(mFace, codepoint);
    FT_Error err = FT_Load_Glyph(mFace, idx, FT_LOAD_NO_HINTING | FT_LOAD_RENDER);
    if (err) {
        fprintf(stderr, "FT_Error code 0x%02x\n", err);
        return false;
    }
    FT_GlyphSlot slot = mFace->glyph;
    unsigned width = slot->bitmap.width, rows = slot->bitmap.rows;
    int pitch = slot->bitmap.pitch;
    if (width > 255 || rows > 255) return false;
    unsigned px = 0, py = 0;
    if (cache) {
        math::vec2<unsigned int> pos(0, 0), size(width + 2, rows + 2);
        if (!mAtlas.GetRegion(pos, size)) {
            if (!UploadPage()) return false;
            mPages.push_back(PageData());
            ResetAtlas();
            if (!mAtlas.GetRegion(pos, size)) return false;
        }
        mAtlasClean = false;
        px = pos.x + 1; py = pos.y + 1;
        if (rows && width) {
            const unsigned char *src = slot->bitmap.buffer;
            for (unsigned r = 0; r < rows; r++, src += pitch)
                memcpy(&mImageData[px + (py + r) * k_pageSize], src, width);
        }
    }
    g.codepoint = codepoint;
    g.glyphIndex = idx;
    g.x = (unsigned short)px; g.y = (unsigned short)py;
    g.w = (unsigned short)width; g.h = (unsigned short)rows;
    g.left = (short)slot->bitmap_left;
    g.top = (short)-slot->bitmap_top;
    g.width = (float)slot->metrics.width * k_26_6;
    g.height = (float)slot->metrics.height * k_26_6;
    g.advance = (float)slot->advance.x * k_26_6;
    g.page = cache ? (mPages.empty() ? 0 : (unsigned short)(mPages.size() - 1)) : 0xffff;
    return true;
}

// @0x6afea0: unscaled metrics normalized by the em square
bool Font::ComputeGlyphMetrics(Glyph &g, unsigned codepoint) const {
    FT_UInt idx = FT_Get_Char_Index(mFace, codepoint);
    FT_Error err = FT_Load_Glyph(mFace, idx, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING);
    if (err) {
        fprintf(stderr, "FT_Error code 0x%02x\n", err);
        return false;
    }
    FT_GlyphSlot slot = mFace->glyph;
    float em = (float)mFace->units_per_EM;
    g.codepoint = codepoint;
    g.glyphIndex = idx;
    g.width = (float)slot->metrics.width / em;
    g.height = (float)slot->metrics.height / em;
    g.page = 0xffff;
    g.advance = (float)slot->advance.x / em;
    return true;
}

// @0x6b0314
Font::HeightData &Font::GetHeightData(unsigned h) const {
    auto it = mFontHeightData.find(h);
    if (it != mFontHeightData.end()) return it->second;
    SetHeight(h);
    const FT_Size_Metrics &m = mFace->size->metrics;
    HeightData &d = mFontHeightData[h];
    d.ascender = (float)m.ascender * k_26_6;
    d.descender = (float)-m.descender * k_26_6;
    d.height = (float)m.height * k_26_6;
    d.lineGap = d.descender + d.ascender;
    Glyph g;
    RenderGlyph(g, 'M', false);
    d.centerHeight = g.height * 0.5f;
    Glyph sp;
    RenderGlyph(sp, ' ', false);
    d.spaceWidth = sp.advance;
    return d;
}

math::vec2<unsigned int> Font::GetPageSize() const { return math::vec2<unsigned int>(k_pageSize, k_pageSize); }
unsigned Font::GetPageCount() const { return (unsigned)mPages.size(); }
boost::shared_ptr<jet::video::Texture> Font::GetPage(unsigned i) const { return mPages[i].texture; }
float Font::GetAscender(unsigned h) const { return GetHeightData(h).ascender; }
float Font::GetDescender(unsigned h) const { return GetHeightData(h).descender; }
float Font::GetLineGap(unsigned h) const { return GetHeightData(h).lineGap; }
float Font::GetCenterHeight(unsigned h) const { return GetHeightData(h).centerHeight; }
float Font::GetSpaceWidth(unsigned h) const { return GetHeightData(h).spaceWidth; }

// @0x6af8ec: kerning is cached as a fraction of the em square
float Font::GetKerning(unsigned height, const Glyph &a, const Glyph &b) const {
    unsigned long long key = ((unsigned long long)b.codepoint << 32) | a.codepoint;
    auto it = mKernings.find(key);
    if (it != mKernings.end()) return it->second * (float)height;
    if (!mLibrary) return 0.0f;
    FT_Vector v;
    float k = 0.0f;
    if (!FT_Get_Kerning(mFace, a.glyphIndex, b.glyphIndex, FT_KERNING_UNSCALED, &v))
        k = (float)v.x / (float)mFace->units_per_EM;
    mKernings[key] = k;
    return k * (float)height;
}

void Font::CollectAllGlyphsToCache(std::vector<GlyphCachedData> &out) const {
    for (auto &h : mFontHeightData)
        for (auto &g : h.second.glyphs) {
            GlyphCachedData c = { g.codepoint, h.first };
            out.push_back(c);
        }
}

void Font::Decode(const char *s, std::vector<unsigned> &out) const {
    out.clear();
    if (!s) return;
    const char *end = s + strlen(s);
    while (s < end) out.push_back(utf8::unchecked::next(s));
}

// @0x6b01f0
bool Font::CacheGlyphs(unsigned height, const std::vector<GlyphCachedData> &list) const {
    if (!SetHeight(height)) return false;
    if (mImageData.empty()) mImageData.assign(k_pageSize * k_pageSize, 0);
    GetHeightData(height);
    for (size_t i = 0; i < list.size(); i++) {
        SetHeight(list[i].height);
        HeightData &d = GetHeightData(list[i].height);
        Glyph g;
        if (!RenderGlyph(g, list[i].codepoint, true)) {
            mAtlas.Reset();
            return false;
        }
        d.glyphs[d.index[list[i].codepoint]] = g;
    }
    if (mAtlasClean) return true;
    return UploadPage();
}

// @0x6b000c
void Font::GetGlyphs(std::vector<Glyph> &out, unsigned height, const char *utf8) const {
    out.clear();
    std::vector<unsigned> cps;
    Decode(utf8, cps);
    if (cps.empty()) return;
    out.reserve(cps.size());
    SetHeight(16);
    for (unsigned cp : cps) {
        auto it = mGlyphMetrics.find(cp);
        if (it == mGlyphMetrics.end()) {
            Glyph g;
            if (!ComputeGlyphMetrics(g, cp)) return;
            it = mGlyphMetrics.insert(std::make_pair(cp, g)).first;
        }
        Glyph g = it->second;
        g.advance *= (float)height;
        g.width *= (float)height;
        g.height *= (float)height;
        out.push_back(g);
    }
}

// @0x6b0858
void Font::GetGlyphs(std::vector<Glyph> &out, std::vector<boost::shared_ptr<jet::video::Texture> > &pages, unsigned height, const char *utf8) const {
    out.clear();
    pages.clear();
    std::vector<unsigned> cps;
    Decode(utf8, cps);
    if (cps.empty()) return;
    for (const PageData &p : mPages) pages.push_back(p.texture);
    out.reserve(cps.size());
    HeightData &hd = GetHeightData(height);
    std::vector<unsigned> missing;
    for (unsigned cp : cps) {
        auto it = hd.index.find(cp);
        if (it != hd.index.end()) { out.push_back(hd.glyphs[it->second]); continue; }
        missing.push_back(cp);
        Glyph g;
        g.page = 0xffff;
        hd.index[cp] = (unsigned)hd.glyphs.size();
        hd.glyphs.push_back(g);
    }
    if (missing.empty()) return;
    out.clear();
    pages.clear();
    std::vector<GlyphCachedData> toCache;
    if (mPages.size() >= k_maxPages) {
        // too many pages: start over with just the glyphs this string needs
        mPages.clear();
        ResetAtlas();
        for (auto &h : mFontHeightData) { h.second.glyphs.clear(); h.second.index.clear(); }
        for (unsigned cp : cps) {
            Glyph g;
            g.page = 0xffff;
            if (hd.index.find(cp) == hd.index.end()) {
                hd.index[cp] = (unsigned)hd.glyphs.size();
                hd.glyphs.push_back(g);
            }
            GlyphCachedData c = { cp, height };
            toCache.push_back(c);
        }
    } else {
        for (unsigned cp : missing) {
            GlyphCachedData c = { cp, height };
            toCache.push_back(c);
        }
    }
    if (!CacheGlyphs(height, toCache)) {
        fprintf(stderr, "Cannot cache glyphs for %d codepoints.\n", (int)cps.size());
        return;
    }
    pages.clear();
    for (const PageData &p : mPages) pages.push_back(p.texture);
    for (unsigned cp : cps) out.push_back(hd.glyphs[hd.index[cp]]);
}

}
