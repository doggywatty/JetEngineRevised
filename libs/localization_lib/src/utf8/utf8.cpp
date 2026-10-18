// libs/localization_lib/src/utf8/utf8.cpp
#include "utf8/utf8.h"

namespace babel::uft8 {

// @0x5b343c
void toUnicode(const unsigned char *p, unsigned &cp, unsigned &len) {
    unsigned c = p[0];
    if (!(c >> 7)) { cp = c; len = 1; }
    else if ((c >> 5) == 6) { cp = ((c & 0x1f) << 6) + (p[1] & 0x3f); len = 2; }
    else if ((c >> 4) == 14) { cp = ((c << 12) & 0xffff) + ((p[1] & 0x3f) << 6) + (p[2] & 0x3f); len = 3; }
    else if ((c >> 3) == 30) { cp = ((c & 3) << 18) + ((p[1] & 0x3f) << 12) + ((p[2] & 0x3f) << 6) + (p[3] & 0x3f); len = 4; }
    else { cp = c; len = 1; }
}

// @0x5b34f8
void fromUnicode(unsigned cp, unsigned char *out, unsigned &len) {
    if (cp < 0x80) { out[0] = (unsigned char)cp; len = 1; }
    else if (cp < 0x800) { out[0] = (unsigned char)(0xc0 | (cp >> 6)); out[1] = (unsigned char)(0x80 | (cp & 0x3f)); len = 2; }
    else if (cp < 0x10000) {
        out[0] = (unsigned char)(0xe0 | (cp >> 12)); out[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
        out[2] = (unsigned char)(0x80 | (cp & 0x3f)); len = 3;
    } else {
        out[0] = (unsigned char)(0xf0 | (cp >> 18)); out[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3f));
        out[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f)); out[3] = (unsigned char)(0x80 | (cp & 0x3f)); len = 4;
    }
}

}
