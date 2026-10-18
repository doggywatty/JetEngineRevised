// libs/vox1.1/include/vox_native_subdecoder_imaadpcm.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_IMAADPCM_H
#define LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_IMAADPCM_H

namespace vox {
class NativeSubDecoderIMAADPCMState : public vox::NativeSubDecoderState {  // sizeof 0x150
public:
    NativeSubDecoderIMAADPCMState(vox::NativePlaylistsManager*);
    virtual ~NativeSubDecoderIMAADPCMState();
    int m_decodeBuffersState[3];  // 0x144
};
}

namespace vox {
class VoxNativeSubDecoderIMAADPCM : public vox::VoxNativeSubDecoder {  // sizeof 0x1C0
public:
    VoxNativeSubDecoderIMAADPCM(vox::StreamCursorInterface*, vox::NativeChunks*, vox::States*, vox::AudioSegments*, std::vector<std::vector<int,vox::SAllocator<int,0> >,vox::SAllocator<std::vector<int,vox::SAllocator<int,0> >,0> >*, vox::TransitionRules*, std::vector<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,vox::SAllocator<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,0> >*, std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,int,vox::StringCompare,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,int>,0> >*, vox::NativePlaylistsManager*);
    virtual ~VoxNativeSubDecoderIMAADPCM();
protected:
    virtual int Seek(int, vox::SegmentState*);
public:
    virtual int Seek(unsigned);
    void GetState(vox::NativeSubDecoderIMAADPCMState*);
    void SetState(vox::NativeSubDecoderIMAADPCMState*);
protected:
    virtual int DecodeCurrentSegmentWithOffset(void*, int);
    virtual int DecodeSegment(void*, int, vox::SegmentState*);
    virtual int GetDecodingBuffer();
    virtual void ReleaseDecodingBuffer(int);
    virtual void Reset();
    virtual void SetCurrentSegmentInitialPosition(unsigned);
    virtual void SetDecodingBufferToSegmentPosition(vox::SegmentState*);
    virtual void EmulateSetDecodingBufferToSegmentPosition(vox::SegmentState*);
    virtual int EmulateDecodeCurrentSegmentWithOffset(int);
    virtual int EmulateDecodeSegment(int, vox::SegmentState*);
private:
    int DecodeBlock(void*, vox::SegmentState*);
    int EmulateDecodeBlock(vox::SegmentState*);
protected:
    unsigned m_samplesPerBlock;  // 0x170
    unsigned char* *m_pDecodeBuffers;  // 0x174
    int m_samplesInBuffer[3];  // 0x178
    int m_samplesInBufferConsumed[3];  // 0x184
    int m_decodeBuffersState[3];  // 0x190
    unsigned char* m_blockReadBuffer;  // 0x19C
    AdpcmState m_states[8];  // 0x1A0
};
}

#endif
