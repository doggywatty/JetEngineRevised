// libs/vox1.1/src/vox.cpp
// VoxEngine: every call takes the engine's lock, finds the object of the handle and does it. A handle that is not valid (the
// emitter is over, the data source was released) is ignored, and the getters give a neutral value.
#include <chrono>
#include <cstdio>
#include "vox_internal.h"

namespace vox {

namespace emitter {
CreationSettings::CreationSettings()
    : m_gain(1.0f), m_minimumRandomGainRatio(1.0f), m_maximumRandomGainRatio(1.0f), m_enableRandomGain(false), m_pitch(1.0f), m_minimumRandomPitchCents(0.0f),
      m_maximumRandomPitchCents(0.0f), m_enableRandomPitch(false), m_isLoop(false), m_groupId((unsigned)-1), m_priorityBankId((unsigned)-1), m_priority(0), m_is3d(false),
      m_busName(0), m_killOnResume(false), m_fadeOnPlay(0.0f), m_fadeOnStop(0.0f), m_uid(-1), m_pEmitterHandleUserData(0), m_stateChangedCallback(0),
      m_pStateChangedCallbackUserData(0), m_numCustomSound(0), m_customSoundStr(0) {}

CreationSettings::CreationSettings(float gain, float pitch, unsigned groupId, int priority, unsigned priorityBankId) {
    *this = CreationSettings();
    m_gain = gain;
    m_pitch = pitch;
    m_groupId = groupId;
    m_priority = priority;
    m_priorityBankId = priorityBankId;
}

void CreationSettings::SetRandomGain(float minimumRatio, float maximumRatio, bool enable) {
    m_minimumRandomGainRatio = minimumRatio;
    m_maximumRandomGainRatio = maximumRatio;
    m_enableRandomGain = enable;
}

void CreationSettings::SetRandomPitch(float minimumCents, float maximumCents, bool enable) {
    m_minimumRandomPitchCents = minimumCents;
    m_maximumRandomPitchCents = maximumCents;
    m_enableRandomPitch = enable;
}
}  // namespace emitter

// the handles of the data sources count on the objects they stand for (the engine frees a data source nobody holds)
DataHandle::DataHandle() : Handle(0) {}

DataHandle::DataHandle(int64_t id) : Handle(id) {
    if (VoxEngineInternal *e = VoxEngineInternal::Peek()) e->IncreaseDataObjectRefCount(*this);
}

DataHandle::DataHandle(const DataHandle &o) : Handle(o.m_id) {
    if (m_id)
        if (VoxEngineInternal *e = VoxEngineInternal::Peek()) e->IncreaseDataObjectRefCount(*this);
}

DataHandle &DataHandle::operator=(const DataHandle &o) {
    if (this == &o) return *this;
    VoxEngineInternal *e = VoxEngineInternal::Peek();
    if (m_id && e) e->DecreaseDataObjectRefCount(*this);
    m_id = o.m_id;
    if (m_id && e) e->IncreaseDataObjectRefCount(*this);
    return *this;
}

DataHandle::~DataHandle() {
    if (m_id)
        if (VoxEngineInternal *e = VoxEngineInternal::Peek()) e->DecreaseDataObjectRefCount(*this);
}

VoxEngine *VoxEngine::s_voxEngine = 0;
VoxEngineInternal *VoxEngine::m_internal = 0;

#define VOX_LOCK ScopedLock vox_lock(m_internal->GetMutex())
#define VOX_EMITTER(h) EmitterObj *e = m_internal->GetEmitterObject(h)
#define VOX_DATA(h) DataObj *d = m_internal->GetDataObject(h)

VoxEngine::VoxEngine() : m_mutex(0), m_isInitialized(false) {
    m_updateThread[0] = m_updateThread[1] = 0;
    m_internal = VoxEngineInternal::GetVoxEngineInternal();
    m_mutex = &m_internal->GetMutex();
}

VoxEngine::~VoxEngine() { Shutdown(); }

// @0x5de6a8
VoxEngine &VoxEngine::GetVoxEngine() {
    if (!s_voxEngine) s_voxEngine = new VoxEngine();
    return *s_voxEngine;
}

void VoxEngine::DestroyVoxEngine() {
    VoxEngine *e = s_voxEngine;
    s_voxEngine = 0;
    delete e;
}

// the time since the last call of a thread, in seconds
static float ElapsedSince(std::chrono::steady_clock::time_point &last) {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const float dt = std::chrono::duration<float>(now - last).count();
    last = now;
    return dt;
}

static void UpdateEmittersThreaded(void *engine, void *) {
    static std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
    static_cast<VoxEngine*>(engine)->UpdateEmitters(ElapsedSince(last));
}

static void UpdateSourcesThreaded(void *engine, void *) { static_cast<VoxEngine*>(engine)->UpdateSources(); }

// @0x5de818: the engine, its driver and the two threads that update the emitters and the data sources
void VoxEngine::Initialize() {
    if (m_isInitialized) return;
    m_isInitialized = true;
    m_internal->Initialize();
    m_updateThread[0] = new VoxThread(UpdateEmittersThreaded, this, 0, 5, "VoxEngine::UpdateEmitters");
    m_updateThread[1] = new VoxThread(UpdateSourcesThreaded, this, 0, 50, "VoxEngine::UpdateSources");
}

void VoxEngine::Shutdown() {
    if (!m_isInitialized) return;
    m_isInitialized = false;
    for (int i = 0; i < 2; i++) {
        delete m_updateThread[i];
        m_updateThread[i] = 0;
    }
    m_internal->Shutdown();
}

int VoxEngine::RegisterStreamType(StreamInterface *(*factory)(void*)) { VOX_LOCK; return m_internal->RegisterStreamType(factory); }

int VoxEngine::RegisterDecoderType(DecoderInterface *(*factory)(void*)) { VOX_LOCK; return m_internal->RegisterDecoderType(factory); }

unsigned VoxEngine::AddPriorityBank(priority_bank::CreationSettings &s) { VOX_LOCK; return m_internal->GetPriorityBankManager()->AddPriorityBank(s); }

bool VoxEngine::ReconfigurePriorityBank(unsigned id, priority_bank::CreationSettings &s) { VOX_LOCK; return m_internal->GetPriorityBankManager()->SetPriorityBank(id, s); }

// @0x5dea68
unsigned VoxEngine::GetPriorityBankIdFromName(const char *name) { VOX_LOCK; return m_internal->GetPriorityBankManager()->GetPriorityBankId(name); }

void VoxEngine::SuspendEngine() { m_internal->Suspend(); }

void VoxEngine::ResumeEngine() { m_internal->Resume(); }

bool VoxEngine::IsEngineSuspended() { return m_internal->IsSuspended(); }

void VoxEngine::UpdateSources() { m_internal->UpdateSources(); }

void VoxEngine::UpdateEmitters(float dt) { m_internal->UpdateEmitters(dt); }

// both updates at once, for an engine that is driven by the game and not by its threads
void VoxEngine::Update(float dt) {
    m_internal->UpdateEmitters(dt);
    m_internal->UpdateSources();
}

DataHandle VoxEngine::LoadDataSource(int streamType, void *streamParams, int decoderType, void *decoderParams, const char *) {
    return m_internal->LoadDataSource(data_source::CreationSettings(streamType, streamParams, decoderType, decoderParams, kLoadNone));
}

DataHandle VoxEngine::LoadDataSource(int streamType, void *streamParams, int decoderType, void *decoderParams, unsigned groupId) {
    data_source::CreationSettings s(streamType, streamParams, decoderType, decoderParams, kLoadNone);
    s.m_groupId = groupId;
    return m_internal->LoadDataSource(s);
}

// @0x5deb14
DataHandle VoxEngine::LoadDataSource(const data_source::CreationSettings &s) { return m_internal->LoadDataSource(s); }

// the sound is loaded at once (the file system of the game is not asynchronous)
DataHandle VoxEngine::LoadDataSourceAsync(const data_source::CreationSettings &s) { return m_internal->LoadDataSource(s); }

DataHandle VoxEngine::ConvertToRawSource(DataHandle &h) { return m_internal->ConvertToRawSource(h); }

DataHandle VoxEngine::ConvertToRamBufferSource(DataHandle &h) { return m_internal->ConvertToRamBufferSource(h); }

void VoxEngine::SetUserData(EmitterHandle &h, EmitterHandleUserData &u) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetUserData(u); }

