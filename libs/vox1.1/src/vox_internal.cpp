// libs/vox1.1/src/vox_internal.cpp
// The objects behind the handles, and the engine that holds them.
#include <algorithm>
#include <chrono>
#include <cmath>
#include "vox_decoder_mpc8.h"
#include "vox_decoder_mswav.h"
#include "vox_decoder_native.h"
#include "vox_decoder_raw.h"
#include "vox_internal.h"

namespace vox {

// the random of the gains and the pitches of the sounds
static float RandomFloat() {
    static unsigned s_seed = 2463534242u;
    s_seed ^= s_seed << 13;
    s_seed ^= s_seed >> 17;
    s_seed ^= s_seed << 5;
    return (float)(s_seed & 0xffffff) / (float)0x1000000;
}

static StreamInterface *StreamCFileFactory(void *params) { return new StreamCFile(static_cast<const char*>(params)); }
static StreamInterface *StreamMemoryBufferFactory(void *params) { return new StreamMemoryBuffer(static_cast<StreamMemoryBufferParams*>(params)); }
static StreamInterface *StreamNativeSegmentFactory(void *params) { return new StreamNativeSegment(static_cast<StreamNativeSegmentParams*>(params)); }

// ---- DataObj

DataObj::DataObj(int64_t id, StreamInterface *stream, DecoderInterface *decoder, int uid, VoxSourceLoadingFlags flags, unsigned groupId)
    : Handlable(id), m_refCount(0), m_groupId(groupId), m_priorityBank(-1), m_uid(uid), m_pStream(stream), m_pDecoder(decoder), m_released(false),
      m_isReady(false), m_loadingFlags(flags) {
    ReadTrackParams();
}

DataObj::~DataObj() {
    delete m_pDecoder;
    delete m_pStream;
}

// the format of the track is what a cursor of the decoder says of it
void DataObj::ReadTrackParams() {
    m_isReady = false;
    if (!m_pStream || !m_pDecoder) return;
    m_pStream->Init();
    StreamCursorInterface *sc = m_pStream->CreateNewCursor();
    if (!sc) return;
    DecoderCursorInterface *dc = m_pDecoder->CreateNewCursor(sc);
    if (dc) {
        m_trackParams = dc->GetTrackParams();
        m_isReady = !dc->IsInError() && m_trackParams.numChannels > 0 && m_trackParams.sampleRate > 0;
        m_pDecoder->DestroyCursor(dc);
    }
    m_pStream->DestroyCursor(sc);
}

void DataObj::SetSource(StreamInterface *stream, DecoderInterface *decoder) {
    if (decoder != m_pDecoder) delete m_pDecoder;
    if (stream != m_pStream) delete m_pStream;
    m_pStream = stream;
    m_pDecoder = decoder;
    ReadTrackParams();
}

// ---- EmitterObj

EmitterObj::EmitterObj(int64_t id, const emitter::CreationSettings &s, DriverSourceInterface *source, StreamCursorInterface *streamCursor, DecoderCursorInterface *cursor, DataObj *data, VoxEngineInternal *engine)
    : Handlable(id), m_groupId(s.m_groupId), m_priority(s.m_priority), m_priorityBank((int)s.m_priorityBankId), m_isInBank(false),
      m_desiredGain(s.m_gain), m_gainModifier(1.0f), m_baseGainFader(s.m_gain, s.m_gain, 0), m_desiredPitch(s.m_pitch), m_pitchModifier(1.0f),
      m_pitchFader(s.m_pitch, s.m_pitch, 0), m_loop(s.m_isLoop), m_currentState(kEmitterInit), m_pauseCount(0), m_killOnResume(s.m_killOnResume),
      m_fadeOnPlay(s.m_fadeOnPlay), m_fadeOnStop(s.m_fadeOnStop), m_is3d(s.m_is3d), m_position(s.m_pPosition), m_direction(s.m_pDirection), m_velocity(s.m_pVelocity),
      m_3Dparameters(s.m_pEmitter3DParameters), m_3DNeedUpdate(true), m_phwSource(source), m_pStreamCursor(streamCursor), m_pDecoderCursor(cursor),
      m_pDataSource(data), m_pEngine(engine), m_needToDie(false), m_autoKill(true), m_uid(s.m_uid), m_stateChangedCallback(s.m_stateChangedCallback),
      m_stateChangedCallbackUserData(s.m_pStateChangedCallbackUserData), m_driverParam(s.m_driverParam), m_decoderFinished(false), m_stopping(false),
      m_pausing(false), m_lastSentState(-1), m_stateSent(false) {
    if (s.m_enableRandomGain) m_gainModifier = s.m_minimumRandomGainRatio + RandomFloat() * (s.m_maximumRandomGainRatio - s.m_minimumRandomGainRatio);
    if (s.m_enableRandomPitch) {
        const float cents = s.m_minimumRandomPitchCents + RandomFloat() * (s.m_maximumRandomPitchCents - s.m_minimumRandomPitchCents);
        m_pitchModifier = powf(2.0f, cents / 1200.0f);
    }
    if (s.m_pEmitterHandleUserData) m_userData = *s.m_pEmitterHandleUserData;
    m_playbackFader = Fader(1, 1, 0);
    if (m_pDecoderCursor) {
        m_pDecoderCursor->SetLoop(m_loop);
        const TrackParams &tp = m_pDecoderCursor->GetTrackParams();
        // whole frames in a buffer
        const int bpf = tp.numChannels * 2 > 0 ? tp.numChannels * 2 : 2;
        m_driverParam.bufferSize = (m_driverParam.bufferSize / bpf) * bpf;
        if (m_phwSource) m_phwSource->Init(tp, m_driverParam);
    }
}

EmitterObj::~EmitterObj() {
    if (m_isInBank && m_pEngine) m_pEngine->GetPriorityBankManager()->RemoveEmitter((unsigned)m_priorityBank, this);
    if (m_pDataSource) {
        if (m_pDecoderCursor) m_pDataSource->GetDecoder()->DestroyCursor(m_pDecoderCursor);
        if (m_pStreamCursor) m_pDataSource->GetStream()->DestroyCursor(m_pStreamCursor);
    }
    if (m_phwSource && m_pEngine && m_pEngine->GetDriver()) m_pEngine->GetDriver()->DestroyDriverSource(m_phwSource);
}

void EmitterObj::SetGain(float gain, float fadeTime) {
    m_desiredGain = gain;
    m_baseGainFader = Fader(m_baseGainFader.GetCurrentValue(), gain, fadeTime);
}

void EmitterObj::SetPitch(float pitch, float fadeTime) {
    m_desiredPitch = pitch;
    m_pitchFader = Fader(m_pitchFader.GetCurrentValue(), pitch, fadeTime);
}

void EmitterObj::SetLoop(bool loop) {
    m_loop = loop;
    if (m_pDecoderCursor) m_pDecoderCursor->SetLoop(loop);
    // a sound that was over because it did not loop reads on
    if (loop && m_decoderFinished && m_currentState == kEmitterPlaying) m_decoderFinished = !m_pDecoderCursor || !m_pDecoderCursor->HasData();
}

void EmitterObj::SetInteractiveMusicStateChange(const char *label) {
    if (m_pDecoderCursor) m_pDecoderCursor->SetInteractiveMusicState(label);
}

// The decoded sound is put in the buffers of the source while it has a free one. A sound that does not loop is over when the
// decoder gives less than it was asked for.
void EmitterObj::Feed() {
    if (!m_pDecoderCursor || !m_phwSource || m_decoderFinished) return;
    const int size = m_driverParam.bufferSize;
    if ((int)m_decodeBuffer.size() < size) m_decodeBuffer.resize(size);
    for (int guard = 0; guard < 8 && !m_decoderFinished && m_phwSource->NeedData(); guard++) {
        const int n = m_pDecoderCursor->Decode(&m_decodeBuffer[0], size);
        if (n > 0) m_phwSource->UploadData(&m_decodeBuffer[0], n);
        if (n <= 0 || (n < size && !m_loop)) m_decoderFinished = true;
    }
}

void EmitterObj::SendStateChange() {
    if (m_lastSentState == m_currentState) return;
    m_lastSentState = m_currentState;
    m_stateSent = true;
    if (!m_stateChangedCallback) return;
    EmitterExternState ext = kExternStateStopped;
    switch (m_currentState) {
        case kEmitterPlaying: ext = kExternStatePlaying; break;
        case kEmitterPaused: ext = kExternStatePaused; break;
        case kEmitterDone: ext = kExternStateDone; break;
        default: break;
    }
    EmitterHandle h(m_id);
    m_stateChangedCallback(h, m_stateChangedCallbackUserData, ext);
}

// the emitter is over: the source is silent and its place in the bank is free
void EmitterObj::Finish(int state) {
    m_currentState = state;
    m_stopping = m_pausing = false;
    if (m_phwSource) m_phwSource->Stop();
    if (m_isInBank && m_pEngine) m_pEngine->GetPriorityBankManager()->RemoveEmitter((unsigned)m_priorityBank, this);
}

void EmitterObj::Kill() {
    Finish(kEmitterStopped);
    m_needToDie = true;
}

void EmitterObj::Play(float fade) {
    if (m_currentState == kEmitterPlaying) return;
    if (m_currentState == kEmitterPaused) { Resume(fade); return; }
    if (m_currentState != kEmitterInit && m_currentState != kEmitterStopped) return;
    if (m_priorityBank >= 0 && !m_pEngine->GetPriorityBankManager()->AddEmitter((unsigned)m_priorityBank, this)) {
        // the bank does not want it: it is as if it was played and over at once
        Finish(kEmitterDone);
        return;
    }
    if (fade <= 0) fade = m_fadeOnPlay;
    m_playbackFader = fade > 0 ? Fader(0, 1, fade) : Fader(1, 1, 0);
    m_stopping = m_pausing = false;
    m_decoderFinished = false;
    m_currentState = m_pauseCount > 0 ? kEmitterPaused : kEmitterPlaying;
    Feed();
    Update3D();
    if (m_phwSource && m_pauseCount <= 0) {
        const float g = m_pEngine->GetGroupManager()->GetEffectiveVolume(m_groupId);
        m_phwSource->SetGain(g * GetAttenuation());
        m_phwSource->SetPitch(m_pitchFader.GetCurrentValue() * m_pitchModifier * m_pEngine->GetGroupManager()->GetEffectivePitch(m_groupId));
        m_phwSource->Play();
    }
}

void EmitterObj::Stop(float fade) {
    if (m_currentState == kEmitterDone || m_currentState == kEmitterStopped) return;
    if (m_currentState == kEmitterInit) { Finish(kEmitterStopped); return; }
    if (fade <= 0) fade = m_fadeOnStop;
    if (fade > 0) {
        m_stopping = true;
        m_playbackFader = Fader(m_playbackFader.GetCurrentValue(), 0, fade);
    } else {
        Finish(kEmitterStopped);
    }
}

void EmitterObj::Pause(float fade) {
    m_pauseCount++;
    if (m_currentState != kEmitterPlaying) return;
    m_currentState = kEmitterPaused;
    if (fade > 0) {
        m_pausing = true;
        m_playbackFader = Fader(m_playbackFader.GetCurrentValue(), 0, fade);
    } else if (m_phwSource) {
        m_phwSource->Pause();
    }
}

void EmitterObj::Resume(float fade) {
    if (m_pauseCount <= 0) return;
    if (--m_pauseCount > 0 || m_currentState != kEmitterPaused) return;
    m_currentState = kEmitterPlaying;
    m_pausing = false;
    m_playbackFader = fade > 0 ? Fader(m_playbackFader.GetCurrentValue(), 1, fade) : Fader(1, 1, 0);
    if (m_phwSource) m_phwSource->Play();
}

float EmitterObj::GetTotalPlayTime() {
    if (!m_phwSource || !m_pDecoderCursor) return 0.0f;
    const TrackParams &tp = m_pDecoderCursor->GetTrackParams();
    if (tp.sampleRate <= 0 || tp.numChannels <= 0) return 0.0f;
    return (float)m_phwSource->GetByteOffset() / (float)(tp.sampleRate * tp.numChannels * 2);
}

// the place in the track: what was played, less the loops it did
float EmitterObj::GetPlayCursor() {
    const float t = GetTotalPlayTime();
    const float d = m_pDataSource ? m_pDataSource->GetDuration() : 0.0f;
    return d > 0.0f && t > d ? fmodf(t, d) : t;
}

void EmitterObj::SetPlayCursor(float seconds) {
    if (!m_pDecoderCursor || !m_phwSource) return;
    const TrackParams &tp = m_pDecoderCursor->GetTrackParams();
    const bool playing = m_currentState == kEmitterPlaying;
    if (m_pDecoderCursor->Seek((unsigned)(seconds * tp.sampleRate)) != 0) return;
    m_phwSource->Reset();
    m_decoderFinished = false;
    Feed();
    if (playing) m_phwSource->Play();
}

void EmitterObj::Set3DParameteri(int id, int v) {
    if (id == k3DEmitterRelativeToListener) m_3Dparameters.relativeToListener = v;
    else { Set3DParameterf(id, (float)v); return; }
    m_3DNeedUpdate = true;
}

void EmitterObj::Set3DParameterf(int id, float v) {
    switch (id) {
        case k3DEmitterMaxDistance: m_3Dparameters.maxDistance = v; break;
        case k3DEmitterReferenceDistance: m_3Dparameters.referenceDistance = v; break;
        case k3DEmitterRolloffFactor: m_3Dparameters.rolloffFactor = v; break;
        case k3DEmitterInnerConeAngle: m_3Dparameters.innerConeAngle = v; break;
        case k3DEmitterOuterConeAngle: m_3Dparameters.outerConeAngle = v; break;
        case k3DEmitterOuterConeGain: m_3Dparameters.outerConeGain = v; break;
        default: return;
    }
    m_3DNeedUpdate = true;
}

void EmitterObj::Set3DParameter3f(int id, float x, float y, float z) {
    switch (id) {
        case k3DEmitterPosition: m_position = VoxVector3f(x, y, z); break;
        case k3DEmitterVelocity: m_velocity = VoxVector3f(x, y, z); break;
        case k3DEmitterDirection: m_direction = VoxVector3f(x, y, z); break;
        default: return;
    }
    m_3DNeedUpdate = true;
}

void EmitterObj::Set3DParameters(const Vox3DEmitterParameters &p) {
    m_3Dparameters = p;
    m_3DNeedUpdate = true;
}

void EmitterObj::Get3DParameteri(int id, int &v) const {
    if (id == k3DEmitterRelativeToListener) v = m_3Dparameters.relativeToListener;
    else { float f = 0; Get3DParameterf(id, f); v = (int)f; }
}

void EmitterObj::Get3DParameterf(int id, float &v) const {
    switch (id) {
        case k3DEmitterMaxDistance: v = m_3Dparameters.maxDistance; break;
        case k3DEmitterReferenceDistance: v = m_3Dparameters.referenceDistance; break;
        case k3DEmitterRolloffFactor: v = m_3Dparameters.rolloffFactor; break;
        case k3DEmitterInnerConeAngle: v = m_3Dparameters.innerConeAngle; break;
        case k3DEmitterOuterConeAngle: v = m_3Dparameters.outerConeAngle; break;
        case k3DEmitterOuterConeGain: v = m_3Dparameters.outerConeGain; break;
        default: break;
    }
}

void EmitterObj::Get3DParameter3f(int id, float &x, float &y, float &z) const {
    const VoxVector3f *p = id == k3DEmitterPosition ? &m_position : id == k3DEmitterVelocity ? &m_velocity : id == k3DEmitterDirection ? &m_direction : 0;
    if (p) { x = p->x; y = p->y; z = p->z; }
}

// the 3D parameters go to the source of the driver, for the sounds that are 3D
void EmitterObj::Update3D() {
    if (!m_is3d || !m_3DNeedUpdate || !m_phwSource) return;
    m_3DNeedUpdate = false;
    const float pos[3] = { m_position.x, m_position.y, m_position.z };
    const float vel[3] = { m_velocity.x, m_velocity.y, m_velocity.z };
    const float dir[3] = { m_direction.x, m_direction.y, m_direction.z };
    m_phwSource->Set3DParameter(k3DEmitterRelativeToListener, &m_3Dparameters.relativeToListener);
    m_phwSource->Set3DParameter(k3DEmitterMaxDistance, &m_3Dparameters.maxDistance);
    m_phwSource->Set3DParameter(k3DEmitterReferenceDistance, &m_3Dparameters.referenceDistance);
    m_phwSource->Set3DParameter(k3DEmitterRolloffFactor, &m_3Dparameters.rolloffFactor);
    m_phwSource->Set3DParameter(k3DEmitterInnerConeAngle, &m_3Dparameters.innerConeAngle);
    m_phwSource->Set3DParameter(k3DEmitterOuterConeAngle, &m_3Dparameters.outerConeAngle);
    m_phwSource->Set3DParameter(k3DEmitterOuterConeGain, &m_3Dparameters.outerConeGain);
    m_phwSource->Set3DParameter(k3DEmitterPosition, pos);
    m_phwSource->Set3DParameter(k3DEmitterVelocity, vel);
    m_phwSource->Set3DParameter(k3DEmitterDirection, dir);
}

void EmitterObj::Update(float dt) {
    m_baseGainFader.Update(dt);
    m_pitchFader.Update(dt);
    m_playbackFader.Update(dt);
    switch (m_currentState) {
        case kEmitterPlaying:
            Feed();
            if (m_stopping && m_playbackFader.IsFinished()) Finish(kEmitterStopped);
            else if (m_decoderFinished && m_phwSource && m_phwSource->GetQueuedBytes() <= 0) Finish(kEmitterDone);
            break;
        case kEmitterPaused:
            if (m_pausing && m_playbackFader.IsFinished()) {
                m_pausing = false;
                if (m_phwSource) m_phwSource->Pause();
            }
            break;
        default:
            break;
    }
    if (m_phwSource && (m_currentState == kEmitterPlaying || m_currentState == kEmitterPaused)) {
        GroupManager *gm = m_pEngine->GetGroupManager();
        m_phwSource->SetGain(gm->GetEffectiveVolume(m_groupId) * GetAttenuation());
        m_phwSource->SetPitch(m_pitchFader.GetCurrentValue() * m_pitchModifier * gm->GetEffectivePitch(m_groupId));
        Update3D();
    }
    SendStateChange();
}

// ---- VoxEngineInternal

VoxEngineInternal *VoxEngineInternal::s_voxEngineInternal = 0;

VoxEngineInternal::VoxEngineInternal() : m_outputMode(kOutputDefault), m_hwDriver(0), m_masterGain(1, 1, 0), m_suspendCount(0), m_nextId(0) {}

VoxEngineInternal::~VoxEngineInternal() { Shutdown(); }

VoxEngineInternal *VoxEngineInternal::GetVoxEngineInternal() {
    if (!s_voxEngineInternal) s_voxEngineInternal = new VoxEngineInternal();
    return s_voxEngineInternal;
}

void VoxEngineInternal::Destroy() {
    VoxEngineInternal *e = s_voxEngineInternal;
    s_voxEngineInternal = 0;
    delete e;
}

// @0x5de818: the types of streams and of decoders are registered in the order the soundpack's formats number them, and the
// driver of the platform is started (the stb_vorbis decoder (2) and the bcwav one (5) are not in this build: the game has no
// sound of those formats)
void VoxEngineInternal::Initialize() {
    ScopedLock lock(m_mutex);
    if (m_hwDriver) return;
    RegisterStreamType(StreamCFileFactory);
    RegisterStreamType(StreamMemoryBufferFactory);
    RegisterStreamType(StreamNativeSegmentFactory);
    RegisterDecoderType(DecoderRawFactory);
    RegisterDecoderType(DecoderMSWavFactory);
    RegisterDecoderType(0);
    RegisterDecoderType(DecoderMPC8Factory);
    RegisterDecoderType(DecoderNativeFactory);
    m_hwDriver = CreateDefaultDriver();
    if (m_hwDriver) m_hwDriver->Init(0);
    _SetDefault3DParameters();
}

void VoxEngineInternal::_SetDefault3DParameters() {
    m_3Dparameters = Vox3DGeneralParameters();
    m_listener = ListenerParameters();
}

void VoxEngineInternal::Shutdown() {
    {
        ScopedLock lock(m_mutex);
        for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it) delete it->second;
        m_emitterObjects.clear();
        for (std::map<int64_t, DataObj*>::iterator it = m_dataObjects.begin(); it != m_dataObjects.end(); ++it) delete it->second;
        m_dataObjects.clear();
    }
    if (m_hwDriver) {
        m_hwDriver->Shutdown();
        delete m_hwDriver;
        m_hwDriver = 0;
    }
}

