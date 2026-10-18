// libs/glot_lib/include/utils/crchelpers.h
#ifndef LIBS_GLOT_LIB_INCLUDE_UTILS_CRCHELPERS_H
#define LIBS_GLOT_LIB_INCLUDE_UTILS_CRCHELPERS_H

namespace glot {
class GlotCRCHelpers {  // sizeof 0x1
public:
    static unsigned long crc(unsigned char*, unsigned);
private:
    static unsigned long update_crc(unsigned long, unsigned char*, unsigned);
    static void make_crc_table();
    static const unsigned crc_table_size;
    static unsigned long crc_table[256];
    static bool crc_table_computed;
};
}

#endif