void VoxEngine::SetUserData(DataHandle &h, DataHandleUserData &u) { VOX_LOCK; VOX_DATA(h); if (d) d->SetUserData(u); }

EmitterHandleUserData VoxEngine::GetUserData(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetUserData() : EmitterHandleUserData(); }

DataHandleUserData VoxEngine::GetUserData(DataHandle &h) { VOX_LOCK; VOX_DATA(h); return d ? d->GetUserData() : DataHandleUserData(); }

void VoxEngine::SetPriorityBankId(EmitterHandle &h, unsigned id) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetPriorityBank((int)id); }

void VoxEngine::SetPriorityBankId(DataHandle &h, unsigned id) { VOX_LOCK; VOX_DATA(h); if (d) d->SetPriorityBank((int)id); }

void VoxEngine::SetUid(DataHandle &h, int uid) { VOX_LOCK; VOX_DATA(h); if (d) d->SetUid(uid); }

int VoxEngine::GetUid(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetUid() : -1; }

// @0x5defc4
int VoxEngine::GetUid(DataHandle &h) { VOX_LOCK; VOX_DATA(h); return d ? d->GetUid() : -1; }

void VoxEngine::ReleaseDatasource(DataHandle &h) { m_internal->ReleaseDatasource(h); }

void VoxEngine::ReleaseDatasourceGroup(const char *name) { VOX_LOCK; m_internal->ReleaseDatasourceGroup(m_internal->GetGroupManager()->GetGroupId(name)); }

