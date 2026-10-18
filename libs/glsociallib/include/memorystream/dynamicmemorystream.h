// libs/glsociallib/include/memorystream/dynamicmemorystream.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_MEMORYSTREAM_DYNAMICMEMORYSTREAM_H
#define LIBS_GLSOCIALLIB_INCLUDE_MEMORYSTREAM_DYNAMICMEMORYSTREAM_H

namespace sociallib {
class CDynamicMemoryStream : public sociallib::CMemoryStream {  // sizeof 0x14
public:
    CDynamicMemoryStream(const sociallib::CDynamicMemoryStream&);
    CDynamicMemoryStream(const unsigned char*, unsigned);
    virtual ~CDynamicMemoryStream();
protected:
    virtual void onWriteOOR(unsigned);
private:
    unsigned roundToNextPowerof2(unsigned);
};
}

#endif
