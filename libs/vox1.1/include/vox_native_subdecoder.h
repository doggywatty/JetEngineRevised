// libs/vox1.1/include/vox_native_subdecoder.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_H
#define LIBS_VOX1_1_INCLUDE_VOX_NATIVE_SUBDECODER_H

namespace vox {
class NativeSubDecoderState {  // sizeof 0x144
public:
    NativeSubDecoderState(vox::NativePlaylistsManager*);
    virtual ~NativeSubDecoderState();
    vox::NativePlaylistsManager *m_pPlaylists;  // 0x4
    int m_oldState;  // 0x8
    int m_currentState;  // 0xC
    int m_newState;  // 0x10
    int m_oldPlaylist;  // 0x14
    int m_currentPlaylist;  // 0x18
    int m_newPlaylist;  // 0x1C
    bool m_resetPlaylist;  // 0x20
    vox::PlaylistElement m_oldPlaylistElement;  // 0x24
    vox::PlaylistElement m_currentPlaylistElement;  // 0x38
    vox::PlaylistElement m_newPlaylistElement;  // 0x4C
    int m_currentRule;  // 0x60
    int m_newRule;  // 0x64
    int m_nbSegmentsPlaying;  // 0x68
    vox::SegmentState m_dyingSegmentState;  // 0x6C
    vox::SegmentState m_oldSegmentState;  // 0xB0
    vox::SegmentState m_currentSegmentState;  // 0xF4
    int m_mixingStartPosition;  // 0x138
    int m_currentSegmentOffset;  // 0x13C
    bool m_hasData;  // 0x140
};
}

namespace vox {
class VoxNativeSubDecoder {  // sizeof 0x170
public:
    VoxNativeSubDecoder(vox::StreamCursorInterface*, vox::NativeChunks*, vox::States*, vox::AudioSegments*, std::vector<std::vector<int,vox::SAllocator<int,0> >,vox::SAllocator<std::vector<int,vox::SAllocator<int,0> >,0> >*, vox::TransitionRules*, std::vector<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,vox::SAllocator<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,0> >*, std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,int,vox::StringCompare,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,int>,0> >*, vox::NativePlaylistsManager*);
    virtual ~VoxNativeSubDecoder();
    static void Clean();
    virtual bool HasData();
protected:
    virtual int Seek(int, vox::SegmentState*) = 0;
public:
    virtual int Seek(unsigned) = 0;
    int Decode(void*, int);
    vox::TrackParams GetTrackParams();
    int GetCurrentState();
    bool IsStateChangeValid(int, int);
    virtual void Reset();
    int EmulateDecode(int);
    void GetState(vox::NativeSubDecoderState*);
    bool SetState(int);
    void SetState(vox::NativeSubDecoderState*);
    void SetLoop(bool);
protected:
    virtual int DecodeCurrentSegmentWithOffset(void*, int) = 0;
    virtual int DecodeSegment(void*, int, vox::SegmentState*) = 0;
    virtual int GetDecodingBuffer() = 0;
    virtual void ReleaseDecodingBuffer(int) = 0;
    virtual void SetCurrentSegmentInitialPosition(unsigned) = 0;
    virtual void SetDecodingBufferToSegmentPosition(vox::SegmentState*) = 0;
    virtual int EmulateDecodeCurrentSegmentWithOffset(int) = 0;
    virtual int EmulateDecodeSegment(int, vox::SegmentState*) = 0;
    int EmulateMixMultipleSegments(int);
    void EmulateMixSegmentInBuffer(int, vox::SegmentState*);
    void ApplyTransitionRule(vox::TransitionRule*);
    void CopySegmentState(vox::SegmentState&, const vox::SegmentState&);
    int GetNextDyingSegmentLifeState();
    void InterpretTransitionRule(int);
    bool IsExtraSegmentNeeded(vox::TransitionRule*);
    int MixMultipleSegments(short*, int);
    void MixSegmentInBuffer(short*, int, vox::SegmentState*);
    void StopSegment(vox::SegmentState*);
    void SwapOldAndCurrentSegments();
    void UpdateCurrentSegmentState(vox::TransitionRule*, bool);
    void UpdateDyingSegmentState(vox::TransitionRule*);
    void UpdateOldSegmentState(vox::TransitionRule*);
    void UpdateSegmentsStates();
    static int* s_pMixingBuffer;
    static int s_nbMixingBufferBytes;
    vox::StreamCursorInterface *m_pStreamCursor;  // 0x4
    vox::AudioFormat m_audioFormat;  // 0x8
    int m_dataStart;  // 0x14
    vox::AudioSegments *m_pAudioSegments;  // 0x18
    vox::TransitionRules *m_pTransitionRules;  // 0x1C
    vox::States *m_pStates;  // 0x20
    std::vector<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,vox::SAllocator<std::vector<vox::TransitionParams,vox::SAllocator<vox::TransitionParams,0> >,0> > *m_pTransitions;  // 0x24
    std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,int,vox::StringCompare,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,int>,0> > *m_pStateLabels;  // 0x28
    std::vector<std::vector<int,vox::SAllocator<int,0> >,vox::SAllocator<std::vector<int,vox::SAllocator<int,0> >,0> > *m_pSegmentsCues;  // 0x2C
    vox::NativePlaylistsManager *m_pPlaylists;  // 0x30
    int m_oldState;  // 0x34
    int m_currentState;  // 0x38
    int m_newState;  // 0x3C
    int m_oldPlaylist;  // 0x40
    int m_currentPlaylist;  // 0x44
    int m_newPlaylist;  // 0x48
    bool m_resetPlaylist;  // 0x4C
    vox::PlaylistElement m_oldPlaylistElement;  // 0x50
    vox::PlaylistElement m_currentPlaylistElement;  // 0x64
    vox::PlaylistElement m_newPlaylistElement;  // 0x78
    int m_currentRule;  // 0x8C
    int m_newRule;  // 0x90
    int m_nbSegmentsPlaying;  // 0x94
    vox::SegmentState m_dyingSegmentState;  // 0x98
    vox::SegmentState m_oldSegmentState;  // 0xDC
    vox::SegmentState m_currentSegmentState;  // 0x120
    int m_mixingStartPosition;  // 0x164
    int m_currentSegmentOffset;  // 0x168
    bool m_hasData;  // 0x16C
};
}

#endif