void VoxEngineInternal::Suspend() {
    ScopedLock lock(m_mutex);
    if (m_suspendCount++ == 0 && m_hwDriver) m_hwDriver->Suspend();
}

// the sounds that are marked to be killed when the engine resumes (the music of a game that was in the background) stop
void VoxEngineInternal::Resume() {
    ScopedLock lock(m_mutex);
    if (m_suspendCount > 0 && --m_suspendCount == 0) {
        KillEmittersOnResume();
        if (m_hwDriver) m_hwDriver->Resume();
    }
}

void VoxEngineInternal::KillEmittersOnResume() {
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it)
        if (it->second->GetKillOnResume()) it->second->Kill();
}

int VoxEngineInternal::RegisterStreamType(StreamInterface *(*factory)(void*)) {
    m_streamTypes.push_back(factory);
    return (int)m_streamTypes.size() - 1;
}

int VoxEngineInternal::RegisterDecoderType(DecoderInterface *(*factory)(void*)) {
    m_decoderTypes.push_back(factory);
    return (int)m_decoderTypes.size() - 1;
}

StreamInterface *VoxEngineInternal::CreateStream(int type, void *params) {
    return type >= 0 && type < (int)m_streamTypes.size() && m_streamTypes[type] ? m_streamTypes[type](params) : 0;
}

