// framework/include/sound/soundmgr.h
#ifndef FRAMEWORK_INCLUDE_SOUND_SOUNDMGR_H
#define FRAMEWORK_INCLUDE_SOUND_SOUNDMGR_H

#include <map>
#include <string>
#include <vector>
#include "core/pstring.h"
#include "jsingleton.h"
#include "smath.h"
#include "vox.h"
#include "vox_soundpack_xml.h"

// The sounds of the game: a layer over the vox engine that plays the sounds of the soundpack by their label (a sound, or an
// event that picks one) and gives each one a handle (an int) the game keeps. SoundMgr::Update forgets the handles of the sounds
// that are over, and frees the data sources that have not been used for 30 seconds.
class SoundMgr : public Singleton<SoundMgr> {  // sizeof 0xD8
public:
    SoundMgr();
    virtual ~SoundMgr();
    void Initialize();
    void Update();
    // every time is in milliseconds; the sound is -1 when it could not be played
    int Play(const jet::String&, unsigned fadeMs);
    int PlayIfNotPlaying(const jet::String&, unsigned fadeMs);
    int Play3D(const jet::String&, const math::vec3<float>&, unsigned fadeMs);
    int PlaySoundFromSoundpack(vox::DataHandle&, const math::vec3<float>&, unsigned fadeMs);
    void SetGroupState(const jet::String&, const jet::String&);
    void SetState(int, const jet::String&);
    // @0x4f4fa4: the emitter of a sound handle (the empty handle when the sound is not there)
    const vox::EmitterHandle &FindEmitterHandleBySoundHandle(int) const;
    void SetStateAll(const jet::String&, const jet::String&);
    void SetListenerPosition(const math::vec3<float>&);
    void SetListenerOrientation(const math::vec3<float>&, const math::vec3<float>&);
    void SetMasterVolume(float, unsigned);
    float GetMasterVolume();
    void SetVolume(int, float, unsigned);
    float GetVolume(int);
    void SetGroupVolume(const jet::String&, float, unsigned);
    float GetGroupVolume(const jet::String&);
    void SetPitch(int, float, unsigned);
    float GetPitch(int);
    float GetCursor(int);
    void SetCursor(int, float);
    void StopAll(unsigned);
    void StopAll(const jet::String&, unsigned);
    void Stop(int, unsigned);
    void StopGroup(const jet::String&, unsigned);
    void PauseAll(unsigned);
    void Pause(int, unsigned);
    void PauseGroup(const jet::String&, unsigned);
    void ResumeAll(unsigned);
    void Resume(int, unsigned);
    void ResumeGroup(const jet::String&, unsigned);
    const char *GetLabel(int);
    bool IsValid(int);
    void SetPosition(int, const math::vec3<float>&);
    unsigned GetPlayCursor(int) const;
    unsigned GetDuration(int) const;
    unsigned GetDuration(const jet::String&) const;
    bool IsPlaying(const jet::String&);
    bool IsPlaying(int);
    bool IsGroupPlaying(const jet::String&);
    void SetLooped(int, bool);
    void SetNinjaTime(bool);
    void SetNinjaTimeSoundToIgnore(int);
    void EnableSound(bool);
    bool GetEnableSound() const { return m_soundEnabled; }
    float GetNearDist(const jet::String&);
    float GetFarDist(const jet::String&);
    bool GetLooped(int);
    bool GetLooped(const jet::String&);
    void ResetEvent(const jet::String&);
    void ReconfigureBanks(const std::vector<std::string>&, float);
    void ReloadSoundPack();
    void OnUserMusicStop();
    void OnUserMusicPlay();
    void OnResume();
    void OnSuspend();
    vox::VoxEngine *GetVoxEngine() { return m_vox; }

    // a sound that plays: the emitter and what it is
    struct SoundInfo {
        SoundInfo() : uid(-1), stopped(false) {}
        vox::EmitterHandle emitter;
        int uid;  // the sound of the soundpack
        bool stopped;  // it was asked to stop
    };
    struct SoundDataHandle {
        SoundDataHandle() : lastUsed(0) {}
        vox::DataHandle handle;
        long long lastUsed;  // jet::System::GetUTime()
    };
protected:
    SoundInfo *FindSoundInfoBySoundHandle(int);
    bool FindDataHandleBySoundUId(vox::DataHandle&, int);
    bool LoadDataSourceFromSoundpack(vox::DataHandle&, int);
private:
    void InitFileSystem();
    vox::VoxEngine *m_vox;  // 0x4
    std::map<int, SoundDataHandle> m_soundUIdToDataHandle;  // 0x8
    int m_lastSoundHandle;  // 0x20
    std::map<int, SoundInfo> m_soundHandleMap;  // 0x24
    vox::EmitterHandle m_emptyEmitterHandle;  // 0x40
    vox::DataHandle m_emptyDataHandle;  // 0x68
    int m_cleanupTimer;  // 0x90
    vox::VoxSoundPackXML m_soundPackXML;  // 0x94
    bool m_ninjaTimeEnable;  // 0x98
    vox::EmitterHandle m_emitterHandleToIgnore;  // 0xA0
    bool m_soundEnabled;  // 0xC8
    long long m_lastUpdate;
public:
    static const jet::String k_noneStr;
    static const jet::String k_musicGroupLabel;
    static const jet::String k_sfxGroupLabel;
    static const jet::String k_uiGroupLabel;
};

#endif