void VoxEngine::ReleaseDatasourceGroup(unsigned group) { m_internal->ReleaseDatasourceGroup(group); }

bool VoxEngine::IsReady(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e && e->IsReady(); }

bool VoxEngine::IsReady(DataHandle &h) { VOX_LOCK; VOX_DATA(h); return d && d->IsReady(); }

// @0x5dff08 (the one of SoundMgr::Update, which forgets the sounds that are over)
bool VoxEngine::IsValid(EmitterHandle &h) { return m_internal->IsValid(h); }

bool VoxEngine::IsValid(DataHandle &h) { return m_internal->IsValid(h); }

float VoxEngine::GetDuration(DataHandle &h) { VOX_LOCK; VOX_DATA(h); return d ? d->GetDuration() : 0.0f; }

// @0x5df0c4
int VoxEngine::GetEmitterHandles(DataHandle &h, EmitterHandle *out, int max) { return m_internal->GetEmitterHandles(h, out, max); }

int VoxEngine::GetAllDataSources(DataHandle *out, int max) { return m_internal->GetAllDataSources(out, max); }

void VoxEngine::RegisterForEmitterStateChangeNotification(EmitterHandle &h, void (*cb)(EmitterHandle&, void*, EmitterExternState), void *user) {
    VOX_LOCK; VOX_EMITTER(h); if (e) e->RegisterStateChangedCallback(cb, user);
}

void VoxEngine::UnregisterForEmitterStateChangeNotification(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); if (e) e->UnregisterStateChangedCallback(); }

EmitterHandle VoxEngine::CreateEmitter(DataHandle &h, int priority, void *userData) {
    emitter::CreationSettings s;
    s.m_priority = priority;
    EmitterHandleUserData u;
    u.data = userData;
    s.m_pEmitterHandleUserData = &u;
    VOX_LOCK;
    VOX_DATA(h);
    if (d) s.m_groupId = d->GetGroup();
    return m_internal->CreateEmitter(h, s);
}

// @0x5df1e0
EmitterHandle VoxEngine::CreateEmitter(DataHandle &h, const emitter::CreationSettings &s) { return m_internal->CreateEmitter(h, s); }

EmitterHandle VoxEngine::CreateEmitterAsync(DataHandle &h, const emitter::CreationSettings &s) { return m_internal->CreateEmitter(h, s); }

void VoxEngine::ReleaseEmitter(EmitterHandle &h) { m_internal->ReleaseEmitter(h); }

DataHandle VoxEngine::GetData(EmitterHandle &h) {
    VOX_LOCK;
    VOX_EMITTER(h);
    return e && e->GetDataSource() ? DataHandle(e->GetDataSource()->GetId()) : DataHandle();
}

void VoxEngine::Play(EmitterHandle &h) { Play(h, 0.0f); }

void VoxEngine::Play(EmitterHandle &h, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Play(fade); }

void VoxEngine::Play(EmitterHandle &h, bool loop) { Play(h, loop, 0.0f); }

// @0x5df6f4: the sound loops or not, and starts, fading in over the time given
void VoxEngine::Play(EmitterHandle &h, bool loop, float fade) {
    VOX_LOCK;
    VOX_EMITTER(h);
    if (!e) return;
    e->SetLoop(loop);
    e->Play(fade);
}

void VoxEngine::Stop(EmitterHandle &h) { Stop(h, 0.0f); }

void VoxEngine::Stop(EmitterHandle &h, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Stop(fade); }

void VoxEngine::Pause(EmitterHandle &h) { Pause(h, 0.0f); }