DecoderInterface *VoxEngineInternal::CreateDecoder(int type, void *params) {
    return type >= 0 && type < (int)m_decoderTypes.size() && m_decoderTypes[type] ? m_decoderTypes[type](params) : 0;
}

// all the sound of a data source, decoded
static bool DecodeAll(DataObj *d, std::vector<unsigned char> &out) {
    StreamCursorInterface *sc = d->GetStream()->CreateNewCursor();
    if (!sc) return false;
    DecoderCursorInterface *dc = d->GetDecoder()->CreateNewCursor(sc);
    if (!dc) { d->GetStream()->DestroyCursor(sc); return false; }
    dc->SetLoop(false);
    unsigned char buf[16384];
    const size_t limit = 64u << 20;
    for (;;) {
        const int n = dc->Decode(buf, sizeof(buf));
        if (n <= 0) break;
        out.insert(out.end(), buf, buf + n);
        if (out.size() > limit) break;
    }
    d->GetDecoder()->DestroyCursor(dc);
    d->GetStream()->DestroyCursor(sc);
    return !out.empty();
}

static bool ReadAll(DataObj *d, std::vector<unsigned char> &out) {
    StreamCursorInterface *sc = d->GetStream()->CreateNewCursor();
    if (!sc) return false;
    unsigned char buf[16384];
    for (;;) {
        const int n = sc->Read(buf, sizeof(buf));
        if (n <= 0) break;
        out.insert(out.end(), buf, buf + n);
    }
    d->GetStream()->DestroyCursor(sc);
    return !out.empty();
}

