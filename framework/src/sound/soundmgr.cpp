// framework/src/sound/soundmgr.cpp
#include "sound/soundmgr.h"
#include <cstdio>
#include "stream/istream.h"
#include "stream/streammgr.h"
#include "system.h"
#include "vox_filesystem.h"

// the labels of the groups of the soundpack @0x3d2bdc (the initialisers of SoundMgr's statics)
const jet::String SoundMgr::k_noneStr("none");
const jet::String SoundMgr::k_musicGroupLabel("music");
const jet::String SoundMgr::k_sfxGroupLabel("sfx");
const jet::String SoundMgr::k_uiGroupLabel("UI");

// the time the engine's calls take, in the seconds of the engine (the .so's literal 0.001: the game counts in milliseconds)
static const float k_msToSeconds = 0.001f;

// The files of the engine are the game's streams (SoundMgr::InitFileSystem @0x63d174 gives the engine these six functions)
// @0x63b96c
static void *OpenSoundFile(const char *path, vox::VoxFileAccessMode) {
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(jet::String(path));
    if (s) s->Start();
    return s;
}

// @0x63b9d0
static int CloseSoundFile(void *file) {
    jet::stream::IStream *s = static_cast<jet::stream::IStream*>(file);
    s->Finish();
    delete s;
    return 0;
}

// @0x63b8b8: the number of items
static int ReadSoundFile(void *dst, int size, int count, void *file) {
    return size ? (int)static_cast<jet::stream::IStream*>(file)->Read(dst, (unsigned)(size * count)) / size : 0;
}

// @0x63b8e8: the sounds are not written
static int WriteSoundFile(const void*, int, int, void*) { return -1; }

// @0x63b8f0
static int SeekSoundFile(void *file, int offset, vox::VoxFileSeekOrigin origin) {
    jet::stream::IStream *s = static_cast<jet::stream::IStream*>(file);
    if (origin == vox::kSeekCur) s->SeekCur(offset);
    else if (origin == vox::kSeekEnd) s->SeekBeg((unsigned)((int)s->Size() - offset));
    else if (origin == vox::kSeekSet) s->SeekBeg((unsigned)offset);
    return 0;
}

// @0x63b958
static int TellSoundFile(void *file) { return (int)static_cast<jet::stream::IStream*>(file)->Tell(); }

static void Warn(const char *format, const char *arg) {
    fprintf(stderr, "[Jet] ");
    fprintf(stderr, format, arg);
    fprintf(stderr, "\n");
}

// @0x63b9fc
SoundMgr::SoundMgr() : m_vox(&vox::VoxEngine::GetVoxEngine()), m_lastSoundHandle(0), m_cleanupTimer(0), m_ninjaTimeEnable(false), m_soundEnabled(true), m_lastUpdate(0) {}

// @0x63bbc4
SoundMgr::~SoundMgr() {
    m_soundHandleMap.clear();
    m_soundUIdToDataHandle.clear();
    vox::VoxEngine::DestroyVoxEngine();
}

// @0x63bcec: the engine, its 3D parameters (the distance model 2, the doppler factor 2 and the speed of sound 1500), the files
// of the game and the soundpack
void SoundMgr::Initialize() {
    m_vox->Initialize();
    m_vox->Set3DGeneralParameteri(vox::k3DGeneralDistanceModel, 2);
    m_vox->Set3DGeneralParameterf(vox::k3DGeneralDopplerFactor, 2.0f);
    m_vox->Set3DGeneralParameterf(vox::k3DGeneralSpeedOfSound, 1500.0f);
    m_vox->Set3DGeneralParameteri(vox::k3DGeneralEnhanced3d, 0);
    InitFileSystem();
    ReloadSoundPack();
}

// @0x63bd54: the banks that are named have their number of sounds multiplied by the factor (one at least)
void SoundMgr::ReconfigureBanks(const std::vector<std::string> &names, float factor) {
    for (size_t i = 0; i < names.size(); i++) {
        const unsigned id = m_vox->GetPriorityBankIdFromName(names[i].c_str());
        if (id == (unsigned)-1) continue;
        vox::priority_bank::CreationSettings s;
        m_soundPackXML.GetBankInfo(id, s);
        unsigned n = (unsigned)(factor * (float)(int)s.m_maxPlayback);
        s.m_maxPlayback = n < 1 ? 1 : n;
        m_vox->ReconfigurePriorityBank(id, s);
    }
}

