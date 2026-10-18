// libs/vox1.1/include/adpcmdecoder.h
#ifndef LIBS_VOX1_1_INCLUDE_ADPCMDECODER_H
#define LIBS_VOX1_1_INCLUDE_ADPCMDECODER_H

#include "adpcmstate.h"

// IMA ADPCM, 4 bits per sample
class AdpcmDecoder {  // sizeof 0x1
public:
    // decodes `count` samples of nibbles (low nibble first) into dst
    static void DecodeAdpcm(int count, AdpcmState *state, short *dst, const unsigned char *src, int step);
    static short DecodeAdpcm(int nibble, AdpcmState *state);
    static const char cAdpcmIndexTable[16];
    static const short cAdpcmStepSizeTable[89];
};

#endif