static StreamInterface *MakeMemoryStream(const std::vector<unsigned char> &bytes) {
    StreamMemoryBufferParams p;
    p.buffer = (unsigned char*)malloc(bytes.size());
    memcpy(p.buffer, &bytes[0], bytes.size());
    p.size = (int)bytes.size();
    p.ownsBuffer = true;
    return new StreamMemoryBuffer(&p);
}

// a sound is loaded: its stream and its decoder are made, and as the flags say it is read to memory ("load to ram") or decoded
// to memory ("load and decode"); a native track plays by its states, so it cannot be decoded once for all
DataHandle VoxEngineInternal::LoadDataSource(const data_source::CreationSettings &s) {
    ScopedLock lock(m_mutex);
    StreamInterface *stream = CreateStream(s.m_streamType, s.m_pStreamParams);
    DecoderInterface *decoder = CreateDecoder(s.m_decoderType, s.m_pDecoderParams);
    if (!stream || !decoder) {
        delete stream;
        delete decoder;
        return DataHandle();
    }
    DataObj *d = new DataObj(GetFreeId(), stream, decoder, s.m_uid, s.m_loadingFlags, s.m_groupId);
    if (!d->IsReady()) {
        delete d;
        return DataHandle();
    }
    if (s.m_loadingFlags == kLoadAndDecode && decoder->GetType() != kFormatVxn) {
        std::vector<unsigned char> pcm;
        if (DecodeAll(d, pcm)) {
            TrackParams tp = d->GetTrackParams();
            tp.numSamples = (int)(pcm.size() / (tp.numChannels * 2));
            d->SetSource(MakeMemoryStream(pcm), new DecoderRaw(&tp));
        }
    } else if (s.m_loadingFlags != kLoadNone) {
        std::vector<unsigned char> bytes;
        if (ReadAll(d, bytes)) d->SetSource(MakeMemoryStream(bytes), d->GetDecoder());
    }
    m_dataObjects[d->GetId()] = d;
    return DataHandle(d->GetId());
}

