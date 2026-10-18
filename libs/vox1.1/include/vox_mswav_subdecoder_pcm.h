// libs/vox1.1/include/vox_mswav_subdecoder_pcm.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_PCM_H
#define LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_PCM_H

namespace vox {
class VoxMSWavSubDecoderPCM : public vox::VoxMSWavSubDecoder {  // sizeof 0x2C
public:
    VoxMSWavSubDecoderPCM(vox::StreamCursorInterface*, vox::WaveChunk*);
    virtual ~VoxMSWavSubDecoderPCM();
    virtual int Decode(void*, int);
    virtual int Seek(unsigned);
    virtual bool HasData();
private:
    int GetNbSamples();
    int GetDataSize();
};
}

#endif