void VoxEngine::Pause(EmitterHandle &h, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Pause(fade); }

void VoxEngine::Resume(EmitterHandle &h) { Resume(h, 0.0f); }

void VoxEngine::Resume(EmitterHandle &h, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Resume(fade); }

float VoxEngine::GetPlayCursor(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetPlayCursor() : 0.0f; }

float VoxEngine::GetTotalPlayTime(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetTotalPlayTime() : 0.0f; }

void VoxEngine::SetPlayCursor(EmitterHandle &h, float t) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetPlayCursor(t); }

// @0x5dfec8
bool VoxEngine::IsAlive(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e && e->IsAlive(); }

bool VoxEngine::IsPlaying(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e && e->IsPlaying(); }

bool VoxEngine::IsDone(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return !e || e->IsDone(); }

int VoxEngine::GetPauseCount(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetPauseCount() : 0; }

unsigned VoxEngine::GetStatus(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetStatus() : (unsigned)kEmitterDone; }

unsigned VoxEngine::AddGroup(group::CreationSettings &s) { VOX_LOCK; return m_internal->GetGroupManager()->AddGroup(s); }

bool VoxEngine::ReconfigureGroup(const char *name, group::CreationSettings &s) { VOX_LOCK; return m_internal->GetGroupManager()->ReconfigureGroup(m_internal->GetGroupManager()->GetGroupId(name), s); }

bool VoxEngine::ReconfigureGroup(unsigned id, group::CreationSettings &s) { VOX_LOCK; return m_internal->GetGroupManager()->ReconfigureGroup(id, s); }

// @0x5dfc40
unsigned VoxEngine::GetGroupId(const char *name) { VOX_LOCK; return m_internal->GetGroupManager()->GetGroupId(name); }

bool VoxEngine::GetGroupName(unsigned id, char *out, int size) { VOX_LOCK; return m_internal->GetGroupManager()->GetGroupName(id, out, size); }

void VoxEngine::SetGroup(EmitterHandle &h, const char *name) { SetGroup(h, GetGroupId(name)); }

void VoxEngine::SetGroup(EmitterHandle &h, unsigned id) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetGroup(id); }

// @0x5dfd0c
unsigned VoxEngine::GetGroup(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? (unsigned)e->GetGroup() : (unsigned)-1; }

void VoxEngine::SetLoop(EmitterHandle &h, bool loop) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetLoop(loop); }

bool VoxEngine::GetLoop(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e && e->GetLoop(); }

void VoxEngine::SetGain(EmitterHandle &h, float g) { SetGain(h, g, 0.0f); }

void VoxEngine::SetGain(EmitterHandle &h, float g, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetGain(g, fade); }

float VoxEngine::GetGain(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetGain() : 0.0f; }

void VoxEngine::SetPitch(EmitterHandle &h, float p) { SetPitch(h, p, 0.0f); }

void VoxEngine::SetPitch(EmitterHandle &h, float p, float fade) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetPitch(p, fade); }

float VoxEngine::GetPitch(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetPitch() : 1.0f; }

void VoxEngine::SetPriority(EmitterHandle &h, int p) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetPriority(p); }

int VoxEngine::GetPriority(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e ? e->GetPriority() : 0; }

// @0x5e0274
void VoxEngine::Set3DEmitterPosition(EmitterHandle &h, float x, float y, float z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameter3f(k3DEmitterPosition, x, y, z); }

void VoxEngine::Set3DEmitterVelocity(EmitterHandle &h, float x, float y, float z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameter3f(k3DEmitterVelocity, x, y, z); }

void VoxEngine::Set3DEmitterDirection(EmitterHandle &h, float x, float y, float z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameter3f(k3DEmitterDirection, x, y, z); }

void VoxEngine::Set3DEmitterParameters(EmitterHandle &h, const Vox3DEmitterParameters &p) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameters(p); }

void VoxEngine::Set3DEmitterParameterf(EmitterHandle &h, int id, float v) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameterf(id, v); }

void VoxEngine::Set3DEmitterParameteri(EmitterHandle &h, int id, int v) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Set3DParameteri(id, v); }

void VoxEngine::Get3DEmitterPosition(EmitterHandle &h, float &x, float &y, float &z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameter3f(k3DEmitterPosition, x, y, z); }

void VoxEngine::Get3DEmitterVelocity(EmitterHandle &h, float &x, float &y, float &z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameter3f(k3DEmitterVelocity, x, y, z); }