DataHandle VoxEngineInternal::ConvertToRawSource(DataHandle &h) {
    ScopedLock lock(m_mutex);
    DataObj *d = GetDataObject(h);
    if (!d || d->GetDecoder()->GetType() == kFormatVxn) return DataHandle();
    std::vector<unsigned char> pcm;
    if (!DecodeAll(d, pcm)) return DataHandle();
    TrackParams tp = d->GetTrackParams();
    tp.numSamples = (int)(pcm.size() / (tp.numChannels * 2));
    DataObj *n = new DataObj(GetFreeId(), MakeMemoryStream(pcm), new DecoderRaw(&tp), d->GetUid(), d->GetLoadingFlags(), d->GetGroup());
    m_dataObjects[n->GetId()] = n;
    return DataHandle(n->GetId());
}

DataHandle VoxEngineInternal::ConvertToRamBufferSource(DataHandle &h) {
    ScopedLock lock(m_mutex);
    DataObj *d = GetDataObject(h);
    if (!d) return DataHandle();
    std::vector<unsigned char> bytes;
    if (!ReadAll(d, bytes)) return DataHandle();
    DecoderInterface *dec = CreateDecoder(d->GetDecoder()->GetType(), d->GetDecoder()->GetParam());
    if (!dec) return DataHandle();
    DataObj *n = new DataObj(GetFreeId(), MakeMemoryStream(bytes), dec, d->GetUid(), kLoadToRam, d->GetGroup());
    m_dataObjects[n->GetId()] = n;
    return DataHandle(n->GetId());
}

