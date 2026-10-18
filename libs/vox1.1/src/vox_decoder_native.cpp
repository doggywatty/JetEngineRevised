// libs/vox1.1/src/vox_decoder_native.cpp
#include <algorithm>
#include <cctype>
#include "vox_decoder_native.h"

namespace vox {

static bool LabelEquals(const char *a, const char *b) {
    for (; *a && *b; a++, b++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    return *a == *b;
}

static unsigned ReadLE32(const unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24); }
static int ReadLE32s(const unsigned char *p) { return (int)ReadLE32(p); }
static unsigned short ReadLE16(const unsigned char *p) { return (unsigned short)(p[0] | (p[1] << 8)); }

// the type 4 of VoxEngine::RegisterDecoderType (the factory @0x5e8f74 of the .so)
DecoderInterface *DecoderNativeFactory(void *) { return new DecoderNative(); }

DecoderNative::DecoderNative() : m_needParsing(true) {}

DecoderNative::~DecoderNative() {}

DecoderCursorInterface *DecoderNative::CreateNewCursor(StreamCursorInterface *stream) { return new DecoderNativeCursor(this, stream); }

void DecoderNative::DestroyCursor(DecoderCursorInterface *cursor) { delete cursor; }

int DecoderNative::GetType() { return kFormatVxn; }

void *DecoderNative::GetParam() { return 0; }

MpcSegmentDecoder::MpcSegmentDecoder(StreamCursorInterface *source, const NativeChunks &chunks, const AudioSegment &segment)
    : m_pStream(0), m_pStreamCursor(0), m_pDecoder(0), m_pDecoderCursor(0) {
    StreamNativeSegmentParams params;
    params.source = source;
    params.offset = (int)(chunks.dataStart + segment.offset);
    params.size = (int)segment.size;
    m_pStream = new StreamNativeSegment(&params);
    m_pStreamCursor = m_pStream->CreateNewCursor();
    DecoderMPC8Params mpc;
    mpc.forceSampleRate = (int)chunks.sampleRate;
    m_pDecoder = new DecoderMPC8(&mpc);
    m_pDecoderCursor = m_pDecoder->CreateNewCursor(m_pStreamCursor);
}

MpcSegmentDecoder::~MpcSegmentDecoder() {
    if (m_pDecoder && m_pDecoderCursor) m_pDecoder->DestroyCursor(m_pDecoderCursor);
    if (m_pStream && m_pStreamCursor) m_pStream->DestroyCursor(m_pStreamCursor);
    delete m_pDecoder;
    delete m_pStream;
}

// @0x5cefac (DecoderNativeCursor::ParseFile): the chunks are read once, by the first cursor of the decoder
bool DecoderNativeCursor::ParseFile() {
    NativeChunks &nc = *m_pNativeChunks;
    if (m_pStreamCursor->IsSeekable()) m_pStreamCursor->Seek(0, kSeekSet);
    unsigned char head[8];
    if (m_pStreamCursor->Read(head, 8) != 8 || ReadLE32(head) != 0x4e786f56 /* "VoxN" */) return false;
    nc.magic = ReadLE32(head);
    nc.headerLength = ReadLE32(head + 4);
    if (nc.headerLength < 16 || nc.headerLength > 256) return false;
    unsigned char hdr[256];
    if (m_pStreamCursor->Read(hdr, (int)nc.headerLength) != (int)nc.headerLength) return false;
    memcpy(nc.version, hdr, 8);
    nc.fileSize = ReadLE32(hdr + 8);
    nc.dataStart = ReadLE32(hdr + 12);
    // the chunks are what is between the header and the "Data" chunk's own header
    const int chunksSize = (int)nc.dataStart - 0x10 - (int)nc.headerLength;
    if (chunksSize <= 0) return false;
    std::vector<unsigned char> buf(chunksSize);
    if (m_pStreamCursor->Read(&buf[0], chunksSize) != chunksSize) return false;

    DecoderNative *d = m_pNative;
    m_pStates->clear();
    NativePlaylistsManager *pm = d->GetPlaylistsManager();
    std::vector<PlaylistInfos> playlistInfos;
    std::vector<GroupInfos> groupInfos;
    std::vector<PlaylistElementInfos> elementInfos;
    int pos = 0;
    while (pos + 8 <= chunksSize) {
        const unsigned char *c = &buf[pos];
        const unsigned id = ReadLE32(c);
        const int size = (int)ReadLE32(c + 4);
        const unsigned char *body = c + 8;
        pos += 8 + size;
        if (size < 0 || pos > chunksSize) return false;
        if (id == 0x746d6641) {  // "Afmt"
            if (size < 12) return false;
            nc.formatTag = ReadLE16(body);
            nc.channels = ReadLE16(body + 2);
            nc.sampleRate = ReadLE32(body + 4);
            nc.blockAlign = ReadLE16(body + 8);
            nc.bitsPerSample = 16;
        } else if (id == 0x6d676553) {  // "Segm"
            const int n = (int)ReadLE32(body);
            if (n <= 0 || (size - 4) / n < 24) return false;
            const int each = (size - 4) / n;
            m_pAudioSegments->resize(n);
            m_pSegmentsCues->resize(n);
            for (int i = 0; i < n; i++) {
                const unsigned char *s = body + 4 + i * each;
                AudioSegment &a = (*m_pAudioSegments)[i];
                a.offset = ReadLE32(s);
                a.size = ReadLE32(s + 4);
                a.samples = ReadLE32(s + 8);
                a.reserved[0] = ReadLE32(s + 12);
                a.reserved[1] = ReadLE32(s + 16);
                a.reserved[2] = ReadLE32(s + 20);
                (*m_pSegmentsCues)[i].assign(1, 0);  // every segment has a cue at its start
            }
        } else if (id == 0x73657543) {  // "Cues": the sample of a cue of a segment
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 12) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    const int segment = ReadLE32s(s + 4);
                    if (segment >= 0 && segment < (int)m_pSegmentsCues->size()) (*m_pSegmentsCues)[segment].push_back(ReadLE32s(s + 8));
                }
            }
        } else if (id == 0x656c7552) {  // "Rule"
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 36) {
                const int each = (size - 4) / n;
                m_pTransitionRules->resize(n);
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    TransitionRule &r = (*m_pTransitionRules)[i];
                    r.reserved0 = ReadLE32s(s);
                    r.exitType = ReadLE32s(s + 4);
                    r.flags = ReadLE32s(s + 8);
                    r.entryType = ReadLE32s(s + 12);
                    r.reserved1[0] = ReadLE32s(s + 16);
                    r.reserved1[1] = ReadLE32s(s + 20);
                    unsigned fo = ReadLE32(s + 24), fi = ReadLE32(s + 28);
                    memcpy(&r.fadeOutTime, &fo, 4);
                    memcpy(&r.fadeInTime, &fi, 4);
                    r.reserved2 = ReadLE32s(s + 32);
                }
            }
        } else if (id == 0x74736c50) {  // "Plst"
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 8) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    PlaylistInfos p;
                    p.groupSwitchMode = ReadLE32s(body + 4 + i * each);
                    p.nbLoops = ReadLE32s(body + 8 + i * each);
                    playlistInfos.push_back(p);
                }
            }
        } else if (id == 0x74617453) {  // "Stat": the playlist and the label of each state
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 5) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    m_pStates->push_back(ReadLE32s(s));
                    char label[64] = { 0 };
                    memcpy(label, s + 4, std::min(each - 4, 63));
                    (*m_pStateLabels)[label] = i;
                }
            }
        } else if (id == 0x6e737254) {  // "Trsn": from a state, to a state, the rule (and one reserved)
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 16) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    const int from = ReadLE32s(s);
                    if (from < 0 || from > 1024) return false;
                    if ((int)m_pTransitions->size() <= from) m_pTransitions->resize(from + 1);
                    TransitionParams t;
                    t.rule = ReadLE32s(s + 8);
                    t.reserved = ReadLE32s(s + 12);
                    (*m_pTransitions)[from].push_back(t);
                }
            }
        } else if (id == 0x73707247) {  // "Grps"
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 24) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    GroupInfos g;
                    g.playlist = ReadLE32s(s); g.group = ReadLE32s(s + 4); g.selectMode = ReadLE32s(s + 8);
                    g.reserved = ReadLE32s(s + 12); g.nbLoops = ReadLE32s(s + 16); g.noRepeatLength = ReadLE32s(s + 20);
                    groupInfos.push_back(g);
                }
            }
        } else if (id == 0x65707247) {  // "Grpe"
            const int n = (int)ReadLE32(body);
            if (n > 0 && (size - 4) / n >= 32) {
                const int each = (size - 4) / n;
                for (int i = 0; i < n; i++) {
                    const unsigned char *s = body + 4 + i * each;
                    PlaylistElementInfos e;
                    e.playlist = ReadLE32s(s); e.position = ReadLE32s(s + 4); e.group = ReadLE32s(s + 8); e.segmentIndex = ReadLE32s(s + 12);
                    e.entryPointType = ReadLE32s(s + 16); e.playPreEntry = ReadLE32s(s + 20); e.nbLoops = ReadLE32s(s + 24); e.playPostExit = ReadLE32s(s + 28);
                    elementInfos.push_back(e);
                }
            }
        }
        // "Msae" (the extended format of MS ADPCM segments) and the unknown chunks are skipped
    }
    pm->Init((int)playlistInfos.size());
    for (size_t i = 0; i < playlistInfos.size(); i++) pm->AddPlaylist((int)i, playlistInfos[i]);
    for (size_t i = 0; i < groupInfos.size(); i++) pm->AddGroup(groupInfos[i]);
    for (size_t i = 0; i < elementInfos.size(); i++) pm->AddPlaylistElement(elementInfos[i]);
    return pm->IsValid() && !m_pAudioSegments->empty() && nc.sampleRate && nc.channels && nc.formatTag == 0x800;
}

