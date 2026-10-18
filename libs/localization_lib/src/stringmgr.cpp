// libs/localization_lib/src/stringmgr.cpp
#include "localization/stringmgr.h"
#include "localization/babel.h"
#include "stream/filestream.h"
#include "stream/memorystream.h"
#include "stream/mmapstream.h"
#include "stream/streammgr.h"
#include "stream/streamutil.h"
#include <cstring>

namespace babel {

jet::String StringMgr::k_empty("");
jet::String StringMgr::k_notFoundString("???");
jet::String StringMgr::k_specialTokenStart("{{");
jet::String StringMgr::k_specialTokenEnd("}}");

static const unsigned k_langs[] = { Lang::EN, Lang::FR, Lang::IT, Lang::ES, Lang::JA, Lang::PT, Lang::BR, Lang::KO, Lang::ZH, Lang::RU, Lang::TR, Lang::AR };

// @0x5ab704
StringMgr::TTextInfo::TTextInfo() : text(k_empty), file(0xff), offset(0), reading(false) {}

// @0x5ab744
void StringMgr::TTextInfo::Reset() {
    text = k_empty;
    reading = false;
}

// @0x5aa8bc
StringMgr::StringMgr(Babel *babel) : m_babel(babel), m_frenchFilter(jet::String("frenchfix.json"), false) {}

StringMgr::~StringMgr() { CloseStreams(); }

// @0x5aaa1c: a custom text of the selected language first
const jet::String &StringMgr::Get(const jet::String &key) const {
    CustomMap::const_iterator c = m_customStrings.find(key);
    if (c != m_customStrings.end()) {
        std::unordered_map<unsigned, jet::String>::const_iterator l = c->second.find((unsigned)m_babel->GetSelectedLanguage());
        if (l != c->second.end()) return l->second;
    }
    std::unordered_map<unsigned, TTextInfo>::const_iterator it = m_texts.find(GetKey(key));
    return it == m_texts.end() ? k_notFoundString : it->second.text;
}

// @0x5aab4c
bool StringMgr::Exists(const jet::String &key) const { return m_texts.find(GetKey(key)) != m_texts.end(); }

// @0x5aaabc: texts referenced from another text are read on demand
const jet::String &StringMgr::LoadString(const jet::String &key) {
    std::unordered_map<unsigned, TTextInfo>::iterator it = m_texts.find(GetKey(key));
    if (it == m_texts.end()) return k_notFoundString;
    TTextInfo &info = it->second;
    if (info.reading) return k_empty;
    if (info.text.Equals(k_empty)) ReadString(info);
    return info.text;
}

// @0x5aab7c
void StringMgr::ReadString(TTextInfo &info) {
    std::shared_ptr<jet::stream::IStream> s = m_streams[info.file];
    if (!s->IsStarted()) {
        s->Start();
        m_startedStreams.push_back(s);
    }
    ReadStringFromStream(*s, info);
}

// @0x5aac10
void StringMgr::ReadStringFromStream(jet::stream::IStream &s, TTextInfo &info) {
    unsigned pos = s.Tell();
    s.SeekBeg(info.offset);
    info.reading = true;
    unsigned len = 0;
    s.Read(&len, 4);
    std::vector<char> buf(len + 1, 0);
    s.Read(&buf[0], len);
    info.text = Process(jet::String(&buf[0]));
    if (m_babel->GetSelectedLanguage() == Lang::FR) info.text = m_frenchFilter.Apply(info.text);
    info.reading = false;
    s.SeekBeg(pos);
}

// @0x5ab47c: "{{key}}" is replaced by the text of key
jet::String StringMgr::Process(const jet::String &text) {
    const std::string src(text.c_str());
    const std::string open(k_specialTokenStart.c_str()), close(k_specialTokenEnd.c_str());
    std::vector<std::string> tokens;
    for (size_t pos = 0; pos < src.size();) {
        size_t n = src.find(open, pos);
        if (n == std::string::npos) n = src.size();
        if (n > pos) tokens.push_back(src.substr(pos, n - pos));
        pos = n + open.size();
    }
    if (tokens.empty()) return text;
    std::string out;
    size_t i = 0;
    if (src.find(open) != 0) out += tokens[i++];
    for (; i < tokens.size(); i++) {
        size_t e = tokens[i].find(close);
        if (e == std::string::npos) { out += tokens[i]; continue; }
        out += LoadString(jet::String(tokens[i].substr(0, e).c_str())).c_str();
        out += tokens[i].substr(e + close.size());
    }
    return jet::String(out.c_str());
}

// @0x5aada4
bool StringMgr::Load(Lang, std::shared_ptr<jet::stream::IStream> stream, bool overwrite) {
    CleanCache();
    m_streams.push_back(stream);
    unsigned char file = (unsigned char)(m_streams.size() - 1);
    jet::stream::IStream &s = *stream;
    unsigned count = 0, base = 0;
    s.Read(&count, 4);
    s.Read(&base, 4);
    for (unsigned i = 0; i < count; i++) {
        unsigned keyLen = 0;
        s.Read(&keyLen, 4);
        std::vector<char> key(keyLen + 1, 0);
        s.Read(&key[0], keyLen);
        TTextInfo info;
        unsigned offset = 0;
        s.Read(&offset, 4);
        info.offset = offset + base;
        info.file = file;
        unsigned k = GetKey(jet::String(&key[0]));
        if (overwrite) m_texts[k] = info;
        else m_texts.insert(std::make_pair(k, info));
    }
    CloseStreams();
    for (std::unordered_map<unsigned, TTextInfo>::iterator it = m_texts.begin(); it != m_texts.end(); ++it) ReadString(it->second);
    CloseStreams();
    return true;
}

// @0x5ab0bc: every stream of that name (the base file and its patches)
bool StringMgr::Load(Lang lang, const jet::String &name) {
    std::vector<jet::stream::IStream*> streams = jet::stream::StreamMgr::GetInstance().CreateStreamArray(name);
    if (streams.empty()) return false;
    bool ok = true;
    for (jet::stream::IStream *raw : streams) {
        if (!raw) { ok = false; continue; }
        std::shared_ptr<jet::stream::IStream> s(raw);
        s->Start();
        ok &= Load(lang, s, false);
        s->Finish();
    }
    return ok;
}

// @0x5ab268
bool StringMgr::LoadFromFile(Lang lang, const jet::String &path, bool overwrite) {
    if (!jet::stream::IsFile(path)) return false;
    std::shared_ptr<jet::stream::IStream> s(new jet::stream::MMapStream(path));
    s->Start();
    bool ok = s->IsStarted();
    if (ok) Load(lang, s, overwrite);
    s->Finish();
    return ok;
}

// @0x5ab3d0
void StringMgr::Reset() {
    CloseStreams();
    m_texts.clear();
    m_streams.clear();
    m_customStrings.clear();
}

void StringMgr::Clean() { CleanCache(); }
void StringMgr::CleanCustomString() { m_customStrings.clear(); }
void StringMgr::Update() {}

// @0x5ab444
void StringMgr::CloseStreams() {
    for (size_t i = 0; i < m_startedStreams.size(); i++) m_startedStreams[i]->Finish();
    m_startedStreams.clear();
}

// @0x5ab6bc
void StringMgr::CleanCache() {
    for (std::unordered_map<unsigned, TTextInfo>::iterator it = m_texts.begin(); it != m_texts.end(); ++it) it->second.Reset();
}

// @0x5ab794: languages missing from langs get en/fr/it/es/ja/pt/br/ko/zh/ru/tr/ar = fallback
void StringMgr::Add(const jet::String &key, const std::unordered_map<unsigned, jet::String> &langs, const jet::String &fallback) {
    std::unordered_map<unsigned, jet::String> all(langs);
    for (unsigned l : k_langs)
        if (all.find(l) == all.end()) all[l] = fallback;
    if (m_customStrings.find(key) == m_customStrings.end()) m_customStrings[key] = all;
}

// @0x5abc44
bool StringMgr::LoadCustomString(const jet::String &path) {
    for (unsigned l : k_langs)
        if (!LoadLangCustomStrings(Lang(l), path)) return false;
    return true;
}

// @0x5abd70: "<path><code>.texts"
bool StringMgr::LoadLangCustomStrings(Lang lang, const jet::String &path) {
    jet::String code;
    lang.ToCode(code);
    jet::String file = path + code + ".texts";
    if (!jet::stream::IsFile(file)) return false;
    jet::stream::MMapStream s(file);
    s.Start();
    if (!s.IsStarted()) return false;
    unsigned count = 0, base = 0;
    s.Read(&count, 4);
    s.Read(&base, 4);
    for (unsigned i = 0; i < count; i++) {
        unsigned keyLen = 0;
        s.Read(&keyLen, 4);
        std::vector<char> key(keyLen + 1, 0);
        s.Read(&key[0], keyLen);
        TTextInfo info;
        unsigned offset = 0;
        s.Read(&offset, 4);
        info.offset = offset + base;
        ReadStringFromStream(s, info);
        m_customStrings[jet::String(&key[0])][(unsigned)lang] = info.text;
    }
    s.Finish();
    return true;
}

// @0x5ac030
bool StringMgr::StoreCustomStrings(const jet::String &path) {
    std::unordered_map<unsigned, std::unordered_map<jet::String, jet::String> > perLang;
    for (CustomMap::const_iterator c = m_customStrings.begin(); c != m_customStrings.end(); ++c)
        for (std::unordered_map<unsigned, jet::String>::const_iterator l = c->second.begin(); l != c->second.end(); ++l) perLang[l->first][c->first] = l->second;
    for (std::unordered_map<unsigned, std::unordered_map<jet::String, jet::String> >::const_iterator it = perLang.begin(); it != perLang.end(); ++it) {
        jet::String code;
        Lang(it->first).ToCode(code);
        jet::stream::FileStream f(path + code + ".texts", (jet::stream::FileStream::OpenMode)(jet::stream::FileStream::k_create), true);
        f.Start();
        if (f.IsOpen()) StoreInStream(f, it->second);
        f.Finish();
    }
    return false;
}

// @0x5ac2c8: the index {count, base, entries} then the text block
bool StringMgr::StoreInStream(jet::stream::IStreamW &out, const std::unordered_map<jet::String, jet::String> &texts) {
    jet::stream::MemoryStream index, block;
    unsigned count = (unsigned)texts.size();
    index << count;
    unsigned basePos = index.Tell();
    index << count;
    for (std::unordered_map<jet::String, jet::String>::const_iterator it = texts.begin(); it != texts.end(); ++it) {
        unsigned offset = block.Tell();
        unsigned len = it->second.size();
        block << len;
        if (len) block.Write(it->second.c_str(), len);
        unsigned keyLen = it->first.size();
        index << keyLen;
        if (keyLen) index.Write(it->first.c_str(), keyLen);
        index << offset;
    }
    unsigned base = index.Tell();
    index.SeekBeg(basePos);
    index << base;
    index.SeekBeg(base);
    out.Write(&index.GetInternalArray()[0], base);
    if (block.Tell()) out.Write(&block.GetInternalArray()[0], block.Tell());
    return false;
}

}
