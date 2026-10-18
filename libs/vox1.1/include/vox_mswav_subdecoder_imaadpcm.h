// libs/vox1.1/include/vox_mswav_subdecoder_imaadpcm.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_IMAADPCM_H
#define LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_IMAADPCM_H

namespace vox {
class VoxMSWavSubDecoderIMAADPCM : public vox::VoxMSWavSubDecoder {  // sizeof 0x88
public:
    VoxMSWavSubDecoderIMAADPCM(vox::StreamCursorInterface*, vox::WaveChunk*);
    virtual ~VoxMSWavSubDecoderIMAADPCM();
    virtual int Decode(void*, int);
    virtual int Seek(unsigned);
    virtual bool HasData();
private:
    int DecodeBlock(void*);
protected:
    AdpcmState m_states[8];  // 0x2C
    unsigned char* m_readBuffer;  // 0x4C
    bool m_usingNeonDecoder;  // 0x50
    unsigned m_samplesPerBlock;  // 0x54
    unsigned m_totalDataBytesRead;  // 0x58
    int m_dataStartPosition;  // 0x5C
    int m_samplesInBuffer;  // 0x60
    int m_samplesInBufferConsumed;  // 0x64
    unsigned m_totalSampleDecoded;  // 0x68
    unsigned char* m_blockReadBuffer;  // 0x6C
    unsigned char* m_extraReadBuffer;  // 0x70
    unsigned char* m_readBufferSpecialAlignment;  // 0x74
    unsigned char* m_extraReadBufferSpecialAlignment;  // 0x78
    unsigned char* m_extraBlockReadBuffer;  // 0x7C
    int m_samplesInExtraBuffer;  // 0x80
    int m_samplesInExtraBufferConsumed;  // 0x84
};
}

#endif
