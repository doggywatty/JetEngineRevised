// libs/localization_lib/src/babel.cpp
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include "stream/istream.h"
#include "stream/streammgr.h"
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <vector>

namespace babel {

// @0x5ae818
jet::String Strfmt(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return jet::String(buf);
}

// @0x5aeb68
bool ReadJsonFile(const jet::String &path, Json::Reader &reader, Json::Value &out) {
    std::unique_ptr<jet::stream::IStream> s(jet::stream::StreamMgr::GetInstance().CreateStream(path));
    if (!s) return false;
    s->Start();
    std::vector<char> data(s->Size());
    if (!data.empty()) s->Read(&data[0], (unsigned)data.size());
    bool ok = reader.parse(std::string(data.begin(), data.end()), out, true);
    s->Finish();
    return ok;
}

// @0x5ae89c: -- is "no language yet"
Babel::Babel() : m_currentLanguage(Lang::NONE), m_stringMgr(0), m_formatter(0), m_loaded(false) {
    m_stringMgr = new StringMgr(this);
    m_formatter = new Formatter(this);
}

Babel::~Babel() {
    delete m_stringMgr;
    delete m_formatter;
}

// @0x5aea1c: always reloads (the loaded flag is never set); the texts come from "<code>.texts"
bool Babel::SelectLanguage(Lang lang, bool load) {
    if (m_currentLanguage == lang && m_loaded) return true;
    m_stringMgr->Reset();
    m_currentLanguage = lang;
    bool ok = true;
    if (load) {
        jet::String code;
        lang.ToCode(code);
        ok = m_stringMgr->Load(lang, code + ".texts");
    }
    return m_formatter->UpdateLangInfo() & ok;
}

bool Babel::LoadFromFile(Lang lang, const jet::String &path, bool overwrite) { return m_stringMgr->LoadFromFile(lang, path, overwrite); }
void Babel::Clean() { m_stringMgr->Clean(); }
void Babel::Reset() { m_stringMgr->Reset(); }
void Babel::Update() { m_stringMgr->Update(); }

}
