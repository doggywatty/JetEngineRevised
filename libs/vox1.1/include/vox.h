// libs/vox1.1/include/vox.h
// The public interface of Gameloft's vox sound engine (version 1.1): VoxEngine, the handles of the objects it makes, and the
// settings that create them. The engine's code is stripped of its symbols in the .so; the interface is the one the game calls
// (SoundMgr, @0x63b9fc..0x63d24c) and the structures are those of its debug information.
//
// How it works. A data source (DataObj) is a sound the engine can read: a stream and a decoder. An emitter (EmitterObj) plays a
// data source: it has a cursor of the decoder, a source of the driver, a gain, a pitch, a group, a priority and a 3D position.
// Two threads (the engine's VoxThread of UpdateEmitters and of UpdateSources, @0x5de818) feed the sources with the decoded sound
// and clean up; the driver (vox_driver.h) mixes the sources and plays them.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_H
#define LIBS_VOX1_1_INCLUDE_VOX_H

#include <cstdint>
#include <string>
#include <vector>
#include "vox_decoder.h"
#include "vox_driver.h"
#include "vox_fader.h"
#include "vox_memory.h"
#include "vox_mutex.h"
#include "vox_stream.h"
#include "vox_types.h"

namespace vox {

class VoxEngineInternal;
class Handlable;
class EmitterHandle;
class DataHandle;
class EmitterObj;

namespace group {
struct CreationSettings {
    CreationSettings()
        : m_name(0), m_busName(0), m_parentId((unsigned)-1), m_volume(1.0f), m_pitch(1.0f), m_enable(true), m_killOnResume(false), m_fadeOnPlay(0.0f), m_fadeOnStop(0.0f) {}
    const char *m_name;
    const char *m_busName;
    unsigned m_parentId;  // -1 for a group that has no parent
    float m_volume;
    float m_pitch;
    bool m_enable;
    bool m_killOnResume;  // the emitters of the group are killed when the engine resumes
    float m_fadeOnPlay;
    float m_fadeOnStop;
};
}  // namespace group

namespace priority_bank {
// what a bank does with a new sound when it already plays its maxPlayback ones
enum Behaviour { kDoNothing = 0, kStealOldest = 1, kStealLowestPriority = 2, kStealLowPriorityOrOldestSamePriority = 3, kStealQuietest = 4 };
struct CreationSettings {
    CreationSettings() : m_name(0), m_threshold((int)0x80000001), m_maxPlayback(0), m_behaviour(kDoNothing), m_parent(-1) {}
    const char *m_name;
    int m_threshold;
    unsigned m_maxPlayback;  // 0: no limit
    Behaviour m_behaviour;
    int m_parent;  // a bank that the sounds of this one count in too, -1 for none
};
}  // namespace priority_bank

namespace data_source {
struct CreationSettings {  // sizeof 0x24
    CreationSettings() : m_streamType(0), m_pStreamParams(0), m_decoderType(0), m_pDecoderParams(0), m_loadingFlags(kLoadNone), m_uid(-1), m_groupId((unsigned)-1), m_numCustomSound(0), m_customSoundStr(0) {}
    CreationSettings(int streamType, void *streamParams, int decoderType, void *decoderParams, VoxSourceLoadingFlags flags)
        : m_streamType(streamType), m_pStreamParams(streamParams), m_decoderType(decoderType), m_pDecoderParams(decoderParams), m_loadingFlags(flags), m_uid(-1), m_groupId((unsigned)-1), m_numCustomSound(0), m_customSoundStr(0) {}
    int m_streamType;  // 0x0
    void *m_pStreamParams;  // 0x4
    int m_decoderType;  // 0x8
    void *m_pDecoderParams;  // 0xC
    VoxSourceLoadingFlags m_loadingFlags;  // 0x10
    int m_uid;  // 0x14
    unsigned m_groupId;  // 0x18
    int m_numCustomSound;  // 0x1C
    const char **m_customSoundStr;  // 0x20
};
}  // namespace data_source

namespace emitter {
struct CreationSettings {  // sizeof 0xA4
    CreationSettings(float gain, float pitch, unsigned groupId, int priority, unsigned priorityBankId);
    CreationSettings();
    void SetRandomGain(float minimumRatio, float maximumRatio, bool enable);
    void SetRandomPitch(float minimumCents, float maximumCents, bool enable);
    float m_gain;  // 0x0
    float m_minimumRandomGainRatio;  // 0x4
    float m_maximumRandomGainRatio;  // 0x8
    bool m_enableRandomGain;  // 0xC
    float m_pitch;  // 0x10
    float m_minimumRandomPitchCents;  // 0x14
    float m_maximumRandomPitchCents;  // 0x18
    bool m_enableRandomPitch;  // 0x1C
    bool m_isLoop;  // 0x1D
    unsigned m_groupId;  // 0x20
    unsigned m_priorityBankId;  // 0x24
    int m_priority;  // 0x28
    bool m_is3d;  // 0x2C
    Vox3DEmitterParameters m_pEmitter3DParameters;  // 0x30
    VoxVector3f m_pPosition;  // 0x4C
    VoxVector3f m_pVelocity;  // 0x58
    VoxVector3f m_pDirection;  // 0x64
    const char *m_busName;  // 0x70
    bool m_killOnResume;  // 0x74
    float m_fadeOnPlay;  // 0x78
    float m_fadeOnStop;  // 0x7C
    int m_uid;  // 0x80
    EmitterHandleUserData *m_pEmitterHandleUserData;  // 0x84
    void (*m_stateChangedCallback)(EmitterHandle&, void*, EmitterExternState);  // 0x88
    void *m_pStateChangedCallbackUserData;  // 0x8C
    int m_numCustomSound;  // 0x90
    const char **m_customSoundStr;  // 0x94
    DriverSourceParam m_driverParam;  // 0x98
};
}  // namespace emitter

// A handle is what the user holds of an object of the engine: an id that stays valid as long as the object lives.
class Handle {  // sizeof 0x28
public:
    Handle() : m_id(0) {}
    virtual ~Handle() {}
    bool operator==(const Handle &o) const { return m_id == o.m_id; }
    bool operator!=(const Handle &o) const { return m_id != o.m_id; }
    int64_t GetId() const { return m_id; }
protected:
    explicit Handle(int64_t id) : m_id(id) {}
    int64_t m_id;  // 0x8
};

class DataHandle : public Handle {  // sizeof 0x28
public:
    virtual ~DataHandle();
    DataHandle();
    DataHandle(const DataHandle &o);
    DataHandle &operator=(const DataHandle &o);
protected:
    friend class VoxEngineInternal;
    friend class VoxEngine;
    explicit DataHandle(int64_t id);
};

class EmitterHandle : public Handle {  // sizeof 0x28
public:
    virtual ~EmitterHandle() {}
    EmitterHandle() {}
    EmitterHandle(const EmitterHandle &o) : Handle(o.m_id) {}
    EmitterHandle &operator=(const EmitterHandle &o) { m_id = o.m_id; return *this; }
protected:
    friend class VoxEngineInternal;
    friend class VoxEngine;
    friend class EmitterObj;
    explicit EmitterHandle(int64_t id) : Handle(id) {}
};

// the interface of the engine (@0x5de6a8: VoxEngine::GetVoxEngine, the one instance)
class VoxEngine {  // sizeof 0x18
public:
    ~VoxEngine();
    static VoxEngine &GetVoxEngine();
    static void DestroyVoxEngine();
    void Initialize();
    void Shutdown();
    int RegisterStreamType(StreamInterface *(*factory)(void*));
    int RegisterDecoderType(DecoderInterface *(*factory)(void*));
    unsigned AddPriorityBank(priority_bank::CreationSettings&);
    bool ReconfigurePriorityBank(unsigned, priority_bank::CreationSettings&);
    unsigned GetPriorityBankIdFromName(const char*);
    void SuspendEngine();
    void ResumeEngine();
    bool IsEngineSuspended();
    void UpdateSources();
    void UpdateEmitters(float dt);
    void Update(float dt);
    DataHandle LoadDataSource(int streamType, void *streamParams, int decoderType, void *decoderParams, const char *name);
    DataHandle LoadDataSource(int streamType, void *streamParams, int decoderType, void *decoderParams, unsigned groupId);
    DataHandle LoadDataSource(const data_source::CreationSettings&);
    DataHandle LoadDataSourceAsync(const data_source::CreationSettings&);
    DataHandle ConvertToRawSource(DataHandle&);
    DataHandle ConvertToRamBufferSource(DataHandle&);
    void SetUserData(EmitterHandle&, EmitterHandleUserData&);
    void SetUserData(DataHandle&, DataHandleUserData&);
    EmitterHandleUserData GetUserData(EmitterHandle&);
    DataHandleUserData GetUserData(DataHandle&);
    void SetPriorityBankId(EmitterHandle&, unsigned);
    void SetPriorityBankId(DataHandle&, unsigned);
    void SetUid(DataHandle&, int);
    int GetUid(EmitterHandle&);
    int GetUid(DataHandle&);
    void ReleaseDatasource(DataHandle&);
    void ReleaseDatasourceGroup(const char*);
    void ReleaseDatasourceGroup(unsigned);
    bool IsReady(EmitterHandle&);
    bool IsReady(DataHandle&);
    bool IsValid(EmitterHandle&);
    bool IsValid(DataHandle&);
    float GetDuration(DataHandle&);
    int GetEmitterHandles(DataHandle&, EmitterHandle*, int);
    int GetAllDataSources(DataHandle*, int);
    void RegisterForEmitterStateChangeNotification(EmitterHandle&, void (*callback)(EmitterHandle&, void*, EmitterExternState), void*);
    void UnregisterForEmitterStateChangeNotification(EmitterHandle&);
    EmitterHandle CreateEmitter(DataHandle&, int priority, void *userData);
    EmitterHandle CreateEmitter(DataHandle&, const emitter::CreationSettings&);
    EmitterHandle CreateEmitterAsync(DataHandle&, const emitter::CreationSettings&);
    void ReleaseEmitter(EmitterHandle&);
    DataHandle GetData(EmitterHandle&);
    void Play(EmitterHandle&);
    void Play(EmitterHandle&, float fade);
    void Play(EmitterHandle&, bool loop);
    void Play(EmitterHandle&, bool loop, float fade);
    void Stop(EmitterHandle&);
    void Stop(EmitterHandle&, float fade);
    void Pause(EmitterHandle&);
    void Pause(EmitterHandle&, float fade);
    void Resume(EmitterHandle&);
    void Resume(EmitterHandle&, float fade);
    float GetPlayCursor(EmitterHandle&);
    float GetTotalPlayTime(EmitterHandle&);
    void SetPlayCursor(EmitterHandle&, float);
    bool IsAlive(EmitterHandle&);
    bool IsPlaying(EmitterHandle&);
    bool IsDone(EmitterHandle&);
    int GetPauseCount(EmitterHandle&);
    unsigned GetStatus(EmitterHandle&);
    unsigned AddGroup(group::CreationSettings&);
    bool ReconfigureGroup(const char*, group::CreationSettings&);
    bool ReconfigureGroup(unsigned, group::CreationSettings&);
    unsigned GetGroupId(const char*);
    bool GetGroupName(unsigned, char*, int);
    void SetGroup(EmitterHandle&, const char*);
    void SetGroup(EmitterHandle&, unsigned);
    unsigned GetGroup(EmitterHandle&);
    void SetLoop(EmitterHandle&, bool);
    bool GetLoop(EmitterHandle&);
    void SetGain(EmitterHandle&, float);
    void SetGain(EmitterHandle&, float, float);
    float GetGain(EmitterHandle&);
    void SetPitch(EmitterHandle&, float);
    void SetPitch(EmitterHandle&, float, float);
    float GetPitch(EmitterHandle&);
    void SetPriority(EmitterHandle&, int);
    int GetPriority(EmitterHandle&);
    void Set3DEmitterPosition(EmitterHandle&, float, float, float);
    void Set3DEmitterVelocity(EmitterHandle&, float, float, float);
    void Set3DEmitterDirection(EmitterHandle&, float, float, float);
    void Set3DEmitterParameters(EmitterHandle&, const Vox3DEmitterParameters&);
    void Set3DEmitterParameterf(EmitterHandle&, int, float);
    void Set3DEmitterParameteri(EmitterHandle&, int, int);
    void Get3DEmitterPosition(EmitterHandle&, float&, float&, float&);
    void Get3DEmitterVelocity(EmitterHandle&, float&, float&, float&);
    void Get3DEmitterDirection(EmitterHandle&, float&, float&, float&);
    void Get3DEmitterParameters(EmitterHandle&, Vox3DEmitterParameters&);
    void Get3DEmitterParameterf(EmitterHandle&, int, float&);
    void Get3DEmitterParameteri(EmitterHandle&, int, int&);
    void SetInteractiveMusicState(EmitterHandle&, const char*);
    void SetKillOnResume(EmitterHandle&, bool);
    bool GetKillOnResume(EmitterHandle&);
    int GetAllEmitters(EmitterHandle*, int);
    void PlayGroup(const char*);
    void PlayGroup(unsigned);
    void PlayGroup(const char*, float);
    void PlayGroup(unsigned, float);
    void StopGroup(const char*);
    void StopGroup(unsigned);
    void StopGroup(const char*, float);
    void StopGroup(unsigned, float);
    void PauseGroup(const char*);
    void PauseGroup(unsigned);
    void PauseGroup(const char*, float);
    void PauseGroup(unsigned, float);
    void ResumeGroup(const char*);
    void ResumeGroup(unsigned);
    void ResumeGroup(const char*, float);
    void ResumeGroup(unsigned, float);
    void SetMasterGain(float);
    void SetMasterGain(float, float);
    float GetMasterGain();
    void SetGroupVolume(const char*, float);
    void SetGroupVolume(unsigned, float);
    void SetGroupVolume(const char*, float, float);
    void SetGroupVolume(unsigned, float, float);
    float GetGroupVolume(const char*);
    float GetGroupVolume(unsigned);
    void SetGroupPitch(const char*, float);
    void SetGroupPitch(unsigned, float);
    void SetGroupPitch(const char*, float, float);
    void SetGroupPitch(unsigned, float, float);
    float GetGroupPitch(const char*);
    float GetGroupPitch(unsigned);
    void SetGroupEnable(const char*, bool);
    void SetGroupEnable(unsigned, bool);
    void SetGroupEnable(const char*, bool, float);
    void SetGroupEnable(unsigned, bool, float);
    bool GetGroupEnable(const char*);
    bool GetGroupEnable(unsigned);
    void Set3DListenerPosition(float, float, float);
    void Set3DListenerVelocity(float, float, float);
    void Set3DListenerOrientation(float, float, float, float, float, float);
    void Set3DGeneralParameter(const Vox3DGeneralParameters&);
    void Set3DGeneralParameterf(int, float);
    void Set3DGeneralParameteri(int, int);
    void Get3DListenerPosition(float&, float&, float&);
    void Get3DListenerVelocity(float&, float&, float&);
    void Get3DListenerOrientation(float&, float&, float&, float&, float&, float&);
    void Get3DGeneralParameter(Vox3DGeneralParameters&);
    void Get3DGeneralParameterf(int, float&);
    void Get3DGeneralParameteri(int, int&);
    VoxOutputMode GetOutputMode();
    bool SetOutputMode(VoxOutputMode);
    void PrintDebug();
private:
    VoxEngine();
    VoxEngine(const VoxEngine&);
    VoxThread *m_updateThread[2];  // 0x0
    Mutex *m_mutex;  // 0x10
    bool m_isInitialized;  // 0x14
    static VoxEngine *s_voxEngine;
    static VoxEngineInternal *m_internal;
};

}  // namespace vox

#endif
