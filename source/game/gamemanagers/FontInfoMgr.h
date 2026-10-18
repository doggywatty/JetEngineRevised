// source/game/gamemanagers/FontInfoMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_FONTINFOMGR_H
#define SOURCE_GAME_GAMEMANAGERS_FONTINFOMGR_H

#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "localization/lang.h"

// How a font is made for one language: the file, its height and whether it reads right to left
struct SLanguageFontInfo {  // sizeof 0xC
    SLanguageFontInfo() : size(0), rightToLeft(0) {}
    jet::String file;  // 0x0
    int size;  // 0x4
    int rightToLeft;  // 0x8
};

// One named font of the GUI (index = the font id GUI texts refer to) and its per language variants
struct SFontInfo {  // sizeof 0x28
    jet::String name;  // 0x0
    SLanguageFontInfo fallback;  // 0x4: used for languages without an entry
    std::map<jet::String, SLanguageFontInfo> languages;  // 0x10
};

// The GUI fonts from the "gui_fonts_info" datalib
class FontInfoMgr : public Singleton<FontInfoMgr> {  // sizeof 0x4
public:
    FontInfoMgr();
    virtual ~FontInfoMgr();
    void Load(const jet::String &path);
    bool CanLoadFontsFor(babel::Lang);
    void LoadFontsFor(babel::Lang);
    static const jet::String &GetFontName(unsigned i);
    static const jet::String k_debugFontName;
private:
    static const SLanguageFontInfo &Variant(const SFontInfo&, const jet::String &code);
    static std::vector<SFontInfo> m_auroraFontsInfo;
};

#endif
