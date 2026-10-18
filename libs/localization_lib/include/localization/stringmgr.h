// libs/localization_lib/include/localization/stringmgr.h
#ifndef LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_STRINGMGR_H
#define LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_STRINGMGR_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "core/pstring.h"
#include "localization/filter.h"
#include "localization/lang.h"

namespace jet::stream { class IStream; class IStreamW; }

namespace babel {

class Babel;

// The ".texts" files: u32 count, u32 data base, then count x {u32 keyLen, key, u32 offset}; a text at
// base + offset is {u32 len, utf8}. Keys are the hashes of the (case-insensitive) names.
class StringMgr {  // sizeof 0x7C
public:
    typedef std::unordered_map<jet::String, std::unordered_map<unsigned, jet::String> > CustomMap;

    struct TTextInfo {
        TTextInfo();
        void Reset();
        jet::String text;  // 0x0
        unsigned char file;  // 0x4: index of the stream holding it
        unsigned offset;  // 0x8
        bool reading;  // 0xC: being resolved (stops reference cycles)
    };

    StringMgr(Babel *babel);
    ~StringMgr();
    const jet::String &Get(const jet::String &key) const;
    bool Exists(const jet::String &key) const;
    bool Load(Lang lang, std::shared_ptr<jet::stream::IStream> stream, bool overwrite);
    bool Load(Lang lang, const jet::String &name);
    bool LoadFromFile(Lang lang, const jet::String &path, bool overwrite);
    void Reset();
    void Clean();
    void CleanCustomString();
    void Update();
    void Add(const jet::String &key, const std::unordered_map<unsigned, jet::String> &langs, const jet::String &fallback);
    bool LoadCustomString(const jet::String &path);
    bool StoreCustomStrings(const jet::String &path);

    static const jet::String &NotFound() { return k_notFoundString; }
private:
    unsigned GetKey(const jet::String &s) const { return s.GetHash(); }
    void ReadString(TTextInfo &info);
    void ReadStringFromStream(jet::stream::IStream &s, TTextInfo &info);
    void CloseStreams();
    void CleanCache();
    jet::String Process(const jet::String &text);
    const jet::String &LoadString(const jet::String &key);
    bool LoadLangCustomStrings(Lang lang, const jet::String &path);
    bool StoreInStream(jet::stream::IStreamW &out, const std::unordered_map<jet::String, jet::String> &texts);

    static jet::String k_empty;
    static jet::String k_notFoundString;
    static jet::String k_specialTokenStart;
    static jet::String k_specialTokenEnd;
    std::unordered_map<unsigned, TTextInfo> m_texts;  // 0x0
    std::vector<std::shared_ptr<jet::stream::IStream> > m_streams;  // 0x18
    std::vector<std::shared_ptr<jet::stream::IStream> > m_startedStreams;  // 0x24
    Babel *m_babel;  // 0x30
    Filter m_frenchFilter;  // 0x34
    CustomMap m_customStrings;  // 0x64
};

}

#endif