DecoderNativeCursor::DecoderNativeCursor(DecoderInterface *decoder, StreamCursorInterface *stream)
    : DecoderCursorInterface(decoder, stream), m_pNative(static_cast<DecoderNative*>(decoder)), m_currentState(-1), m_newState(-1),
      m_playlists(0), m_currentPlaylist(-1), m_ended(false) {
    m_pNativeChunks = m_pNative->GetNativeChunks();
    m_pAudioSegments = m_pNative->GetAudioSegments();
    m_pStates = m_pNative->GetStates();
    m_pTransitionRules = m_pNative->GetTransitionRules();
    m_pTransitions = m_pNative->GetTransitions();
    m_pSegmentsCues = m_pNative->GetSegmentsCues();
    m_pStateLabels = m_pNative->GetStateLabels();
    if (m_pNative->NeedParsing()) {
        if (!ParseFile()) { m_isDecoderInError = true; return; }
        m_pNative->SetParsed();
    }
    m_playlists = new NativePlaylistsManager(*m_pNative->GetPlaylistsManager());
    m_trackParams.numChannels = m_pNativeChunks->channels;
    m_trackParams.sampleRate = (int)m_pNativeChunks->sampleRate;
    m_trackParams.bitsPerSample = 16;
    // the length of the track: the segments of the first state played once each
    int total = 0;
    {
        NativePlaylistsManager probe(*m_pNative->GetPlaylistsManager());
        const int pl = m_pStates->empty() ? 0 : (*m_pStates)[0];
        PlaylistElement e;
        NativePlaylist *p = probe.GetPlaylist(pl);
        for (int guard = 0; p && guard < 64 && p->NextElement(e); guard++)
            if (e.m_segmentIndex >= 0 && e.m_segmentIndex < (int)m_pAudioSegments->size()) total += (int)(*m_pAudioSegments)[e.m_segmentIndex].samples;
    }
    m_trackParams.numSamples = total;
    m_currentState = m_pStates->empty() ? -1 : 0;
    StartPlaylist(m_pStates->empty() ? 0 : (*m_pStates)[0], 0);
}

