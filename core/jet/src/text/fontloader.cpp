// core/jet/src/text/fontloader.cpp
#include "text/fontloader.h"
#include "stream/streammgr.h"
#include <cstdio>

namespace jet::text {

FontLoader::FontLoader() {}
FontLoader::~FontLoader() {}

// @0x65af18: a stream of that name is opened through the StreamMgr and kept until Free
boost::shared_ptr<jet::text2::Font> FontLoader::LoadFreetypeFont(const jet::String &name) {
    auto it = m_map2.find(name.c_str());
    if (it != m_map2.end() && it->second) return it->second;
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    jet::stream::IStream *s = sm.CreateStream(name);
    if (!s) s = sm.CreateStreamByName(name);
    if (!s) {
        fprintf(stderr, "ERROR: cannot find stream '%s'\n", name.c_str());
        m_map2[name.c_str()].reset();
        return boost::shared_ptr<jet::text2::Font>();
    }
    boost::shared_ptr<jet::text2::Font> font(new jet::text2::Font());
    font->SetResourceName(name);
    if (!font->Init(s) || !font->Load()) return boost::shared_ptr<jet::text2::Font>();
    m_map2[name.c_str()] = font;
    return font;
}

// @0x65ac08: drops the caller's reference and uncaches the font once nothing else holds it
void FontLoader::Free(boost::shared_ptr<jet::text2::Font> &f) {
    if (!f) return;
    for (auto it = m_map2.begin(); it != m_map2.end(); ++it)
        if (it->second == f) {
            f.reset();
            if (it->second.use_count() == 1) m_map2.erase(it);
            return;
        }
}

}
