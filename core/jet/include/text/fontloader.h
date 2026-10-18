// core/jet/include/text/fontloader.h
#ifndef CORE_JET_INCLUDE_TEXT_FONTLOADER_H
#define CORE_JET_INCLUDE_TEXT_FONTLOADER_H

#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include "jsingleton.h"
#include "core/pstring.h"
#include "text2/font2.h"

namespace jet::text {
// TrueType font cache keyed by stream name. The BMFont / Aurora / old FreeType font types of the
// engine have no callers in the 0.0.3 game and are not rebuilt.
class FontLoader : public Singleton<FontLoader> {  // sizeof 0x54
public:
    FontLoader();
    virtual ~FontLoader();
    boost::shared_ptr<jet::text2::Font> LoadFreetypeFont(const jet::String&);
    void Free(boost::shared_ptr<jet::text2::Font>&);
private:
    std::unordered_map<std::string, boost::shared_ptr<jet::text2::Font> > m_map2;  // 0x20
};
}

#endif
