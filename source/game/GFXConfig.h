// source/game/GFXConfig.h
#ifndef SOURCE_GAME_GFXCONFIG_H
#define SOURCE_GAME_GFXCONFIG_H

#include <vector>
#include "core/pstring.h"

// one display size the GUI was authored for and the datalib folders to use on it (gfx_config)
struct GFXConfigStruct {  // sizeof 0x30
    unsigned screenWidth, screenHeight;       // 0x0 the display size this entry is for
    unsigned designWidth, designHeight;       // 0x8 the size the GUI layers were authored at
    jet::String guiLayers;                    // 0x10 gui datalib
    jet::String guiNames;                     // 0x14 its element names file
    jet::String sprites;                      // 0x18 sprites datalib
    jet::String spritePrefix;                 // 0x1c
    jet::String fonts;                        // 0x20 fonts datalib
    std::vector<jet::String> suffixes;        // 0x24 device specific layer suffixes
};

class GFXConfig {  // sizeof 0x10
public:
    GFXConfig() : m_isUsingUncompressedTextures(false) {}
    void Load(const jet::String&);
    const GFXConfigStruct &GetConfig(unsigned i) const { return m_gfxConfigData[i]; }
    // the entry for exactly this display size, else for the same aspect ratio
    const GFXConfigStruct *GetConfig(unsigned width, unsigned height) const;
    bool IsUsingUncompressedTextures() const { return m_isUsingUncompressedTextures; }
    unsigned GetDataCount() const { return (unsigned)m_gfxConfigData.size(); }
private:
    std::vector<GFXConfigStruct> m_gfxConfigData;  // 0x0
    bool m_isUsingUncompressedTextures;            // 0xc
};

#endif
