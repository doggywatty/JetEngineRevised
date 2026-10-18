// source/game/gamestates/GS_Gameplay.h
#ifndef SOURCE_GAME_GAMESTATES_GS_GAMEPLAY_H
#define SOURCE_GAME_GAMESTATES_GS_GAMEPLAY_H

// GameState.h is already real (ported earlier); jsingleton.h is the convention every other
// Singleton<T> subclass in this tree uses (see MenuMgr.h, GameSettings.h, etc).
#include <map>
#include "GameState.h"
#include "jsingleton.h"
#include "core/pstring.h"
class Menu_Ingame;
class MaterialDef;
namespace game::common { class TrackingSession; }

// sizeof verified @0x500560 (GS_Loading::_UpdateLoadingStep): mallocs 0x78 bytes right before
// constructing this, not the 0x60 this header's fields alone add up to -- see the undocumented map
// member below and re/NOTES.md for how that was found.
class GS_Gameplay : public GameState, public Singleton<GS_Gameplay> {  // sizeof 0x78
public:
    // The modes of GS_Gameplay::UpdateGameplayMode() (@0x4fcf4c): mode 0 is Normal, and the end and begin tables are keyed on (mode - 1).
    enum GameplayMode { Mode_Normal = 0, Mode_Fluffy = 1, Mode_Rocket = 2, Mode_LargeMinion = 3, Mode_Boss = 4 };
    // The PDB header had a stray non-virtual "void GetStateName();" alongside this real override --
    // same kind of extraction artifact as the missing-const GetMenuName() bug fixed on every Menu_*
    // class this session. Removed; this is the only GetStateName() GameState's pure virtual needs.
    virtual const jet::String &GetStateName();
    GS_Gameplay();
    virtual ~GS_Gameplay();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    void OnResumeCountdownEnded();
    void UpdateSubStateInit();
    void UpdateSubStatePlaying();
    void UpdateGameplayMode();
    void StartFadeIn(unsigned);
    void StartFadeOut(unsigned);
    Menu_Ingame *GetInterface();
    void ReturnFromDeath();
    void EnableBonusGameplay(bool);
    bool IsInBonusGameplay();
    bool IsInShootingArea();
    void EnableShootingArea(unsigned char);
    unsigned char GetShootingAreaMask();
    void BeginFluffyGameplay();
    void EndFluffyGameplay();
    void BeginRocketGameplay();
    void EndRocketGameplay();
    void MakeFluffyInitialFlash();
    void MakeFluffyFinalFlash();
    GameplayMode GetCurrentMode() const { return m_currentMode; }
    bool IsLargeMinionGameplayMode();
    bool IsFluffyGameplayMode();
    bool IsRocketGameplayMode();
    bool IsBossGameplayMode();
    void BeginLargeMinionGameplay();
    void EndLargeMinionGameplay();
    void MakeLargeMinionFinalFlash();
    static void ApplyLargeMinionBeginChanges();
    static void ApplyLargeMinionEndChanges();
    void ApplyFluffyBeginChanges();
    void ApplyFluffyEndChanges();
    void MakeRocketInitialFlash();
    void MakeRocketFinalFlash();
    void MakeRocketCancelFlash();
    static void OnRocketFlashFinished();
    void ApplyRocketBeginChanges();
    void ApplyRocketEndChanges();
    void OnBossStarted();
    void OnBossDefeated();
    void OnBossEnded();
    void OnBossGameplayEnded();
    static void ApplyBossBeginChanges();
    static void ApplyBossEndChanges();
    void MakeBossInitialFlash();
    void MakeBossFinalFlash();
    void ApplyAmbianceSound(jet::String*, bool);
    void ApplyMusic(jet::String*, bool);
    void ApplySoundSettings(jet::String*, jet::String*);
    void ApplyGameplaySfxVolume(int);
    void ResetSfxVolume();
    void SetCurrentAmbienceSoundGain(float);
    void SetCurrentMusicGain(float);
    void SetGameplaySoundGain(float);
    void StartCheckpointLauncher(const unsigned&, const unsigned&);
    static void ReviveMinion();
    void BeginRevivingSequence();
    void SetBossGameplayMode(bool);
    void ForceEndBossGameplayMode();
    void SetProcessingRevive(bool);
    int GetMusicHandle();
    float GetCurrentRunTime();
private:
    void InitSound();
    void StopSound();
    void InitSessions();
    // Not in the PDB header, despite being a real, named symbol in the binary (GS_Gameplay::
    // RestartSessions(), 0x4fc8dc) -- an extraction gap like the ones found in other headers this
    // session, just for a private method instead of a missing include/member this time.
    void RestartSessions();
    void UpdateTimers();
    void RenderFadeEffect();
    void CheckGameplayEnd();
public:
    static const jet::String k_stateName;
private:
    // Not in the PDB header -- the real constructor (0x4fc260) builds a std::map<jet::String,int> (an
    // _Rb_tree ctor/dtor pair) and uses operator[] on it once, right after calling GetInterface()->
    // RestartGame(), before any of the fields below are touched. This is what makes the real sizeof
    // 0x78 instead of 0x60. Purpose not yet determined (possibly a per-checkpoint or per-trigger
    // counter) -- not read or written by any method ported so far, kept only so sizeof stops lying.
    std::map<jet::String, int> m_unverifiedCounterMap;
    int m_subState;  // 0x10
    int m_fadeTimer;  // 0x14
    unsigned m_fadeDuration;  // 0x18
    unsigned m_fadeTargetAlpha;  // 0x1C
    // .so offsets of the members CheckGameplayEnd, UpdateTimers, InitSound and StopSound use (the rest of this layout is not verified):
    // +0x10 m_soundInitialized, +0x25 m_endGame, +0x28 m_endGameTimer, +0x70 m_endGameLocked, +0x74 m_previousBestScore.
    bool m_endGame;  // .so +0x25: set when the end of the run starts (CheckGameplayEnd)
    float m_endGameTimer;  // .so +0x28: the death timer; UpdateTimers takes the update time of the frame off it, the end waits for it to reach 0
    bool m_soundInitialized;  // .so +0x10: set by InitSound, StopSound only acts when it is set
    bool m_endGameLocked;  // .so +0x70: CheckGameplayEnd and InitSound return while it is set; the .so never writes it in the paths traced so far
    jet::String m_ambienceSoundLabel;  // 0x28
    int m_ambienceSoundHandle;  // 0x2C
    jet::String m_musicLabel;  // 0x30
    // the sound handle of each music label (the boss music's handle is the one of the current label; @0x4fd9d4 and @0x4fda50 read it)
    std::map<jet::String, int> m_musicSoundHandles;  // 0x34
    float m_gameplaySoundGain;  // 0x38
    bool m_isInBonusGameplay;  // 0x3C
    unsigned char m_shootingAreaClassMask;  // 0x3D
    GS_Gameplay::GameplayMode m_lastMode;  // 0x40
    GS_Gameplay::GameplayMode m_currentMode;  // 0x44
    GS_Gameplay::GameplayMode m_desiredMode;  // 0x48
    MaterialDef *m_oldPostFx;  // 0x4C
    game::common::TrackingSession *m_runSession;  // 0x50
    game::common::TrackingSession *m_currentRunSession;  // 0x54
    bool m_processingRevive;  // 0x58
    static const float s_recallInvincibleTime;
    static const jet::String k_factoryBossMusicStartLabel;
    static const jet::String k_factoryBossMusicEndLabel;
    // .so +0x74: the best score (Statistics type 0) the level knows of; set by the constructor, raised by CheckGameplayEnd
    long long m_previousBestScore;
};

#endif
