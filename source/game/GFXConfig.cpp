// source/game/GFXConfig.cpp
#include "game/GFXConfig.h"
#include "stream/streammgr.h"
#include "stream/istream.h"
#include <cmath>
#include <cstdio>

// @0x50bb94
void GFXConfig::Load(const jet::String &name) {
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) return;
    s->Start();
    unsigned uncompressed = 0, count = 0;
    *s >> uncompressed;
    m_isUsingUncompressedTextures = uncompressed != 0;
    fprintf(stderr, "m_isUsingUncompressedTextures = %d\n", m_isUsingUncompressedTextures);
    *s >> count;
    fprintf(stderr, "m_gfxConfigDataCount = %d\n", count);
    m_gfxConfigData.assign(count, GFXConfigStruct());
    for (unsigned i = 0; i < count; i++) {
        GFXConfigStruct &c = m_gfxConfigData[i];
        unsigned n = 0;
        *s >> c.screenWidth >> c.screenHeight >> c.designWidth >> c.designHeight;
        *s >> c.guiLayers >> c.guiNames >> c.sprites >> c.spritePrefix >> c.fonts;
        *s >> n;
        c.suffixes.clear();
        for (unsigned k = 0; k < n; k++) {
            jet::String suffix;
            *s >> suffix;
            c.suffixes.push_back(suffix);
        }
    }
    s->Finish();
    delete s;
}

// @0x50be44
const GFXConfigStruct *GFXConfig::GetConfig(unsigned w, unsigned h) const {
    for (const GFXConfigStruct &c : m_gfxConfigData)
        if (c.screenWidth == w && c.screenHeight == h) return &c;
    for (const GFXConfigStruct &c : m_gfxConfigData)
        if (fabsf((float)w / (float)c.screenWidth - (float)h / (float)c.screenHeight) < 1.1920929e-7f) return &c;
    fprintf(stderr, "*** Error: GetConfig algorithm failed for scrW = %u scrH = %u\n", w, h);
    return m_gfxConfigData.empty() ? 0 : &m_gfxConfigData[0];
}