DecoderNativeCursor::~DecoderNativeCursor() {
    StopPlayer(m_current);
    StopPlayer(m_dying);
    delete m_playlists;
}

void DecoderNativeCursor::StopPlayer(Player &p) {
    delete p.segment;
    p.segment = 0;
    p.active = false;
}

// the next element of the playlist is read from its start
bool DecoderNativeCursor::StartElement(Player &p) {
    StopPlayer(p);
    NativePlaylist *pl = m_playlists->GetPlaylist(m_currentPlaylist);
    PlaylistElement e;
    if (!pl || !pl->NextElement(e) || e.m_segmentIndex < 0 || e.m_segmentIndex >= (int)m_pAudioSegments->size()) return false;
    p.element = e;
    p.segmentIndex = e.m_segmentIndex;
    const AudioSegment &seg = (*m_pAudioSegments)[p.segmentIndex];
    p.segment = new MpcSegmentDecoder(m_pStreamCursor, *m_pNativeChunks, seg);
    p.remaining = (int)seg.samples;
    p.loopsLeft = e.m_nbLoops == 0 ? 1 : e.m_nbLoops;
    p.active = true;
    return true;
}

void DecoderNativeCursor::StartPlaylist(int playlist, int fadeInSamples) {
    m_currentPlaylist = playlist;
    m_playlists->ResetPlaylist(playlist);
    m_current = Player();
    m_current.fadeTotal = fadeInSamples;
    m_current.fadeIn = true;
    m_ended = !StartElement(m_current);
}

