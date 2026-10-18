// libs/manhattan/include/stream/spplzmadecompressor.h
#ifndef LIBS_MANHATTAN_INCLUDE_STREAM_SPPLZMADECOMPRESSOR_H
#define LIBS_MANHATTAN_INCLUDE_STREAM_SPPLZMADECOMPRESSOR_H

namespace manhattan::stream {
class SPPLzmaDecompressor : public manhattan::stream::StreamPipeProcess {  // sizeof 0xC
public:
    SPPLzmaDecompressor(manhattan::stream::IStreamW*, manhattan::stream::IStreamW*);
    virtual ~SPPLzmaDecompressor();
    virtual int Execute(unsigned char*, int, unsigned char*, int);
};
}

#endif
