// libs/vox1.1/include/vox_native_subdecoder_pcm.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_PCM_H
#define LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_PCM_H

namespace vox {
class NativeSubDecoderPCMState : public vox::NativeSubDecoderState {  // sizeof 0x144
public:
    NativeSubDecoderPCMState(vox::NativePlaylistsManager*);
    virtual ~NativeSubDecoderPCMState();
};
}

namespace vox {
class VoxNativeSubDecoderPCM : public vox::VoxNativeSubDecoder {  // sizeof 0x170
public:
    VoxNativeSubDecoderPCM(vox::StreamCursorInterface*, vox::NativeChunks*, vox::States*, vox::AudioSegments*, std::vector<std::vector<int,vox::SAllocator<int,0> >,vox::SAllocator<std::vector<int,vox::SAllocator<int,0> >,0> >*, vox::TransitionRules*, std::vector<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,vox::SAllocator<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,0> >*, std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,int,vox::StringCompare,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,int>,0> >*, vox::NativePlaylistsManager*);
    virtual ~VoxNativeSubDecoderPCM();
protected:
    virtual int Seek(int, vox::SegmentState*);
public:
    virtual int Seek(unsigned);
    void GetState(vox::NativeSubDecoderPCMState*);
    void SetState(vox::NativeSubDecoderPCMState*);
protected:
    virtual int DecodeCurrentSegmentWithOffset(void*, int);
    virtual int DecodeSegment(void*, int, vox::SegmentState*);
    virtual int GetDecodingBuffer();
    virtual void ReleaseDecodingBuffer(int);
    virtual void Reset();
    virtual void SetCurrentSegmentInitialPosition(unsigned);
    virtual void SetDecodingBufferToSegmentPosition(vox::SegmentState*);
    virtual int EmulateDecodeCurrentSegmentWithOffset(int);
    virtual int EmulateDecodeSegment(int, vox::SegmentState*);
};
}

#endif
