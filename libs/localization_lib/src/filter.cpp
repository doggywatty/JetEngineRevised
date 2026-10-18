// libs/localization_lib/src/filter.cpp
#include "localization/filter.h"
#include "localization/babel.h"
#include "utf8/utf8.h"
#include <string>

namespace babel {

// @0x5aeca4
Filter::Filter(const jet::String &file, bool reverse) {
    Json::Value root(Json::nullValue);
    Json::Reader reader;
    if (!ReadJsonFile(file, reader, root)) return;
    m_conversion.clear();
    if (!root.isArray()) return;
    for (unsigned i = 0; i < root.size(); i++) {
        Json::Value &e = root[i];
        if (!e.isMember("in") || !e.isMember("out")) continue;
        unsigned in = 0, out = 0, len = 0;
        uft8::toUnicode((const unsigned char*)e["in"].asCString(), in, len);
        uft8::toUnicode((const unsigned char*)e["out"].asCString(), out, len);
        if (!reverse) {
            m_conversion.insert(std::make_pair(in, out));
            m_result.insert(out);
        } else {
            m_conversion.insert(std::make_pair(out, in));
            m_result.insert(in);
        }
    }
}

Filter::~Filter() {}

// @0x5af150: below 0x30 untouched; unmapped characters are remembered
unsigned Filter::Convert(unsigned cp) const {
    if (cp < 0x30) return cp;
    std::unordered_map<unsigned, unsigned>::const_iterator it = m_conversion.find(cp);
    if (it != m_conversion.end()) return it->second;
    m_result.insert(cp);
    return cp;
}

// @0x5aef70
jet::String Filter::Apply(const jet::String &text) const {
    std::string out;
    const unsigned char *p = (const unsigned char*)text.c_str();
    for (unsigned i = 0; i < text.size();) {
        unsigned cp = 0, len = 1;
        uft8::toUnicode(p + i, cp, len);
        unsigned char buf[4];
        unsigned n = 0;
        uft8::fromUnicode(Convert(cp), buf, n);
        out.append((const char*)buf, n);
        i += len;
    }
    return jet::String(out.c_str());
}

}