void VoxEngine::Get3DEmitterDirection(EmitterHandle &h, float &x, float &y, float &z) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameter3f(k3DEmitterDirection, x, y, z); }

void VoxEngine::Get3DEmitterParameters(EmitterHandle &h, Vox3DEmitterParameters &p) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameters(p); }

void VoxEngine::Get3DEmitterParameterf(EmitterHandle &h, int id, float &v) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameterf(id, v); }

void VoxEngine::Get3DEmitterParameteri(EmitterHandle &h, int id, int &v) { VOX_LOCK; VOX_EMITTER(h); if (e) e->Get3DParameteri(id, v); }

// @0x5e0768
void VoxEngine::SetInteractiveMusicState(EmitterHandle &h, const char *label) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetInteractiveMusicStateChange(label); }

void VoxEngine::SetKillOnResume(EmitterHandle &h, bool b) { VOX_LOCK; VOX_EMITTER(h); if (e) e->SetKillOnResume(b); }

bool VoxEngine::GetKillOnResume(EmitterHandle &h) { VOX_LOCK; VOX_EMITTER(h); return e && e->GetKillOnResume(); }

int VoxEngine::GetAllEmitters(EmitterHandle *out, int max) { return m_internal->GetAllEmitters(out, max); }

void VoxEngine::PlayGroup(const char *n) { PlayGroup(GetGroupId(n), 0.0f); }
void VoxEngine::PlayGroup(unsigned g) { PlayGroup(g, 0.0f); }
void VoxEngine::PlayGroup(const char *n, float fade) { PlayGroup(GetGroupId(n), fade); }
void VoxEngine::PlayGroup(unsigned g, float fade) { m_internal->PlayGroup(g, fade); }
void VoxEngine::StopGroup(const char *n) { StopGroup(GetGroupId(n), 0.0f); }
void VoxEngine::StopGroup(unsigned g) { StopGroup(g, 0.0f); }
void VoxEngine::StopGroup(const char *n, float fade) { StopGroup(GetGroupId(n), fade); }
// @0x5df9d8
void VoxEngine::StopGroup(unsigned g, float fade) { m_internal->StopGroup(g, fade); }
void VoxEngine::PauseGroup(const char *n) { PauseGroup(GetGroupId(n), 0.0f); }
void VoxEngine::PauseGroup(unsigned g) { PauseGroup(g, 0.0f); }
void VoxEngine::PauseGroup(const char *n, float fade) { PauseGroup(GetGroupId(n), fade); }
void VoxEngine::PauseGroup(unsigned g, float fade) { m_internal->PauseGroup(g, fade); }
void VoxEngine::ResumeGroup(const char *n) { ResumeGroup(GetGroupId(n), 0.0f); }
void VoxEngine::ResumeGroup(unsigned g) { ResumeGroup(g, 0.0f); }
void VoxEngine::ResumeGroup(const char *n, float fade) { ResumeGroup(GetGroupId(n), fade); }
void VoxEngine::ResumeGroup(unsigned g, float fade) { m_internal->ResumeGroup(g, fade); }

void VoxEngine::SetMasterGain(float g) { m_internal->SetMasterGain(g, 0.0f); }

// @0x5dffc8
void VoxEngine::SetMasterGain(float g, float fade) { m_internal->SetMasterGain(g, fade); }

float VoxEngine::GetMasterGain() { return m_internal->GetMasterGain(); }

