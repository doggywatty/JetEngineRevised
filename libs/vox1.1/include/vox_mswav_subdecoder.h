// libs/vox1.1/include/vox_mswav_subdecoder.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_H
#define LIBS_VOX1_1_INCLUDE_VOX_MSWAV_SUBDECODER_H

namespace vox {
class VoxMSWavSubDecoder {  // sizeof 0x2C
public:
    VoxMSWavSubDecoder(vox::StreamCursorInterface*, vox::WaveChunk*);
    virtual ~VoxMSWavSubDecoder();
    vox::TrackParams GetTrackParams();
    void SetLoop(bool);
    virtual int Decode(void*, int) = 0;
    virtual int Seek(unsigned) = 0;
    virtual bool HasData() = 0;
protected:
    void GoToNextDataChunk();
    vox::StreamCursorInterface *m_pStreamCursor;  // 0x4
    vox::WaveChunk *m_pWaveChunks;  // 0x8
    vox::DataNode *m_currentDataNode;  // 0xC
    vox::TrackParams m_trackParams;  // 0x10
    unsigned m_currentChunkPosition;  // 0x20
    unsigned m_decodedSamples;  // 0x24
    bool m_loop;  // 0x28
    bool m_isDecoderInError;  // 0x29
};
}

#endif
