// source/game/gamemanagers/FontInfoMgr.cpp
#include "FontInfoMgr.h"
#include "sprites/fontmgr.h"
#include "sprites/sprite.h"
#include "stream/istream.h"
#include "stream/streammgr.h"
#include "PainterMgr.h"
#include <cstdio>
#include <memory>

std::vector<SFontInfo> FontInfoMgr::m_auroraFontsInfo;
const jet::String FontInfoMgr::k_debugFontName("PubHAN");

FontInfoMgr::FontInfoMgr() {}
FontInfoMgr::~FontInfoMgr() {}

// @0x485958: count x {name, fallback {file, size, rtl}, count x {file, size, rtl, count x language code}}
void FontInfoMgr::Load(const jet::String &path) {
    m_auroraFontsInfo.clear();
    std::unique_ptr<jet::stream::IStream> s(jet::stream::StreamMgr::GetInstance().CreateStream(path));
    if (!s) return;
    s->Start();
    unsigned count = 0;
    *s >> count;
    fprintf(stderr, "m_FontInfoDataCount = %d\n", count);
    for (unsigned i = 0; i < count; i++) {
        SFontInfo f;
        *s >> f.name >> f.fallback.file;
        *s >> f.fallback.size >> f.fallback.rightToLeft;
        unsigned variants = 0;
        *s >> variants;
        for (unsigned v = 0; v < variants; v++) {
            SLanguageFontInfo info;
            *s >> info.file >> info.size >> info.rightToLeft;
            unsigned langs = 0;
            *s >> langs;
            for (unsigned l = 0; l < langs; l++) {
                jet::String code;
                *s >> code;
                f.languages[code] = info;
            }
        }
        m_auroraFontsInfo.push_back(f);
    }
    s->Finish();
}

const SLanguageFontInfo &FontInfoMgr::Variant(const SFontInfo &f, const jet::String &code) {
    std::map<jet::String, SLanguageFontInfo>::const_iterator it = f.languages.find(code);
    return it == f.languages.end() ? f.fallback : it->second;
}

// @0x485c38: every font file of the language is there
bool FontInfoMgr::CanLoadFontsFor(babel::Lang lang) {
    jet::String code;
    lang.ToCode(code);
    for (size_t i = 0; i < m_auroraFontsInfo.size(); i++)
        if (!jet::stream::StreamMgr::GetInstance().CanCreateStream(Variant(m_auroraFontsInfo[i], code).file, false)) return false;
    return true;
}

// @0x485d08: makes the language's fonts on the 2D painter, sized by the GUI scale
void FontInfoMgr::LoadFontsFor(babel::Lang lang) {
    jet::String code;
    lang.ToCode(code);
    bool anyRightToLeft = false;
    for (size_t i = 0; i < m_auroraFontsInfo.size(); i++) {
        const SFontInfo &f = m_auroraFontsInfo[i];
        const SLanguageFontInfo &v = Variant(f, code);
        float h = (float)v.size * Sprite::GetDefaultPositionScale();
        bool rtl = v.rightToLeft == 1;
        FontMgr::GetInstance()->AddFreeTypeFont(f.name, v.file, h > 0 ? (unsigned)h : 0, PainterMgr::GetInstance()->Get2DPainter(), rtl, rtl);
        anyRightToLeft |= rtl;
    }
    FontMgr::GetInstance()->SetRightToLeft(anyRightToLeft);
}

// @0x485e34: empty past the last font
const jet::String &FontInfoMgr::GetFontName(unsigned i) {
    static const jet::String none;
    return i < m_auroraFontsInfo.size() ? m_auroraFontsInfo[i].name : none;
}
