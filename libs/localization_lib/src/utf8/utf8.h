// libs/localization_lib/src/utf8/utf8.h
#ifndef LIBS_LOCALIZATION_LIB_SRC_UTF8_UTF8_H
#define LIBS_LOCALIZATION_LIB_SRC_UTF8_UTF8_H

namespace babel::uft8 {
void toUnicode(const unsigned char *p, unsigned &cp, unsigned &len);
void fromUnicode(unsigned cp, unsigned char *out, unsigned &len);
}

#endif