DataObj *VoxEngineInternal::GetDataObject(const DataHandle &h) {
    ScopedLock lock(m_mutex);
    std::map<int64_t, DataObj*>::iterator it = m_dataObjects.find(h.GetId());
    return it == m_dataObjects.end() ? 0 : it->second;
}

void VoxEngineInternal::IncreaseDataObjectRefCount(const DataHandle &h) {
    ScopedLock lock(m_mutex);
    if (DataObj *d = GetDataObject(h)) d->m_refCount++;
}

void VoxEngineInternal::DecreaseDataObjectRefCount(const DataHandle &h) {
    ScopedLock lock(m_mutex);
    if (DataObj *d = GetDataObject(h)) d->m_refCount--;
}

int VoxEngineInternal::GetAllDataSources(DataHandle *out, int max) {
    ScopedLock lock(m_mutex);
    int n = 0;
    for (std::map<int64_t, DataObj*>::iterator it = m_dataObjects.begin(); it != m_dataObjects.end() && n < max; ++it) out[n++] = DataHandle(it->first);
    return n;
}

// the data source goes when its emitters are over
void VoxEngineInternal::ReleaseDatasource(DataHandle &h) {
    ScopedLock lock(m_mutex);
    if (DataObj *d = GetDataObject(h)) d->Release();
}

