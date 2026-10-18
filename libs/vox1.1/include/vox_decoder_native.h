// libs/vox1.1/include/vox_decoder_native.h
// The native format of the vox engine (".vxn", magic "VoxN"): an interactive music. A file is some audio segments (here each
// one a Musepack stream) and the chunks that say how they are played: the states of the music (each one a playlist of groups of
// segments) and the transitions between the states (a rule: when the old state stops and how fast it fades out).
//
//   "VoxN", u32 length (0x10), char version[8], u32 fileSize, u32 dataStart, then the chunks (id, u32 size, body):
//   Afmt (the audio format), Segm (u32 count + 6 u32 each: offset in the data, bytes, samples, 3 reserved), Cues (u32 count +
//   3 u32: id, segment, sample), Rule (count + 9 u32), Plst, Stat (count + u32 playlist and char label[28]), Trsn (count +
//   from, to, rule, reserved), Grps, Grpe, Msae (the extended format of an MS ADPCM file), Data (the audio at dataStart)
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_NATIVE_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_NATIVE_H

#include <map>
#include <string>
#include <vector>
#include "vox_decoder.h"
#include "vox_decoder_mpc8.h"
#include "vox_native_playlists.h"

namespace vox {

// the header and the audio format of the file (the .so's NativeChunks)
struct NativeChunks {  // sizeof 0x2C
    unsigned magic;  // 0x0
    unsigned headerLength;  // 0x4
    char version[8];  // 0x8
    unsigned fileSize;  // 0x10
    unsigned dataStart;  // 0x14
    unsigned short formatTag;  // 0x20 (Afmt): 0x800 Musepack, 1 PCM, 2 MS ADPCM, 0x11 IMA ADPCM
    unsigned short channels;  // 0x22
    unsigned sampleRate;  // 0x24
    unsigned short blockAlign;  // 0x28
    unsigned short bitsPerSample;  // 0x2A
    NativeChunks() : magic(0), headerLength(0), fileSize(0), dataStart(0), formatTag(0), channels(0), sampleRate(0), blockAlign(0), bitsPerSample(0) { version[0] = 0; }
};

struct AudioSegment {  // 0x18 bytes in the "Segm" chunk
    unsigned offset;  // in the data
    unsigned size;
    unsigned samples;
    unsigned reserved[3];
};

// a "Rule" record: how a state change goes
struct TransitionRule {  // 0x24 bytes in the "Rule" chunk
    int reserved0;
    int exitType;  // when the old state is left: 0 at once, 4 at the next cue, 5 at the next cue after the third, 6 at the end of its segment
    int flags;
    int entryType;
    int reserved1[2];
    float fadeOutTime;  // seconds, of the old state
    float fadeInTime;  // seconds, of the new one
    int reserved2;
};

// "Trsn": what goes from a state to another
struct TransitionParams {
    int rule;  // the index of the rule, -1 when the change is not allowed
    int reserved;
};

class DecoderNative : public DecoderInterface {  // sizeof 0xA4
public:
    DecoderNative();
    virtual ~DecoderNative();
    virtual DecoderCursorInterface *CreateNewCursor(StreamCursorInterface *stream);
    virtual void DestroyCursor(DecoderCursorInterface *cursor);
    virtual int GetType();
    virtual void *GetParam();
    bool NeedParsing() const { return m_needParsing; }
    void SetParsed() { m_needParsing = false; }
    NativeChunks *GetNativeChunks() { return &m_nativeChunks; }
    std::vector<AudioSegment> *GetAudioSegments() { return &m_audioSegments; }
    std::vector<int> *GetStates() { return &m_states; }  // the playlist of each state
    std::vector<TransitionRule> *GetTransitionRules() { return &m_transitionRules; }
    std::vector<std::vector<TransitionParams> > *GetTransitions() { return &m_transitions; }
    std::map<std::string, int> *GetStateLabels() { return &m_stateLabels; }
    NativePlaylistsManager *GetPlaylistsManager() { return &m_playlists; }
    std::vector<std::vector<int> > *GetSegmentsCues() { return &m_segmentsCues; }
private:
    NativeChunks m_nativeChunks;  // 0x4
    std::vector<AudioSegment> m_audioSegments;  // 0x30
    NativePlaylistsManager m_playlists;  // 0x38
    std::vector<int> m_states;  // 0x48
    std::vector<TransitionRule> m_transitionRules;  // 0x50
    std::vector<std::vector<int> > m_segmentsCues;  // 0x58
    std::vector<std::vector<TransitionParams> > m_transitions;  // 0x6C
    std::map<std::string, int> m_stateLabels;  // 0x80
    bool m_needParsing;  // 0xA0
};

// one reading of a segment of the file
struct MpcSegmentDecoder {  // sizeof 0x10
    MpcSegmentDecoder(StreamCursorInterface *source, const NativeChunks &chunks, const AudioSegment &segment);
    ~MpcSegmentDecoder();
    StreamNativeSegment *m_pStream;
    StreamCursorInterface *m_pStreamCursor;
    DecoderMPC8 *m_pDecoder;
    DecoderCursorInterface *m_pDecoderCursor;
};

class DecoderNativeCursor : public DecoderCursorInterface {  // sizeof 0x84
public:
    DecoderNativeCursor(DecoderInterface *decoder, StreamCursorInterface *stream);
    virtual ~DecoderNativeCursor();
    virtual int Decode(void *dst, int bytes);
    virtual int Seek(unsigned sample);
    virtual bool HasData();
    virtual void SetLoop(bool loop);
    virtual void Reset();
    virtual bool IsNative() { return true; }
    bool IsStateChangeValid(const char *label);
    virtual void SetInteractiveMusicState(const char *label);
private:
    // what plays a segment: the element of the playlist it is of, and the segment being read
    struct Player {
        Player() : segment(0), element(), segmentIndex(-1), remaining(0), loopsLeft(0), fadeTotal(0), fadePos(0), fadeIn(false), active(false) {}
        MpcSegmentDecoder *segment;
        PlaylistElement element;
        int segmentIndex;
        int remaining;  // samples left in this reading of the segment
        int loopsLeft;  // readings left of the element, -1 for ever
        int fadeTotal;  // the samples of the fade the player is under (0: none)
        int fadePos;
        bool fadeIn;  // the fade rises (a new state) or falls (a state that is left)
        bool active;
    };
    bool ParseFile();
    void StartPlaylist(int playlist, int fadeInSamples);
    bool StartElement(Player &p);
    void StopPlayer(Player &p);
    int DecodePlayer(Player &p, short *dst, int frames);
    int GetStateIndex() const { return m_currentState; }
    DecoderNative *m_pNative;
    NativeChunks *m_pNativeChunks;  // 0x20
    std::vector<AudioSegment> *m_pAudioSegments;  // 0x24
    std::vector<int> *m_pStates;  // 0x28
    std::vector<TransitionRule> *m_pTransitionRules;  // 0x2C
    std::vector<std::vector<TransitionParams> > *m_pTransitions;  // 0x30
    std::vector<std::vector<int> > *m_pSegmentsCues;  // 0x34
    std::map<std::string, int> *m_pStateLabels;  // 0x38
    NativePlaylistsManager *m_playlists;  // 0x3C (this cursor's copy)
    int m_currentState;
    int m_newState;  // the state asked for, -1 when none
    int m_currentPlaylist;
    Player m_current;
    Player m_dying;  // the old state, while it fades out
    bool m_ended;  // the playlist is over
    std::vector<short> m_scratch;
};

// the type 4 of VoxEngine::RegisterDecoderType
DecoderInterface *DecoderNativeFactory(void *params);

}  // namespace vox

#endif