// @0x63be1c
void SoundMgr::ReloadSoundPack() {
    if (m_soundPackXML.LoadXML("soundpack")) {
        m_soundPackXML.AutoSetupBanks();
        m_soundPackXML.AutoSetupGroups();
    } else {
        Warn("%s", "WARNING: could not load VOX XML soundpack");
    }
}

// @0x63d174
void SoundMgr::InitFileSystem() {
    vox::FileSystemInterface &t = vox::GetFileSystem();
    t.open = OpenSoundFile;
    t.close = CloseSoundFile;
    t.read = ReadSoundFile;
    t.write = WriteSoundFile;
    t.seek = SeekSoundFile;
    t.tell = TellSoundFile;
}

// @0x63be78: every 200 ms the sounds that are over are forgotten, and so are the data sources that were not used for 30 seconds
void SoundMgr::Update() {
    if (!m_soundEnabled) return;
    const long long now = jet::System::GetUTime();
    const int dt = m_lastUpdate ? (int)((now - m_lastUpdate) / 1000) : 0;
    m_lastUpdate = now;
    m_cleanupTimer -= dt;
    if (m_cleanupTimer > 0) return;
    m_cleanupTimer = 200;
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end();) {
        if (m_vox->IsDone(it->second.emitter)) m_soundHandleMap.erase(it++);
        else ++it;
    }
    for (std::map<int, SoundDataHandle>::iterator it = m_soundUIdToDataHandle.begin(); it != m_soundUIdToDataHandle.end();) {
        if (now - it->second.lastUsed > 30000000LL) m_soundUIdToDataHandle.erase(it++);
        else ++it;
    }
}

// @0x43dea8
int SoundMgr::Play(const jet::String &name, unsigned fadeMs) { return Play3D(name, math::vec3<float>(0, 0, 0), fadeMs); }

// @0x47f42c
int SoundMgr::PlayIfNotPlaying(const jet::String &name, unsigned fadeMs) { return IsPlaying(name) ? -1 : Play(name, fadeMs); }

// @0x63bfe0
int SoundMgr::Play3D(const jet::String &name, const math::vec3<float> &position, unsigned fadeMs) {
    if (m_soundEnabled && !name.Equals(k_noneStr)) {
        vox::emitter::CreationSettings es;
        vox::DataHandle dh;
        if (m_soundPackXML.GetEmitterInfoFromSoundOrEvent(name.c_str(), es) && FindDataHandleBySoundUId(dh, es.m_uid))
            return PlaySoundFromSoundpack(dh, position, fadeMs);
        Warn("WARNING: PlaySoundLabel - not in soundpack, could not find handle to %s", name.c_str());
    }
    return -1;
}

// @0x63c2b0
int SoundMgr::PlaySoundFromSoundpack(vox::DataHandle &data, const math::vec3<float> &position, unsigned fadeMs) {
    const int uid = m_vox->GetUid(data);
    vox::emitter::CreationSettings es;
    if (!m_soundPackXML.GetEmitterInfo(uid, es)) {
        Warn("%s", "WARNING: PlaySoundFromSoundpack - could not find emitter info");
        return -1;
    }
    SoundInfo info;
    info.emitter = m_vox->CreateEmitter(data, es);
    m_vox->Set3DEmitterPosition(info.emitter, position.x, position.y, position.z);
    m_vox->Play(info.emitter, es.m_isLoop, (float)fadeMs * k_msToSeconds);
    info.uid = uid;
    const int handle = m_lastSoundHandle++;
    m_soundHandleMap[handle] = info;
    return handle;
}

// @0x63c5b0: the data source of a sound, loaded the first time it is asked for
bool SoundMgr::FindDataHandleBySoundUId(vox::DataHandle &out, int uid) {
    std::map<int, SoundDataHandle>::iterator it = m_soundUIdToDataHandle.find(uid);
    if (it == m_soundUIdToDataHandle.end()) {
        if (LoadDataSourceFromSoundpack(out, uid)) return true;
        const char *label = "";
        m_soundPackXML.GetSoundLabel(uid, label);
        Warn("Could not load %s", label);
        return false;
    }
    out = it->second.handle;
    it->second.lastUsed = jet::System::GetUTime();
    return true;
}