void VoxEngineInternal::ReleaseDatasourceGroup(unsigned group) {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, DataObj*>::iterator it = m_dataObjects.begin(); it != m_dataObjects.end(); ++it)
        if (m_groupManager.IsChild(it->second->GetGroup(), group)) it->second->Release();
}

bool VoxEngineInternal::IsValid(const DataHandle &h) { return GetDataObject(h) != 0; }

bool VoxEngineInternal::IsValid(const EmitterHandle &h) { return GetEmitterObject(h) != 0; }

int VoxEngineInternal::GetEmitterHandles(DataHandle &h, EmitterHandle *out, int max) {
    ScopedLock lock(m_mutex);
    DataObj *d = GetDataObject(h);
    int n = 0;
    if (d)
        for (std::list<int64_t>::iterator it = d->GetRegisteredEmitters().begin(); it != d->GetRegisteredEmitters().end() && n < max; ++it) out[n++] = EmitterHandle(*it);
    return n;
}

EmitterObj *VoxEngineInternal::GetEmitterObject(const EmitterHandle &h) {
    ScopedLock lock(m_mutex);
    std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.find(h.GetId());
    return it == m_emitterObjects.end() ? 0 : it->second;
}

EmitterHandle VoxEngineInternal::CreateEmitter(DataHandle &data, const emitter::CreationSettings &s) {
    ScopedLock lock(m_mutex);
    DataObj *d = GetDataObject(data);
    if (!d || !d->IsReady() || !m_hwDriver) return EmitterHandle();
    StreamCursorInterface *sc = d->GetStream()->CreateNewCursor();
    DecoderCursorInterface *dc = sc ? d->GetDecoder()->CreateNewCursor(sc) : 0;
    if (!dc || dc->IsInError()) {
        if (dc) d->GetDecoder()->DestroyCursor(dc);
        if (sc) d->GetStream()->DestroyCursor(sc);
        return EmitterHandle();
    }
    DriverSourceInterface *source = m_hwDriver->CreateDriverSource();
    emitter::CreationSettings cs = s;
    if (cs.m_priorityBankId == (unsigned)-1 && d->GetPriorityBank() >= 0) cs.m_priorityBankId = (unsigned)d->GetPriorityBank();
    EmitterObj *e = new EmitterObj(GetFreeId(), cs, source, sc, dc, d, this);
    d->RegisterEmitter(e->GetId());
    m_emitterObjects[e->GetId()] = e;
    return EmitterHandle(e->GetId());
}

void VoxEngineInternal::KillEmitter(EmitterObj *e) {
    if (e->GetDataSource()) e->GetDataSource()->UnregisterEmitter(e->GetId());
    m_emitterObjects.erase(e->GetId());
    delete e;
}

void VoxEngineInternal::ReleaseEmitter(EmitterHandle &h) {
    ScopedLock lock(m_mutex);
    if (EmitterObj *e = GetEmitterObject(h)) KillEmitter(e);
}

int VoxEngineInternal::GetAllEmitters(EmitterHandle *out, int max) {
    ScopedLock lock(m_mutex);
    int n = 0;
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end() && n < max; ++it) out[n++] = EmitterHandle(it->first);
    return n;
}

// The UpdateSources thread: the data sources nobody holds any more (and that no emitter plays) are freed
void VoxEngineInternal::UpdateSources() {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, DataObj*>::iterator it = m_dataObjects.begin(); it != m_dataObjects.end();) {
        if (it->second->ShouldDie()) {
            delete it->second;
            m_dataObjects.erase(it++);
        } else {
            ++it;
        }
    }
}