// decodes the player, going on to the next element of the playlist when a segment is over; returns the frames it made
int DecoderNativeCursor::DecodePlayer(Player &p, short *dst, int frames) {
    const int ch = m_trackParams.numChannels;
    int done = 0, idle = 0;
    while (done < frames && p.active) {
        if (p.remaining <= 0) {
            // a reading of the segment is over: again, or the next element
            if (p.loopsLeft < 0 || p.loopsLeft > 1) {
                if (p.loopsLeft > 1) p.loopsLeft--;
                if (p.segment->m_pDecoderCursor->Seek(0) != 0) { p.remaining = 0; p.active = false; break; }
                p.remaining = (int)(*m_pAudioSegments)[p.segmentIndex].samples;
            } else if (&p == &m_current) {
                if (!StartElement(p)) { p.active = false; break; }
            } else {
                p.active = false;
                break;
            }
            if (p.remaining <= 0) { p.active = false; break; }
        }
        int n = std::min(frames - done, p.remaining);
        const int got = p.segment->m_pDecoderCursor->Decode(dst + done * ch, n * ch * 2) / (ch * 2);
        if (got <= 0) {
            // the stream is shorter than its segment says
            p.remaining = 0;
            if (++idle > 2) { p.active = false; break; }
            continue;
        }
        idle = 0;
        p.remaining -= got;
        done += got;
    }
    return done;
}

bool DecoderNativeCursor::IsStateChangeValid(const char *label) {
    std::map<std::string, int>::iterator it = m_pStateLabels->end();
    for (it = m_pStateLabels->begin(); it != m_pStateLabels->end(); ++it)
        if (LabelEquals(it->first.c_str(), label)) break;
    if (it == m_pStateLabels->end() || m_currentState < 0) return false;
    const int to = it->second;
    if (to == m_currentState || m_currentState >= (int)m_pTransitions->size()) return false;
    const std::vector<TransitionParams> &t = (*m_pTransitions)[m_currentState];
    return to < (int)t.size() && t[to].rule >= 0 && t[to].rule < (int)m_pTransitionRules->size();
}

void DecoderNativeCursor::SetInteractiveMusicState(const char *label) {
    if (!label || m_isDecoderInError || !IsStateChangeValid(label)) return;
    for (std::map<std::string, int>::iterator it = m_pStateLabels->begin(); it != m_pStateLabels->end(); ++it)
        if (LabelEquals(it->first.c_str(), label)) m_newState = it->second;
}