// @0x63c688
bool SoundMgr::LoadDataSourceFromSoundpack(vox::DataHandle &out, int uid) {
    vox::data_source::CreationSettings cs;
    if (!m_soundPackXML.GetDataSourceInfo(uid, cs)) return false;
    out = m_vox->LoadDataSource(cs);
    SoundDataHandle d;
    d.handle = out;
    d.lastUsed = jet::System::GetUTime();
    m_soundUIdToDataHandle[uid] = d;
    return !(out == m_emptyDataHandle);
}

SoundMgr::SoundInfo *SoundMgr::FindSoundInfoBySoundHandle(int handle) {
    std::map<int, SoundInfo>::iterator it = m_soundHandleMap.find(handle);
    return it == m_soundHandleMap.end() ? 0 : &it->second;
}

// @0x4f4fa4: the empty handle for a sound that is not there
const vox::EmitterHandle &SoundMgr::FindEmitterHandleBySoundHandle(int handle) const {
    std::map<int, SoundInfo>::const_iterator it = m_soundHandleMap.find(handle);
    return it == m_soundHandleMap.end() ? m_emptyEmitterHandle : it->second.emitter;
}

// @0x63c814: the sounds of a group go to a state
void SoundMgr::SetGroupState(const jet::String &group, const jet::String &state) {
    const unsigned gid = m_vox->GetGroupId(group.c_str());
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it)
        if (m_vox->GetGroup(it->second.emitter) == gid) m_vox->SetInteractiveMusicState(it->second.emitter, state.c_str());
}

// @0x55b314
void SoundMgr::SetState(int handle, const jet::String &state) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->SetInteractiveMusicState(h, state.c_str());
}

// @0x63ca80: the sounds that are the sound of the label go to a state
void SoundMgr::SetStateAll(const jet::String &label, const jet::String &state) {
    const int uid = m_soundPackXML.GetSoundUid(label.c_str());
    if (uid < 0) return;
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it)
        if (it->second.uid == uid) m_vox->SetInteractiveMusicState(it->second.emitter, state.c_str());
}

void SoundMgr::SetListenerPosition(const math::vec3<float> &p) { m_vox->Set3DListenerPosition(p.x, p.y, p.z); }

void SoundMgr::SetListenerOrientation(const math::vec3<float> &forward, const math::vec3<float> &up) {
    m_vox->Set3DListenerOrientation(forward.x, forward.y, forward.z, up.x, up.y, up.z);
}

void SoundMgr::SetMasterVolume(float v, unsigned fadeMs) { m_vox->SetMasterGain(v, (float)fadeMs * k_msToSeconds); }

float SoundMgr::GetMasterVolume() { return m_vox->GetMasterGain(); }

void SoundMgr::SetVolume(int handle, float v, unsigned fadeMs) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->SetGain(h, v, (float)fadeMs * k_msToSeconds);
}

float SoundMgr::GetVolume(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return h == m_emptyEmitterHandle ? 0.0f : m_vox->GetGain(h);
}

// @0x63c7a8
void SoundMgr::SetGroupVolume(const jet::String &group, float v, unsigned fadeMs) { m_vox->SetGroupVolume(group.c_str(), v, (float)fadeMs * k_msToSeconds); }

// @0x63c7f4
float SoundMgr::GetGroupVolume(const jet::String &group) { return m_vox->GetGroupVolume(group.c_str()); }

void SoundMgr::SetPitch(int handle, float p, unsigned fadeMs) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->SetPitch(h, p, (float)fadeMs * k_msToSeconds);
}

float SoundMgr::GetPitch(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return h == m_emptyEmitterHandle ? 1.0f : m_vox->GetPitch(h);
}

float SoundMgr::GetCursor(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return h == m_emptyEmitterHandle ? 0.0f : m_vox->GetPlayCursor(h);
}

void SoundMgr::SetCursor(int handle, float t) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->SetPlayCursor(h, t);
}

// @0x63c93c: the sounds that are not over stop
void SoundMgr::StopAll(unsigned fadeMs) {
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it) {
        if (m_vox->IsDone(it->second.emitter)) continue;
        it->second.stopped = true;
        m_vox->Stop(it->second.emitter, (float)fadeMs * k_msToSeconds);
    }
}