// the UpdateEmitters thread: the groups and the master gain fade, every emitter is updated, and those that are over go
void VoxEngineInternal::UpdateEmitters(float dt) {
    ScopedLock lock(m_mutex);
    if (m_suspendCount > 0) return;
    m_groupManager.Update(dt);
    m_masterGain.Update(dt);
    if (m_hwDriver) m_hwDriver->SetMasterGain(m_masterGain.GetCurrentValue());
    std::vector<EmitterObj*> dead;
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it) {
        it->second->Update(dt);
        if (it->second->ShouldDie()) dead.push_back(it->second);
    }
    for (size_t i = 0; i < dead.size(); i++) KillEmitter(dead[i]);
}

void VoxEngineInternal::PlayGroup(unsigned group, float fade) {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it)
        if (m_groupManager.IsChild((unsigned)it->second->GetGroup(), group)) it->second->Play(fade);
}

void VoxEngineInternal::StopGroup(unsigned group, float fade) {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it)
        if (m_groupManager.IsChild((unsigned)it->second->GetGroup(), group)) it->second->Stop(fade);
}

void VoxEngineInternal::PauseGroup(unsigned group, float fade) {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it)
        if (m_groupManager.IsChild((unsigned)it->second->GetGroup(), group)) it->second->Pause(fade);
}

void VoxEngineInternal::ResumeGroup(unsigned group, float fade) {
    ScopedLock lock(m_mutex);
    for (std::map<int64_t, EmitterObj*>::iterator it = m_emitterObjects.begin(); it != m_emitterObjects.end(); ++it)
        if (m_groupManager.IsChild((unsigned)it->second->GetGroup(), group)) it->second->Resume(fade);
}

void VoxEngineInternal::SetMasterGain(float gain, float fade) {
    ScopedLock lock(m_mutex);
    m_masterGain = Fader(m_masterGain.GetCurrentValue(), gain, fade);
}

void VoxEngineInternal::Set3DListenerPosition(float x, float y, float z) {
    ScopedLock lock(m_mutex);
    m_listener.position = VoxVector3f(x, y, z);
    if (m_hwDriver) { const float v[3] = { x, y, z }; m_hwDriver->Set3DParameter(100, v); }
}

void VoxEngineInternal::Set3DListenerVelocity(float x, float y, float z) {
    ScopedLock lock(m_mutex);
    m_listener.velocity = VoxVector3f(x, y, z);
    if (m_hwDriver) { const float v[3] = { x, y, z }; m_hwDriver->Set3DParameter(101, v); }
}

void VoxEngineInternal::Set3DListenerOrientation(float fx, float fy, float fz, float ux, float uy, float uz) {
    ScopedLock lock(m_mutex);
    m_listener.lookAt = VoxVector3f(fx, fy, fz);
    m_listener.up = VoxVector3f(ux, uy, uz);
    if (m_hwDriver) { const float v[6] = { fx, fy, fz, ux, uy, uz }; m_hwDriver->Set3DParameter(102, v); }
}

void VoxEngineInternal::Set3DGeneralParameter(const Vox3DGeneralParameters &p) {
    ScopedLock lock(m_mutex);
    m_3Dparameters = p;
    if (!m_hwDriver) return;
    m_hwDriver->Set3DParameter(k3DGeneralDopplerFactor, &p.dopplerFactor);
    m_hwDriver->Set3DParameter(k3DGeneralSpeedOfSound, &p.speedOfSound);
    m_hwDriver->Set3DParameter(k3DGeneralDistanceModel, &p.distanceModel);
    m_hwDriver->Set3DParameter(k3DGeneralEnhanced3d, &p.enhanced3d);
}

void VoxEngineInternal::Set3DGeneralParameterf(int id, float v) {
    ScopedLock lock(m_mutex);
    if (id == k3DGeneralDopplerFactor) m_3Dparameters.dopplerFactor = v;
    else if (id == k3DGeneralSpeedOfSound) m_3Dparameters.speedOfSound = v;
    else return;
    if (m_hwDriver) m_hwDriver->Set3DParameter(id, &v);
}

void VoxEngineInternal::Set3DGeneralParameteri(int id, int v) {
    ScopedLock lock(m_mutex);
    const unsigned u = (unsigned)v;
    if (id == k3DGeneralDistanceModel) m_3Dparameters.distanceModel = u;
    else if (id == k3DGeneralEnhanced3d) m_3Dparameters.enhanced3d = u;
    else return;
    if (m_hwDriver) m_hwDriver->Set3DParameter(id, &u);
}

}  // namespace vox
