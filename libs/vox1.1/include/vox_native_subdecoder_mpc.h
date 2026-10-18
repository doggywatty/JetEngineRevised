// libs/vox1.1/include/vox_native_subdecoder_mpc.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_MPC_H
#define LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_MPC_H

namespace vox {
struct MpcSegmentDecoder {  // sizeof 0x10
    MpcSegmentDecoder();
    ~MpcSegmentDecoder();
    vox::StreamInterface *m_pStream;  // 0x0
    vox::StreamCursorInterface *m_pStreamCursor;  // 0x4
    vox::DecoderInterface *m_pDecoder;  // 0x8
    vox::DecoderCursorInterface *m_pDecoderCursor;  // 0xC
};
}

namespace vox {
class NativeSubDecoderMPCState : public vox::NativeSubDecoderState {  // sizeof 0x144
public:
    NativeSubDecoderMPCState(vox::NativePlaylistsManager*);
    virtual ~NativeSubDecoderMPCState();
};
}

namespace vox {
class VoxNativeSubDecoderMPC : public vox::VoxNativeSubDecoder {  // sizeof 0x190
public:
    VoxNativeSubDecoderMPC(vox::StreamCursorInterface*, vox::NativeChunks*, vox::States*, vox::AudioSegments*, std::vector<std::vector<int,vox::SAllocator<int,0> >,vox::SAllocator<std::vector<int,vox::SAllocator<int,0> >,0> >*, vox::TransitionRules*, std::vector<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,vox::SAllocator<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,0> >*, std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,int,vox::StringCompare,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,int>,0> >*, vox::NativePlaylistsManager*);
    virtual ~VoxNativeSubDecoderMPC();
protected:
    virtual int Seek(int, vox::SegmentState*);
public:
    virtual int Seek(unsigned);
    void GetState(vox::NativeSubDecoderMPCState*);
    void SetState(vox::NativeSubDecoderMPCState*);
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
    bool AddSegmentDecoder(int, vox::AudioSegment*);
    bool CreateSegmentDecoder(vox::SegmentState*);
    vox::DecoderCursorInterface *GetDecoderCursor(vox::SegmentState*);
    vox::DecoderInterface *m_pDecoder;  // 0x170
    std::map<int,vox::MpcSegmentDecoder *,vox::SegmentIdCompare,vox::SAllocator<std::pair<int,vox::MpcSegmentDecoder *>,0> > m_segmentDecoders;  // 0x174
};
}

#endif