int DecoderNativeCursor::Decode(void *dstv, int bytes) {
    if (m_isDecoderInError) return -1;
    const int ch = m_trackParams.numChannels;
    const int frames = bytes / (ch * 2);
    short *dst = static_cast<short*>(dstv);
    int done = 0;
    while (done < frames) {
        int chunk = frames - done;
        if (m_newState >= 0) {
            const TransitionRule &rule = (*m_pTransitionRules)[(*m_pTransitions)[m_currentState][m_newState].rule];
            if (rule.exitType == 0 || !m_current.active) {
                // @0x5d2c7c / @0x5d2dbc: the old state fades out while the new one starts at once
                const int fadeOut = (int)(rule.fadeOutTime * m_trackParams.sampleRate), fadeIn = (int)(rule.fadeInTime * m_trackParams.sampleRate);
                StopPlayer(m_dying);
                if (m_current.active && fadeOut > 0) {
                    m_dying = m_current;
                    m_current = Player();
                    m_dying.fadeTotal = fadeOut;
                    m_dying.fadePos = 0;
                    m_dying.fadeIn = false;
                } else {
                    StopPlayer(m_current);
                }
                m_currentState = m_newState;
                m_newState = -1;
                StartPlaylist((*m_pStates)[m_currentState], fadeIn);
            } else if (m_current.remaining > 0 && m_current.remaining < chunk) {
                // the old state is left at the end of the segment it plays (the implicit cue of a segment)
                chunk = m_current.remaining;
            }
        }
        std::vector<short> &mix = m_scratch;
        mix.assign((size_t)chunk * ch, 0);
        int made = DecodePlayer(m_current, &mix[0], chunk);
        if (m_current.fadeTotal > 0 && m_current.fadeIn) {
            for (int i = 0; i < made; i++) {
                const float g = m_current.fadePos + i >= m_current.fadeTotal ? 1.0f : (float)(m_current.fadePos + i) / m_current.fadeTotal;
                for (int c = 0; c < ch; c++) mix[i * ch + c] = (short)(mix[i * ch + c] * g);
            }
            m_current.fadePos += made;
            if (m_current.fadePos >= m_current.fadeTotal) m_current.fadeTotal = 0;
        }
        if (m_dying.active) {
            std::vector<short> old((size_t)chunk * ch, 0);
            const int oldMade = DecodePlayer(m_dying, &old[0], chunk);
            for (int i = 0; i < oldMade; i++) {
                const int pos = m_dying.fadePos + i;
                const float g = pos >= m_dying.fadeTotal ? 0.0f : 1.0f - (float)pos / m_dying.fadeTotal;
                for (int c = 0; c < ch; c++) {
                    const int v = mix[i * ch + c] + (int)(old[i * ch + c] * g);
                    mix[i * ch + c] = (short)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
                }
            }
            if (oldMade > made) made = oldMade;
            m_dying.fadePos += chunk;
            if (m_dying.fadePos >= m_dying.fadeTotal || !m_dying.active) StopPlayer(m_dying);
        }
        if (made <= 0) { m_ended = !m_current.active && !m_dying.active; break; }
        memcpy(dst + done * ch, &mix[0], (size_t)made * ch * 2);
        done += made;
        if (made < chunk) break;
    }
    if (!m_current.active && !m_dying.active && m_newState < 0) m_ended = true;
    return done * ch * 2;
}

// a native track goes back to the start of its first state
int DecoderNativeCursor::Seek(unsigned sample) {
    if (m_isDecoderInError || sample != 0) return -1;
    Reset();
    return 0;
}

void DecoderNativeCursor::Reset() {
    if (m_isDecoderInError) return;
    StopPlayer(m_current);
    StopPlayer(m_dying);
    m_newState = -1;
    m_currentState = m_pStates->empty() ? -1 : 0;
    StartPlaylist(m_pStates->empty() ? 0 : (*m_pStates)[0], 0);
}

void DecoderNativeCursor::SetLoop(bool loop) { DecoderCursorInterface::SetLoop(loop); }

// the music is over when its playlist has been played (the loops of the elements are the ones of the file); an emitter that
// loops starts it again
bool DecoderNativeCursor::HasData() {
    if (m_isDecoderInError) return false;
    if (m_ended && m_loop && m_newState < 0) Reset();
    return !m_ended || m_newState >= 0;
}

}  // namespace vox
