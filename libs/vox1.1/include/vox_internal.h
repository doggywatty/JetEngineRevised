// libs/vox1.1/include/vox_internal.h
// The inside of the engine: the objects the handles stand for (data sources and emitters), the groups, the priority banks and
// VoxEngineInternal that holds them all (one lock protects them; the two update threads and the game's calls take it).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_INTERNAL_H
#define LIBS_VOX1_1_INCLUDE_VOX_INTERNAL_H

#include <list>
#include <map>
#include <string>
#include <vector>
#include "vox.h"

namespace vox {

class EmitterObj;
class VoxEngineInternal;

class Handlable {  // sizeof 0x20
public:
    explicit Handlable(int64_t id) : m_id(id) {}
    virtual ~Handlable() {}
    int64_t GetId() const { return m_id; }
protected:
    int64_t m_id;  // 0x8
};

// a sound the engine can read
class DataObj : public Handlable {  // sizeof 0x78
public:
    DataObj(int64_t id, StreamInterface *stream, DecoderInterface *decoder, int uid, VoxSourceLoadingFlags flags, unsigned groupId);
    virtual ~DataObj();
    StreamInterface *GetStream() { return m_pStream; }
    DecoderInterface *GetDecoder() { return m_pDecoder; }
    // replaces the stream and the decoder (a sound that is loaded to RAM or decoded)
    void SetSource(StreamInterface *stream, DecoderInterface *decoder);
    void RegisterEmitter(int64_t id) { m_currentEmitters.push_back(id); }
    void UnregisterEmitter(int64_t id) { m_currentEmitters.remove(id); }
    std::list<int64_t> &GetRegisteredEmitters() { return m_currentEmitters; }
    unsigned GetGroup() const { return m_groupId; }
    int GetPriorityBank() const { return m_priorityBank; }
    void SetPriorityBank(int b) { m_priorityBank = b; }
    void SetUid(int uid) { m_uid = uid; }
    int GetUid() const { return m_uid; }
    void Release() { m_released = true; }
    bool ShouldDie() const { return (m_released || m_refCount <= 0) && m_currentEmitters.empty(); }
    bool IsReady() const { return m_isReady; }
    float GetDuration() const { return m_trackParams.sampleRate > 0 ? (float)m_trackParams.numSamples / (float)m_trackParams.sampleRate : 0.0f; }
    const TrackParams &GetTrackParams() const { return m_trackParams; }
    VoxSourceLoadingFlags GetLoadingFlags() const { return m_loadingFlags; }
    void SetUserData(const DataHandleUserData &d) { m_userData = d; }
    DataHandleUserData GetUserData() const { return m_userData; }
    int m_refCount;  // the handles that stand for it
private:
    void ReadTrackParams();
    unsigned m_groupId;  // 0x20
    int m_priorityBank;  // 0x24
    int m_uid;  // 0x28
    TrackParams m_trackParams;  // 0x2C
    StreamInterface *m_pStream;  // 0x3C
    DecoderInterface *m_pDecoder;  // 0x40
    std::list<int64_t> m_currentEmitters;  // 0x44
    DataHandleUserData m_userData;  // 0x5C
    bool m_released;  // 0x65
    bool m_isReady;
    VoxSourceLoadingFlags m_loadingFlags;  // 0x6C
};

// a sound that plays
class EmitterObj : public Handlable {  // sizeof 0x1B8
public:
    EmitterObj(int64_t id, const emitter::CreationSettings &settings, DriverSourceInterface *source, StreamCursorInterface *streamCursor, DecoderCursorInterface *cursor, DataObj *data, VoxEngineInternal *engine);
    virtual ~EmitterObj();
    void Update(float dt);
    int GetGroup() const { return (int)m_groupId; }
    void SetGroup(unsigned g) { m_groupId = g; }
    int GetPriority() const { return m_priority; }
    void SetPriority(int p) { m_priority = p; }
    int GetPriorityBank() const { return m_priorityBank; }
    void SetPriorityBank(int b) { m_priorityBank = b; }
    bool IsInPriorityBank() const { return m_isInBank; }
    void SetInPriorityBank(bool b) { m_isInBank = b; }
    int GetUid() const { return m_uid; }
    float GetGain() const { return m_desiredGain; }
    // the gain that the group and the master of the engine do not touch, with the fade of the play and the stop and the random part
    float GetAttenuation() const { return m_baseGainFader.GetCurrentValue() * m_gainModifier * m_playbackFader.GetCurrentValue(); }
    void SetGain(float gain, float fadeTime);
    float GetPitch() const { return m_desiredPitch; }
    void SetPitch(float pitch, float fadeTime);
    int GetState() const { return m_currentState; }
    void Play(float fade);
    void Stop(float fade);
    void Pause(float fade);
    void Resume(float fade);
    bool GetLoop() const { return m_loop; }
    void SetLoop(bool loop);
    void SetUserData(const EmitterHandleUserData &d) { m_userData = d; }
    EmitterHandleUserData GetUserData() const { return m_userData; }
    void SetInteractiveMusicStateChange(const char *label);
    bool IsReady() const { return true; }
    bool IsAlive() const { return !m_needToDie && (m_currentState == kEmitterInit || m_currentState == kEmitterPlaying || m_currentState == kEmitterPaused); }
    bool IsPlaying() const { return m_currentState == kEmitterPlaying; }
    bool IsDone() const { return m_currentState == kEmitterDone || m_currentState == kEmitterStopped; }
    unsigned GetStatus() const { return (unsigned)m_currentState; }
    int GetPauseCount() const { return m_pauseCount; }
    float GetPlayCursor();
    float GetTotalPlayTime();
    void SetPlayCursor(float seconds);
    void SetAutoKillAfterDone(bool b) { m_autoKill = b; }
    void NeedToDie() { m_needToDie = true; }
    bool ShouldDie() const { return m_needToDie || (m_autoKill && IsDone() && m_stateSent); }
    void SetKillOnResume(bool b) { m_killOnResume = b; }
    bool GetKillOnResume() const { return m_killOnResume; }
    float GetFadeOnPlay() const { return m_fadeOnPlay; }
    float GetFadeOnStop() const { return m_fadeOnStop; }
    DataObj *GetDataSource() { return m_pDataSource; }
    DriverSourceInterface *GetDriverSource() { return m_phwSource; }
    // 3D
    void Set3DParameteri(int id, int v);
    void Set3DParameterf(int id, float v);
    void Set3DParameter3f(int id, float x, float y, float z);
    void Get3DParameteri(int id, int &v) const;
    void Get3DParameterf(int id, float &v) const;
    void Get3DParameter3f(int id, float &x, float &y, float &z) const;
    void Get3DParameters(Vox3DEmitterParameters &p) const { p = m_3Dparameters; }
    void Set3DParameters(const Vox3DEmitterParameters &p);
    bool Is3D() const { return m_is3d; }
    void RegisterStateChangedCallback(void (*cb)(EmitterHandle&, void*, EmitterExternState), void *user) { m_stateChangedCallback = cb; m_stateChangedCallbackUserData = user; }
    void UnregisterStateChangedCallback() { m_stateChangedCallback = 0; }
    // stops it at once, whatever its fade (a bank that steals the place, the end of the engine)
    void Kill();
private:
    void Feed();
    void Finish(int state);
    void Update3D();
    void SendStateChange();
    unsigned m_groupId;  // 0x2C
    int m_priority;  // 0x30
    int m_priorityBank;  // 0x34
    bool m_isInBank;  // 0x38
    float m_desiredGain;  // 0x40
    float m_gainModifier;  // 0x44: the random part
    Fader m_baseGainFader;  // 0x48
    Fader m_playbackFader;  // 0x5C
    float m_pitchModifier;  // 0x84
    float m_desiredPitch;  // 0x88
    Fader m_pitchFader;  // 0x8C
    bool m_loop;  // 0xB0
    int m_currentState;  // 0xBC
    int m_pauseCount;  // 0xC8
    bool m_killOnResume;  // 0xDC
    float m_fadeOnPlay;  // 0xE0
    float m_fadeOnStop;  // 0xE4
    bool m_is3d;
    VoxVector3f m_position;  // 0xE8
    VoxVector3f m_direction;  // 0xF4
    VoxVector3f m_velocity;  // 0x100
    Vox3DEmitterParameters m_3Dparameters;  // 0x10C
    bool m_3DNeedUpdate;
    DriverSourceInterface *m_phwSource;  // 0x160
    StreamCursorInterface *m_pStreamCursor;
    DecoderCursorInterface *m_pDecoderCursor;  // 0x164
    DataObj *m_pDataSource;  // 0x168
    VoxEngineInternal *m_pEngine;
    bool m_needToDie;  // 0x16C
    bool m_autoKill;  // 0x16D
    int m_uid;  // 0x170
    EmitterHandleUserData m_userData;  // 0x188
    void (*m_stateChangedCallback)(EmitterHandle&, void*, EmitterExternState);  // 0x190
    void *m_stateChangedCallbackUserData;  // 0x194
    DriverSourceParam m_driverParam;  // 0xD0
    bool m_decoderFinished;
    bool m_stopping;  // fading out to stop
    bool m_pausing;  // fading out to pause
    int m_lastSentState;
    bool m_stateSent;
    std::vector<char> m_decodeBuffer;
};

// the groups: a tree of volumes and pitches (and on / off) that the emitters belong to
class GroupManager {  // sizeof 0x14
public:
    GroupManager() {}
    unsigned AddGroup(const group::CreationSettings &s);
    bool ReconfigureGroup(unsigned id, const group::CreationSettings &s);
    unsigned GetGroupId(const char *name) const;
    bool GetGroupName(unsigned id, char *out, int size) const;
    bool IsChild(unsigned child, unsigned ancestor) const;
    bool IsGroupValid(unsigned id) const { return id < m_groups.size(); }
    bool GetEnable(unsigned id) const;
    bool SetEnable(unsigned id, bool enable, float fade);
    float GetVolume(unsigned id) const;
    bool SetVolume(unsigned id, float volume, float fade);
    float GetPitch(unsigned id) const;
    bool SetPitch(unsigned id, float pitch, float fade);
    void Update(float dt);
    // the volume and the pitch of a group with those of the groups it is in
    float GetEffectiveVolume(unsigned id) const;
    float GetEffectivePitch(unsigned id) const;
    const group::CreationSettings *GetSettings(unsigned id) const { return id < m_groups.size() ? &m_groups[id].settings : 0; }
private:
    struct Group {
        std::string name;
        std::string bus;
        group::CreationSettings settings;
        Fader volume;
        Fader pitch;
        Fader enable;  // 1 on, 0 off, so that a group fades out when it is disabled
    };
    std::vector<Group> m_groups;
};

// the banks limit how many sounds of a kind play at once
class PriorityBankManager {  // sizeof 0x18
public:
    struct Bank {
        std::string name;
        priority_bank::CreationSettings settings;
        std::list<EmitterObj*> elements;  // the emitters that play, the oldest first
    };
    unsigned AddPriorityBank(const priority_bank::CreationSettings &s);
    bool SetPriorityBank(unsigned id, const priority_bank::CreationSettings &s);
    unsigned GetPriorityBankId(const char *name) const;
    // false when the bank does not want the emitter (it is not played)
    bool AddEmitter(unsigned id, EmitterObj *e);
    bool RemoveEmitter(unsigned id, EmitterObj *e);
private:
    std::vector<Bank> m_banks;
};

class VoxEngineInternal {  // sizeof 0x338
public:
    static VoxEngineInternal *GetVoxEngineInternal();
    // the engine if it was made (the handles of the data sources count on it, and may outlive it)
    static VoxEngineInternal *Peek() { return s_voxEngineInternal; }
    static void Destroy();
    ~VoxEngineInternal();
    void Initialize();
    void Shutdown();
    void Suspend();
    void Resume();
    bool IsSuspended() const { return m_suspendCount > 0; }
    void UpdateSources();
    void UpdateEmitters(float dt);
    // data sources
    DataObj *GetDataObject(const DataHandle &h);
    void DecreaseDataObjectRefCount(const DataHandle &h);
    void IncreaseDataObjectRefCount(const DataHandle &h);
    int GetAllDataSources(DataHandle *out, int max);
    DataHandle LoadDataSource(const data_source::CreationSettings &s);
    DataHandle ConvertToRawSource(DataHandle &h);
    DataHandle ConvertToRamBufferSource(DataHandle &h);
    void ReleaseDatasource(DataHandle &h);
    void ReleaseDatasourceGroup(unsigned group);
    bool IsValid(const DataHandle &h);
    bool IsValid(const EmitterHandle &h);
    int GetEmitterHandles(DataHandle &h, EmitterHandle *out, int max);
    // emitters
    EmitterObj *GetEmitterObject(const EmitterHandle &h);
    EmitterHandle CreateEmitter(DataHandle &data, const emitter::CreationSettings &s);
    void ReleaseEmitter(EmitterHandle &h);
    int GetAllEmitters(EmitterHandle *out, int max);
    void KillEmittersOnResume();
    // groups, banks
    GroupManager *GetGroupManager() { return &m_groupManager; }
    PriorityBankManager *GetPriorityBankManager() { return &m_priorityBankMgr; }
    void PlayGroup(unsigned group, float fade);
    void StopGroup(unsigned group, float fade);
    void PauseGroup(unsigned group, float fade);
    void ResumeGroup(unsigned group, float fade);
    void SetMasterGain(float gain, float fade);
    float GetMasterGain() const { return m_masterGain.GetEndValue(); }
    // 3D
    void Set3DListenerPosition(float x, float y, float z);
    void Set3DListenerVelocity(float x, float y, float z);
    void Set3DListenerOrientation(float fx, float fy, float fz, float ux, float uy, float uz);
    void Set3DGeneralParameter(const Vox3DGeneralParameters &p);
    void Set3DGeneralParameterf(int id, float v);
    void Set3DGeneralParameteri(int id, int v);
    // types
    int RegisterStreamType(StreamInterface *(*factory)(void*));
    int RegisterDecoderType(DecoderInterface *(*factory)(void*));
    StreamInterface *CreateStream(int type, void *params);
    DecoderInterface *CreateDecoder(int type, void *params);
    DriverInterface *GetDriver() { return m_hwDriver; }
    Mutex &GetMutex() { return m_mutex; }
    ListenerParameters m_listener;
    Vox3DGeneralParameters m_3Dparameters;
    VoxOutputMode m_outputMode;
private:
    VoxEngineInternal();
    void _SetDefault3DParameters();
    void KillEmitter(EmitterObj *e);
    int64_t GetFreeId() { return ++m_nextId; }
    static VoxEngineInternal *s_voxEngineInternal;
    Mutex m_mutex;  // 0x11C
    std::map<int64_t, DataObj*> m_dataObjects;  // 0x0
    std::map<int64_t, EmitterObj*> m_emitterObjects;  // 0x98
    GroupManager m_groupManager;  // 0x118
    PriorityBankManager m_priorityBankMgr;  // 0x114
    std::vector<StreamInterface *(*)(void*)> m_streamTypes;  // 0x1DC
    std::vector<DecoderInterface *(*)(void*)> m_decoderTypes;  // 0x260
    DriverInterface *m_hwDriver;  // 0x2E4
    Fader m_masterGain;
    int m_suspendCount;  // 0x32C
    int64_t m_nextId;
};

}  // namespace vox

#endif
