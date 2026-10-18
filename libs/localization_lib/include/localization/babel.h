// libs/localization_lib/include/localization/babel.h
#ifndef LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_BABEL_H
#define LIBS_LOCALIZATION_LIB_INCLUDE_LOCALIZATION_BABEL_H

#include "core/pstring.h"
#include "jsingleton.h"
#include "json/json.h"
#include "localization/lang.h"

namespace babel {

class Formatter;
class StringMgr;

jet::String Strfmt(const char *fmt, ...);
bool ReadJsonFile(const jet::String &path, Json::Reader &reader, Json::Value &out);

// The localization front end: the selected language, its texts and the number/date formatter.
class Babel : public Singleton<Babel> {  // sizeof 0x14
public:
    Babel();
    virtual ~Babel();
    Lang GetSelectedLanguage() const { return m_currentLanguage; }
    bool SelectLanguage(Lang lang, bool load);
    StringMgr *GetStringMgr() const { return m_stringMgr; }
    Formatter *GetFormatter() const { return m_formatter; }
    bool LoadFromFile(Lang lang, const jet::String &path, bool overwrite);
    void Clean();
    void Reset();
    void Update();
private:
    Lang m_currentLanguage;  // 0x4
    StringMgr *m_stringMgr;  // 0x8
    Formatter *m_formatter;  // 0xC
    bool m_loaded;  // 0x10
};

}

#endif
