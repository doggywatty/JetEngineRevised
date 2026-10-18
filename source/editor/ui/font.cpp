// source/editor/ui/font.cpp
// The editor's font: FreeType (libs/freetype2, the engine's copy) rasterises printable ASCII into one alpha atlas.
#include "editor/ui/ui.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cstring>

namespace jeteditor::ui {

namespace {

constexpr int kFirstGlyph = 32;
constexpr int kLastGlyph = 126;
constexpr int kGlyphCount = kLastGlyph - kFirstGlyph + 1;

}  // namespace

bool Font::Load(const std::string &path, int pixelHeight, std::string &error) {
    FT_Library lib = nullptr;
    if (FT_Init_FreeType(&lib) != 0) {
        error = "FreeType does not start";
        return false;
    }
    FT_Face face = nullptr;
    if (FT_New_Face(lib, path.c_str(), 0, &face) != 0) {
        error = "cannot open font " + path;
        FT_Done_FreeType(lib);
        return false;
    }
    if (FT_Set_Pixel_Sizes(face, 0, pixelHeight) != 0) {
        error = "font " + path + " has no " + std::to_string(pixelHeight) + " px size";
        FT_Done_Face(face);
        FT_Done_FreeType(lib);
        return false;
    }

    const float atlasF = static_cast<float>(kAtlasSize);
    atlas_.assign(static_cast<size_t>(kAtlasSize) * kAtlasSize, 0);
    // A 2x2 white block at the origin, for filled rectangles
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x) atlas_[static_cast<size_t>(y) * kAtlasSize + x] = 255;
    solid_.u0 = 0.5f / atlasF;
    solid_.v0 = 0.5f / atlasF;
    solid_.u1 = 1.5f / atlasF;
    solid_.v1 = 1.5f / atlasF;

    int penX = 2, penY = 0, shelf = 2;
    bool ok = true;
    for (int i = 0; i < kGlyphCount && ok; ++i) {
        if (FT_Load_Char(face, static_cast<FT_ULong>(kFirstGlyph + i), FT_LOAD_RENDER) != 0) {
            error = "font has no glyph for " + std::to_string(kFirstGlyph + i);
            ok = false;
            break;
        }
        const FT_GlyphSlot slot = face->glyph;
        const FT_Bitmap &bmp = slot->bitmap;
        const int w = static_cast<int>(bmp.width), h = static_cast<int>(bmp.rows);
        if (penX + w + 1 > kAtlasSize) {
            penX = 0;
            penY += shelf + 1;
            shelf = 0;
        }
        if (penY + h > kAtlasSize) {
            error = "glyphs do not fit a " + std::to_string(kAtlasSize) + " px atlas at " +
                    std::to_string(pixelHeight) + " px";
            ok = false;
            break;
        }
        for (int row = 0; row < h; ++row) {
            const unsigned char *src = bmp.buffer + static_cast<ptrdiff_t>(row) * bmp.pitch;
            uint8_t *dst = &atlas_[static_cast<size_t>(penY + row) * kAtlasSize + penX];
            std::memcpy(dst, src, static_cast<size_t>(w));
        }
        Glyph &g = glyphs_[i];
        g.u0 = penX / atlasF;
        g.v0 = penY / atlasF;
        g.u1 = (penX + w) / atlasF;
        g.v1 = (penY + h) / atlasF;
        g.w = static_cast<float>(w);
        g.h = static_cast<float>(h);
        g.left = static_cast<float>(slot->bitmap_left);
        g.top = static_cast<float>(slot->bitmap_top);
        g.advance = static_cast<float>(slot->advance.x) / 64.0f;
        penX += w + 1;
        shelf = std::max(shelf, h);
    }

    if (ok) {
        ascent_ = static_cast<float>(face->size->metrics.ascender) / 64.0f;
        lineHeight_ = static_cast<float>(face->size->metrics.height) / 64.0f;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(lib);
    return ok;
}

const Glyph &Font::Get(unsigned char c) const {
    if (c < kFirstGlyph || c > kLastGlyph) c = '?';
    return glyphs_[c - kFirstGlyph];
}

float Font::Width(const std::string &text) const {
    float w = 0;
    for (unsigned char c : text) w += Get(c).advance;
    return w;
}

}  // namespace jeteditor::ui
