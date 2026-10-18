// libs/vox1.1/src/adpcmdecoder.cpp
#include "adpcmdecoder.h"

const char AdpcmDecoder::cAdpcmIndexTable[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };

const short AdpcmDecoder::cAdpcmStepSizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
    143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
    1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
    9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };

short AdpcmDecoder::DecodeAdpcm(int nibble, AdpcmState *state) {
    int index = state->GetPrevIndex();
    int predictor = state->GetPrevSample();
    const int step = cAdpcmStepSizeTable[index];
    int diff = step >> 3;
    if (nibble & 1) diff += step >> 2;
    if (nibble & 2) diff += step >> 1;
    if (nibble & 4) diff += step;
    if (nibble & 8) diff = -diff;
    predictor += diff;
    if (predictor > 32767) predictor = 32767;
    else if (predictor < -32768) predictor = -32768;
    index += cAdpcmIndexTable[nibble & 15];
    if (index < 0) index = 0;
    else if (index > 88) index = 88;
    state->SetPrevSample((short)predictor);
    state->SetPrevIndex((unsigned char)index);
    return (short)predictor;
}

// `step` is the distance between two samples of the output (the channels of a block are interleaved)
void AdpcmDecoder::DecodeAdpcm(int count, AdpcmState *state, short *dst, const unsigned char *src, int step) {
    for (int i = 0; i < count; i++) {
        const unsigned char b = src[i >> 1];
        dst[i * step] = DecodeAdpcm((i & 1) ? (b >> 4) : (b & 15), state);
    }
}