// @0x63c9c8: only the instances of the sound of the label
void SoundMgr::StopAll(const jet::String &label, unsigned fadeMs) {
    const int uid = m_soundPackXML.GetSoundUid(label.c_str());
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it) {
        if (m_vox->IsDone(it->second.emitter) || it->second.uid != uid) continue;
        it->second.stopped = true;
        m_vox->Stop(it->second.emitter, (float)fadeMs * k_msToSeconds);
    }
}

// @0x4f4f38
void SoundMgr::Stop(int handle, unsigned fadeMs) {
    SoundInfo *info = FindSoundInfoBySoundHandle(handle);
    if (!info) return;
    info->stopped = true;
    m_vox->Stop(info->emitter, (float)fadeMs * k_msToSeconds);
}

// @0x4f4e98
void SoundMgr::StopGroup(const jet::String &group, unsigned fadeMs) { m_vox->StopGroup(group.c_str(), (float)fadeMs * k_msToSeconds); }

// @0x63cb1c: everything but the music pauses
void SoundMgr::PauseAll(unsigned fadeMs) {
    const unsigned music = m_vox->GetGroupId(k_musicGroupLabel.c_str());
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it) {
        if (m_vox->GetGroup(it->second.emitter) == music || m_vox->IsDone(it->second.emitter)) continue;
        m_vox->Pause(it->second.emitter, (float)fadeMs * k_msToSeconds);
    }
}

// @0x55b354
void SoundMgr::Pause(int handle, unsigned fadeMs) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->Pause(h, (float)fadeMs * k_msToSeconds);
}

void SoundMgr::PauseGroup(const jet::String &group, unsigned fadeMs) { m_vox->PauseGroup(group.c_str(), (float)fadeMs * k_msToSeconds); }

// @0x63cbdc
void SoundMgr::ResumeAll(unsigned fadeMs) {
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it)
        if (!m_vox->IsDone(it->second.emitter)) m_vox->Resume(it->second.emitter, (float)fadeMs * k_msToSeconds);
}

// @0x4f4edc
void SoundMgr::Resume(int handle, unsigned fadeMs) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->Resume(h, (float)fadeMs * k_msToSeconds);
}

void SoundMgr::ResumeGroup(const jet::String &group, unsigned fadeMs) { m_vox->ResumeGroup(group.c_str(), (float)fadeMs * k_msToSeconds); }

const char *SoundMgr::GetLabel(int handle) {
    SoundInfo *info = FindSoundInfoBySoundHandle(handle);
    const char *label = 0;
    return info && m_soundPackXML.GetSoundLabel(info->uid, label) ? label : "";
}

bool SoundMgr::IsValid(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return !(h == m_emptyEmitterHandle) && m_vox->IsValid(h);
}

// @0x5889f4: the position, and a sound that does not move
void SoundMgr::SetPosition(int handle, const math::vec3<float> &p) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) {
        m_vox->Set3DEmitterPosition(h, p.x, p.y, p.z);
        m_vox->Set3DEmitterVelocity(h, 0, 0, 0);
    }
}

// @0x63ce74: in milliseconds
unsigned SoundMgr::GetPlayCursor(int handle) const {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return !(h == m_emptyEmitterHandle) ? (unsigned)(m_vox->GetPlayCursor(h) * 1000.0f) : 0;
}

// @0x63cec4: the duration of the sound of an emitter, in milliseconds
unsigned SoundMgr::GetDuration(int handle) const {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) return 0;
    vox::DataHandle d = m_vox->GetData(h);
    return !(d == m_emptyDataHandle) ? (unsigned)(m_vox->GetDuration(d) * 1000.0f) : 0;
}

// @0x63cf64
unsigned SoundMgr::GetDuration(const jet::String &label) const {
    SoundMgr *self = const_cast<SoundMgr*>(this);
    vox::DataHandle d;
    if (!self->FindDataHandleBySoundUId(d, self->m_soundPackXML.GetSoundUid(label.c_str()))) return 0;
    return (unsigned)(m_vox->GetDuration(d) * 1000.0f);
}