void VoxEngine::SetGroupVolume(const char *n, float v) { SetGroupVolume(GetGroupId(n), v, 0.0f); }
void VoxEngine::SetGroupVolume(unsigned g, float v) { SetGroupVolume(g, v, 0.0f); }
// @0x5e003c
void VoxEngine::SetGroupVolume(const char *n, float v, float fade) { SetGroupVolume(GetGroupId(n), v, fade); }
void VoxEngine::SetGroupVolume(unsigned g, float v, float fade) { VOX_LOCK; m_internal->GetGroupManager()->SetVolume(g, v, fade); }
// @0x5e0128
float VoxEngine::GetGroupVolume(const char *n) { return GetGroupVolume(GetGroupId(n)); }
float VoxEngine::GetGroupVolume(unsigned g) { VOX_LOCK; return m_internal->GetGroupManager()->GetVolume(g); }
void VoxEngine::SetGroupPitch(const char *n, float p) { SetGroupPitch(GetGroupId(n), p, 0.0f); }
void VoxEngine::SetGroupPitch(unsigned g, float p) { SetGroupPitch(g, p, 0.0f); }
void VoxEngine::SetGroupPitch(const char *n, float p, float fade) { SetGroupPitch(GetGroupId(n), p, fade); }
void VoxEngine::SetGroupPitch(unsigned g, float p, float fade) { VOX_LOCK; m_internal->GetGroupManager()->SetPitch(g, p, fade); }
float VoxEngine::GetGroupPitch(const char *n) { return GetGroupPitch(GetGroupId(n)); }
float VoxEngine::GetGroupPitch(unsigned g) { VOX_LOCK; return m_internal->GetGroupManager()->GetPitch(g); }
void VoxEngine::SetGroupEnable(const char *n, bool b) { SetGroupEnable(GetGroupId(n), b, 0.0f); }
void VoxEngine::SetGroupEnable(unsigned g, bool b) { SetGroupEnable(g, b, 0.0f); }
void VoxEngine::SetGroupEnable(const char *n, bool b, float fade) { SetGroupEnable(GetGroupId(n), b, fade); }
void VoxEngine::SetGroupEnable(unsigned g, bool b, float fade) { VOX_LOCK; m_internal->GetGroupManager()->SetEnable(g, b, fade); }
bool VoxEngine::GetGroupEnable(const char *n) { return GetGroupEnable(GetGroupId(n)); }
bool VoxEngine::GetGroupEnable(unsigned g) { VOX_LOCK; return m_internal->GetGroupManager()->GetEnable(g); }

void VoxEngine::Set3DListenerPosition(float x, float y, float z) { m_internal->Set3DListenerPosition(x, y, z); }
void VoxEngine::Set3DListenerVelocity(float x, float y, float z) { m_internal->Set3DListenerVelocity(x, y, z); }
void VoxEngine::Set3DListenerOrientation(float fx, float fy, float fz, float ux, float uy, float uz) { m_internal->Set3DListenerOrientation(fx, fy, fz, ux, uy, uz); }
void VoxEngine::Set3DGeneralParameter(const Vox3DGeneralParameters &p) { m_internal->Set3DGeneralParameter(p); }
// @0x5e05d8
void VoxEngine::Set3DGeneralParameterf(int id, float v) { m_internal->Set3DGeneralParameterf(id, v); }
// @0x5e05f0
void VoxEngine::Set3DGeneralParameteri(int id, int v) { m_internal->Set3DGeneralParameteri(id, v); }
void VoxEngine::Get3DListenerPosition(float &x, float &y, float &z) { VOX_LOCK; x = m_internal->m_listener.position.x; y = m_internal->m_listener.position.y; z = m_internal->m_listener.position.z; }
void VoxEngine::Get3DListenerVelocity(float &x, float &y, float &z) { VOX_LOCK; x = m_internal->m_listener.velocity.x; y = m_internal->m_listener.velocity.y; z = m_internal->m_listener.velocity.z; }
void VoxEngine::Get3DListenerOrientation(float &fx, float &fy, float &fz, float &ux, float &uy, float &uz) {
    VOX_LOCK;
    fx = m_internal->m_listener.lookAt.x; fy = m_internal->m_listener.lookAt.y; fz = m_internal->m_listener.lookAt.z;
    ux = m_internal->m_listener.up.x; uy = m_internal->m_listener.up.y; uz = m_internal->m_listener.up.z;
}
void VoxEngine::Get3DGeneralParameter(Vox3DGeneralParameters &p) { VOX_LOCK; p = m_internal->m_3Dparameters; }
void VoxEngine::Get3DGeneralParameterf(int id, float &v) {
    VOX_LOCK;
    if (id == k3DGeneralDopplerFactor) v = m_internal->m_3Dparameters.dopplerFactor;
    else if (id == k3DGeneralSpeedOfSound) v = m_internal->m_3Dparameters.speedOfSound;
}
void VoxEngine::Get3DGeneralParameteri(int id, int &v) {
    VOX_LOCK;
    if (id == k3DGeneralDistanceModel) v = (int)m_internal->m_3Dparameters.distanceModel;
    else if (id == k3DGeneralEnhanced3d) v = (int)m_internal->m_3Dparameters.enhanced3d;
}

VoxOutputMode VoxEngine::GetOutputMode() { return m_internal->m_outputMode; }

bool VoxEngine::SetOutputMode(VoxOutputMode m) { m_internal->m_outputMode = m; return true; }

void VoxEngine::PrintDebug() { if (m_internal->GetDriver()) m_internal->GetDriver()->PrintDebug(); }

}  // namespace vox
