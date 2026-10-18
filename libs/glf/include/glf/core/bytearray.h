// libs/glf/include/glf/core/bytearray.h
#ifndef LIBS_GLF_INCLUDE_GLF_CORE_BYTEARRAY_H
#define LIBS_GLF_INCLUDE_GLF_CORE_BYTEARRAY_H

namespace glf {
struct ByteArray {  // sizeof 0x4
    virtual ~ByteArray();
    bool Empty();
    virtual const unsigned char *GetData() = 0;
    virtual const unsigned char *GetData() = 0;
    virtual unsigned Size() = 0;
    bool operator==(const glf::ByteArray&);
    bool operator!=(const glf::ByteArray&);
};
}

#endif