// @0x63cff8
bool SoundMgr::IsPlaying(const jet::String &label) {
    vox::emitter::CreationSettings es;
    int uid = m_soundPackXML.GetSoundUid(label.c_str());
    if (uid < 0) {
        if (!m_soundPackXML.GetEmitterInfoFromSoundOrEvent(label.c_str(), es)) return false;
        uid = es.m_uid;
    }
    vox::DataHandle d;
    if (!FindDataHandleBySoundUId(d, uid)) return false;
    vox::EmitterHandle emitters[10];
    const int n = m_vox->GetEmitterHandles(d, emitters, 10);
    for (int i = 0; i < n; i++)
        if (m_vox->IsAlive(emitters[i])) return true;
    return false;
}

bool SoundMgr::IsPlaying(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return !(h == m_emptyEmitterHandle) && m_vox->IsPlaying(h);
}

// @0x63c894
bool SoundMgr::IsGroupPlaying(const jet::String &group) {
    const unsigned gid = m_vox->GetGroupId(group.c_str());
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it)
        if (m_vox->GetGroup(it->second.emitter) == gid && m_vox->IsAlive(it->second.emitter)) return true;
    return false;
}

void SoundMgr::SetLooped(int handle, bool loop) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    if (!(h == m_emptyEmitterHandle)) m_vox->SetLoop(h, loop);
}

// @0x63cc60: the slow motion of the ninja time: the sounds go half as fast and the music is paused, and back
void SoundMgr::SetNinjaTime(bool on) {
    if (m_ninjaTimeEnable == on) return;
    for (std::map<int, SoundInfo>::iterator it = m_soundHandleMap.begin(); it != m_soundHandleMap.end(); ++it) {
        if (it->second.emitter == m_emitterHandleToIgnore) continue;
        m_vox->SetPitch(it->second.emitter, on ? 0.5f : 1.0f, 0.0f);
    }
    if (on) m_vox->PauseGroup(jet::String("MUSIC").c_str(), 0.5f);
    else m_vox->ResumeGroup(jet::String("MUSIC").c_str(), 0.5f);
    m_ninjaTimeEnable = on;
}

// @0x63cc5c
void SoundMgr::SetNinjaTimeSoundToIgnore(int handle) { m_emitterHandleToIgnore = FindEmitterHandleBySoundHandle(handle); }

// the sounds of the game are stopped when they are disabled (SetEnableSounds @0x534edc calls StopAll)
void SoundMgr::EnableSound(bool enable) {
    m_soundEnabled = enable;
    if (!enable) StopAll(0);
}

// @0x63cd58
float SoundMgr::GetNearDist(const jet::String &label) {
    vox::emitter::CreationSettings es;
    m_soundPackXML.GetEmitterInfoFromSoundOrEvent(label.c_str(), es);
    return es.m_pEmitter3DParameters.referenceDistance;
}

// @0x63cd94
float SoundMgr::GetFarDist(const jet::String &label) {
    vox::emitter::CreationSettings es;
    m_soundPackXML.GetEmitterInfoFromSoundOrEvent(label.c_str(), es);
    return es.m_pEmitter3DParameters.maxDistance;
}

// @0x63ce3c
bool SoundMgr::GetLooped(int handle) {
    vox::EmitterHandle h = FindEmitterHandleBySoundHandle(handle);
    return h == m_emptyEmitterHandle ? false : m_vox->GetLoop(h);
}

// @0x63cdd0: a native (interactive) music loops by itself
bool SoundMgr::GetLooped(const jet::String &label) {
    vox::emitter::CreationSettings es;
    m_soundPackXML.GetEmitterInfoFromSoundOrEvent(label.c_str(), es);
    vox::data_source::CreationSettings ds;
    m_soundPackXML.GetDataSourceInfo(label.c_str(), ds);
    return ds.m_decoderType == vox::kFormatVxn ? true : es.m_isLoop;
}

void SoundMgr::ResetEvent(const jet::String &label) { m_soundPackXML.ResetEvent(label.c_str()); }

// @0x63d244: the music of the player's own is playing (the game's is off), and back
void SoundMgr::OnUserMusicStop() { m_vox->SetGroupEnable(k_musicGroupLabel.c_str(), true, 1.0f); }

// @0x63d208
void SoundMgr::OnUserMusicPlay() { m_vox->SetGroupEnable(k_musicGroupLabel.c_str(), false, 1.0f); }

// @0x63d200 and @0x63d1f8: nothing
void SoundMgr::OnResume() {}

void SoundMgr::OnSuspend() {}
