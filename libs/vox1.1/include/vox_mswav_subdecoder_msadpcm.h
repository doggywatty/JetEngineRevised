// libs/vox1.1/include/vox_mswav_subdecoder_msadpcm.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_MSADPCM_H
#define LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_MSADPCM_H

namespace vox {
class VoxMSWavSubDecoderMSADPCM : public vox::VoxMSWavSubDecoder {  // sizeof 0x450
public:
    VoxMSWavSubDecoderMSADPCM(vox::StreamCursorInterface*, vox::WaveChunk*);
    virtual ~VoxMSWavSubDecoderMSADPCM();
    virtual int Decode(void*, int);
    virtual int Seek(unsigned);
    virtual bool HasData();
private:
    int DecodeBlock(void*);
    short DecodeSample(vox::MsAdpcmState*, unsigned, const short*);
protected:
    unsigned char* m_readBuffer;  // 0x2C
    unsigned m_totalDataBytesRead;  // 0x30
    int m_dataStartPosition;  // 0x34
    int m_samplesInBuffer;  // 0x38
    int m_samplesInBufferConsumed;  // 0x3C
    unsigned m_totalSampleDecoded;  // 0x40
    unsigned char* m_blockReadBuffer;  // 0x44
    bool m_usingNeonDecoder;  // 0x48
    vox::FmtExtendedInfos m_fmtExtendedInfos;  // 0x4A
};
}

#endif
