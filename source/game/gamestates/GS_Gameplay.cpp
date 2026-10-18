// source/game/gamestates/GS_Gameplay.cpp
//
// Partial port. sizeof is correct (0x78, see GS_Gameplay.h), and this is wired into GS_Loading's
// GoTo_Gameplay path. ResumeState/SuspendState set the action phase and the sounds (@0x4fc5bc/@0x4fc658),
// UpdateSubStateInit runs the level-start sequence (minus meta-progression subsystems -- see its own
// comment) and UpdateSubStatePlaying drives the level forward every frame.
#include "GS_Gameplay.h"
#include "game/interfaces/menus/Menu_Ingame.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/online/tracking/SessionTrackingMgr.h"
#include "game/GameLevel.h"
#include "game/entities/Minion.h"
#include "game/entities/LevelDef.h"
#include "game/input/InputMgr.h"
#include "GS_Pause.h"
#include "game/Game.h"
#include "game/entities/BossGameplay.h"
#include "game/entities/LargeMinionGameplay.h"
#include "game/entities/Boss.h"
#include "game/Statistics.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/leveltemplate/LevelSequenceGraph.h"
#include "game/interfaces/GameSettings.h"
#include "sound/soundmgr.h"
#include "camera/cameramgr.h"
#include "camera/cameracontroller.h"

const jet::String GS_Gameplay::k_stateName("GS_Gameplay");
const float GS_Gameplay::s_recallInvincibleTime = 1.0f;
const jet::String GS_Gameplay::k_factoryBossMusicStartLabel("");
const jet::String GS_Gameplay::k_factoryBossMusicEndLabel("");

const jet::String &GS_Gameplay::GetStateName() { return k_stateName; }

// Verified structure @0x4fc260: calls GetInterface()->RestartGame() and the still-unexplained map
// operator[] (see the header comment on m_unverifiedCounterMap) before InitSessions(). Member values
// below are this session's best-effort defaults for an untouched gameplay start, not individually
// verified against the constructor's immediate-field-store instructions the way GS_MainMenu's was --
// flagged since the layout gap above means this constructor couldn't be fully trusted line-by-line.
// m_subState(1): forced by UpdateState()'s dispatch (1->UpdateSubStateInit, 2->UpdateSubStatePlaying)
// together with UpdateSubStateInit() being the only place that writes m_subState (to 2, once) -- for the
// init pass to run exactly once right after construction the way the real binary's control flow
// requires, the constructor must set it to 1. Not an instruction-by-instruction verified store (the
// constructor's field-store instructions for this specific field weren't traced), but logically forced
// by the two methods that do have verified bodies.
GS_Gameplay::GS_Gameplay()
    : m_subState(1), m_fadeTimer(0), m_fadeDuration(0), m_fadeTargetAlpha(0), m_endGame(false),
      m_endGameTimer(0.0f), m_soundInitialized(false), m_endGameLocked(false), m_ambienceSoundHandle(-1), m_gameplaySoundGain(1.0f),
      m_isInBonusGameplay(false), m_shootingAreaClassMask(0), m_lastMode(Mode_Normal), m_currentMode(Mode_Normal),
      m_desiredMode(static_cast<GameplayMode>(-1)), m_oldPostFx(0), m_runSession(0), m_currentRunSession(0),
      m_processingRevive(false), m_previousBestScore(0) {
    // @0x4fc260: the state id 1 is what GameLevel tests (GetStateID() == 1) to reach the gameplay's sound settings
    m_stateID = 1;
    if (Menu_Ingame *ig = GetInterface()) ig->RestartGame();
    // @0x4fc340: the constructor makes the entry of the empty music label (-1), so the label reads -1 and not 0 (a 0 would name sound handle 0)
    m_musicSoundHandles[m_musicLabel] = -1;
    m_previousBestScore = Statistics::GetInstance()->GetInt64Value(-1, StatisticsSaveData::k_statScope_bestScore, 0);
    InitSessions();
}

// @0x4fc3f0
GS_Gameplay::~GS_Gameplay() {}

// @0x4fc5bc: the ambience resumes in 500 ms when it has a handle, the music label's sound resumes the same way (the label map is keyed by
// the label, operator[] adds it), the action phase starts (Game+0x41B, the level's templates and the minion move only in it) and both
// sessions resume; the sound effects volume is applied at once.
void GS_Gameplay::ResumeState() {
    SoundMgr *sm = SoundMgr::GetInstance();
    if (m_ambienceSoundHandle >= 0) sm->Resume(m_ambienceSoundHandle, 500);
    const int musicHandle = m_musicSoundHandles[m_musicLabel];
    if (musicHandle >= 0) sm->Resume(musicHandle, 500);
    Game::GetInstance()->SetIsInActionPhase(true);
    if (m_runSession) m_runSession->Resume();
    if (m_currentRunSession) m_currentRunSession->Resume();
    ApplyGameplaySfxVolume(0);
}

// @0x4fc658: the action phase ends, the sounds are paused at once, both sessions pause and the sound effects volume is reset
void GS_Gameplay::SuspendState() {
    Game::GetInstance()->SetIsInActionPhase(false);
    SoundMgr::GetInstance()->PauseAll(0);
    if (m_runSession) m_runSession->Pause();
    if (m_currentRunSession) m_currentRunSession->Pause();
    ResetSfxVolume();
}

// @0x4fdd5c: the sound effects group takes the saved sound volume times the gameplay's gain (+0x50), faded over the given time
void GS_Gameplay::ApplyGameplaySfxVolume(int fadeMs) {
    SoundMgr::GetInstance()->SetGroupVolume(SoundMgr::k_sfxGroupLabel, GameSettings::GetInstance()->GetSavedSoundVolume() * m_gameplaySoundGain, (unsigned)fadeMs);
}

// @0x4fddc4: the same group volume, set at once
void GS_Gameplay::ResetSfxVolume() {
    SoundMgr::GetInstance()->SetGroupVolume(SoundMgr::k_sfxGroupLabel, GameSettings::GetInstance()->GetSavedSoundVolume() * m_gameplaySoundGain, 0);
}

// Verified structure @0x4fc6b4: a mutex-guarded pause-request flag on the Game instance is checked first
// (pushes GS_Pause and returns if set -- now real, ported this update), then substate dispatch
// (1->UpdateSubStateInit, 2->UpdateSubStatePlaying), UpdateGameplayMode(), InitSound(), UpdateTimers(),
// CheckGameplayEnd(), then both sessions get Update(dt).
void GS_Gameplay::UpdateState() {
    if (Game::GetInstance()->ConsumePauseRequest()) {
        GameState::PushState(new GS_Pause(), true);
        Game::GetInstance()->SetGoToIGM(false);
        return;
    }
    if (m_subState == 1) UpdateSubStateInit();
    else if (m_subState == 2) UpdateSubStatePlaying();
    UpdateGameplayMode();
    InitSound();
    UpdateTimers();
    CheckGameplayEnd();
    // @0x4fc6b4 (tail): both sessions run on the update time of the frame in seconds (Application+0x1C is the update time as a float in ms)
    const float dt = Game::GetInstance()->GetUpdateTimeAsFloat() * 0.001f;
    if (m_runSession) m_runSession->Update(dt);
    if (m_currentRunSession) m_currentRunSession->Update(dt);
}

// Verified @0x4fd0f0: GameLevel::Render() then RenderFadeEffect(). RenderFadeEffect isn't implemented
// yet (no fade-effect rendering path identified in this tree) -- left out rather than guessed.
void GS_Gameplay::RenderState() { GameLevel::GetInstance()->Render(); }

// Verified @0x4fdb0c: a tail call to MenuMgr::GetInstance()->GetMenu(Menu_Ingame::k_menuName).
Menu_Ingame *GS_Gameplay::GetInterface() {
    return static_cast<Menu_Ingame *>(MenuMgr::GetInstance()->GetMenu(Menu_Ingame::k_menuName));
}

// @0x4fcf4c (UpdateGameplayMode): nothing runs while the HUD's resume countdown is on (Menu_Ingame+0x1FC). A requested mode ends the mode
// it replaces and begins the new one (the tables keyed on mode - 1: 3 = LargeMinion, 4 = Boss), then the current mode's gameplay gets
// Update(). The fluffy (1) and rocket (2) gameplays are not in the tree.
void GS_Gameplay::UpdateGameplayMode() {
    Menu_Ingame *ingame = GetInterface();
    if (ingame && ingame->IsCountdownEnabled()) return;
    GameLevel *level = GameLevel::GetInstance();
    if (m_desiredMode != -1) {
        const GameplayMode oldMode = m_currentMode;
        m_lastMode = oldMode;
        m_currentMode = m_desiredMode;
        m_desiredMode = static_cast<GameplayMode>(-1);
        if (oldMode == Mode_LargeMinion) level->EndLargeMinionGameplay();
        if (oldMode == Mode_Boss) level->EndBossGameplay();      // @0x4fcff4
        if (m_currentMode == Mode_LargeMinion) level->BeginLargeMinionGameplay();
        if (m_currentMode == Mode_Boss) level->BeginBossGameplay();  // @0x4fd064
    }
    if (m_currentMode == Mode_LargeMinion) static_cast<LargeMinionGameplay*>(level->GetLargeMinionGameplay())->Update();
    if (m_currentMode == Mode_Boss && level->GetBossGameplay()) level->GetBossGameplay()->Update();  // @0x4fd0d8 (the update through the vtable)
}

bool GS_Gameplay::IsFluffyGameplayMode() { return m_currentMode == Mode_Fluffy; }
bool GS_Gameplay::IsRocketGameplayMode() { return m_currentMode == Mode_Rocket; }
bool GS_Gameplay::IsLargeMinionGameplayMode() { return m_currentMode == Mode_LargeMinion; }

// @0x4fd420: the large minion starts (the large minion pickup, GameItem::Collect). The statistic counts it, the level's post effect is kept
// and the post effect of the mode flashes in; the mode itself is requested when the flash is done (ApplyLargeMinionBeginChanges). The minion
// takes the large minion state (label 0x13).
void GS_Gameplay::BeginLargeMinionGameplay() {
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_largeMinionCount, 1);
    GameLevel *level = GameLevel::GetInstance();
    m_oldPostFx = level->GetPostFxMaterial();
    LargeMinionGameplay *lmg = static_cast<LargeMinionGameplay*>(level->GetLargeMinionGameplay());
    level->SetCurrentPostEffect(lmg->GetPostFxMaterial(), true, GameLevel::k_fadeFx_glowTechnique, 200, 1000, 300,
                                jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, &GS_Gameplay::ApplyLargeMinionBeginChanges);
    level->GetMinion()->SM_SetStateLabel(0x13);
}

// @0x4fd510: the large minion mode ends with a flash back to the level's post effect (ApplyLargeMinionEndChanges ends the mode when it is done)
void GS_Gameplay::MakeLargeMinionFinalFlash() {
    GameLevel::GetInstance()->SetCurrentPostEffect(m_oldPostFx, true, GameLevel::k_fadeFx_glowTechnique, 300, 500, 100,
                                                   jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, &GS_Gameplay::ApplyLargeMinionEndChanges);
}

// @0x4fd584: the flash is done: the large minion mode is requested (UpdateGameplayMode begins it) when the minion follows a pacesetter
// (minion+0x218); without one the minion's state 1 is set instead and the mode is not requested
void GS_Gameplay::ApplyLargeMinionBeginChanges() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (minion->GetPacesetter()) GS_Gameplay::GetInstance()->m_desiredMode = Mode_LargeMinion;
    else minion->SM_SetStateLabel(1);
}

// @0x4fd5d8: the flash is done: the mode goes back to normal (UpdateGameplayMode ends the large minion)
void GS_Gameplay::ApplyLargeMinionEndChanges() { GS_Gameplay::GetInstance()->m_desiredMode = Mode_Normal; }

// @0x4fd4d8: the large minion mode ends with the minion's engine event 0x2F
void GS_Gameplay::EndLargeMinionGameplay() { GameLevel::GetInstance()->GetMinion()->SM_OnEngineEvent(0x2f); }
bool GS_Gameplay::IsBossGameplayMode() { return m_currentMode == Mode_Boss; }

bool GS_Gameplay::IsInShootingArea() { return m_shootingAreaClassMask != 0; }
unsigned char GS_Gameplay::GetShootingAreaMask() { return m_shootingAreaClassMask; }
// Real EnableShootingArea @0x4fd304 also flips a flag on an unidentified global when the mask's
// zero/nonzero-ness changes (likely a touch-input lock) -- left out, only the mask store is real/safe.
void GS_Gameplay::EnableShootingArea(unsigned char mask) { m_shootingAreaClassMask = mask; }

bool GS_Gameplay::IsInBonusGameplay() { return m_isInBonusGameplay; }
void GS_Gameplay::EnableBonusGameplay(bool v) { m_isInBonusGameplay = v; }

// @0x4fe084: the boss gameplay mode is the next mode when the boss starts, and Normal when it ends
void GS_Gameplay::SetBossGameplayMode(bool v) { m_desiredMode = v ? Mode_Boss : Mode_Normal; }

// @0x4fe098: the modes go back to none (the last and the current are Normal), and the level ends the boss gameplay
void GS_Gameplay::ForceEndBossGameplayMode() {
    m_lastMode = Mode_Normal;
    m_currentMode = Mode_Normal;
    m_desiredMode = static_cast<GameplayMode>(-1);
    GameLevel::GetInstance()->EndBossGameplay();
}

// @0x4fd7f4: the boss gameplay's mode is set when the boss begins
void GS_Gameplay::ApplyBossBeginChanges() { GS_Gameplay::GetInstance()->SetBossGameplayMode(true); }

// @0x4fd818: and reset when it ends
void GS_Gameplay::ApplyBossEndChanges() { GS_Gameplay::GetInstance()->SetBossGameplayMode(false); }

// @0x4fd83c: the level flashes with the glow of the large minion's post effect; the post effect of the level is kept to go back to, and the
// boss's mode is set when the flash is done
void GS_Gameplay::MakeBossInitialFlash() {
    GameLevel *level = GameLevel::GetInstance();
    m_oldPostFx = level->GetPostFxMaterial();
    LargeMinionGameplay *lmg = static_cast<LargeMinionGameplay*>(level->GetLargeMinionGameplay());
    level->SetCurrentPostEffect(lmg->GetPostFxMaterial(), true, GameLevel::k_fadeFx_glowTechnique, 300, 1200, 300,
                                jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, &GS_Gameplay::ApplyBossBeginChanges);
}

// @0x4fd8b8: the final flash of the boss gameplay, which ends the mode when it is done
void GS_Gameplay::MakeBossFinalFlash() {
    GameLevel *level = GameLevel::GetInstance();
    m_oldPostFx = level->GetPostFxMaterial();
    LargeMinionGameplay *lmg = static_cast<LargeMinionGameplay*>(level->GetLargeMinionGameplay());
    level->SetCurrentPostEffect(lmg->GetPostFxMaterial(), true, GameLevel::k_fadeFx_glowTechnique, 300, 1200, 300,
                                jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, &GS_Gameplay::ApplyBossEndChanges);
}

// @0x4fd9d0: empty in the .so
void GS_Gameplay::OnBossStarted() {}

// the boss's music outro state (Boss::k_bossSoundStateOutro)
static const jet::String k_bossSoundStateOutro("outro");

// @0x4fd9d4: the boss is defeated: the sound of the current music label goes to its outro, when it is still there
void GS_Gameplay::OnBossDefeated() {
    int handle = m_musicSoundHandles[m_musicLabel];
    if (handle < 0) return;
    SoundMgr::GetInstance()->SetState(handle, k_bossSoundStateOutro);
}

// @0x4fda50: the boss gameplay's music ends: the factory boss music of the end label stops, and the sound of the current music label stops
// (it is then no longer assigned to a label)
void GS_Gameplay::OnBossEnded() {
    SoundMgr::GetInstance()->StopAll(k_factoryBossMusicEndLabel, 500);
    int handle = m_musicSoundHandles[m_musicLabel];
    if (handle < 0) return;
    SoundMgr::GetInstance()->Stop(handle, 0);
    m_musicSoundHandles[m_musicLabel] = -1;
    m_musicLabel = jet::String();
}

// @0x4fdae0: the boss gameplay's state becomes the final one (it ends after its final flash)
void GS_Gameplay::OnBossGameplayEnded() { GameLevel::GetInstance()->GetBossGameplay()->SetState(BossGameplay::BossGameplayState_Final); }

// the handle of the sound of the current music label (-1 when there is none)
int GS_Gameplay::GetMusicHandle() {
    std::map<jet::String, int>::const_iterator it = m_musicSoundHandles.find(m_musicLabel);
    return it == m_musicSoundHandles.end() ? -1 : it->second;
}

// @0x4fdbdc: the music becomes the label. Nothing is done when it is the current label and it is not asked to play. The sound of the
// previous label (its emitter, when it has one) is faded out in 0.2 s. The new label's sound is resumed in 200 ms when its emitter is
// still there, and is played otherwise; its handle is kept in the label map (GS+0x38) for the boss hooks.
void GS_Gameplay::ApplyMusic(jet::String *label, bool play) {
    if (m_musicLabel == *label && !play) return;
    if (!m_musicLabel.empty()) {
        const int handle = m_musicSoundHandles[m_musicLabel];
        if (handle >= 0) {
            SoundMgr *sm = SoundMgr::GetInstance();
            vox::EmitterHandle emitter = sm->FindEmitterHandleBySoundHandle(handle);
            if (emitter != vox::EmitterHandle()) sm->GetVoxEngine()->Stop(emitter, 0.2f);
        }
    }
    m_musicLabel = *label;
    if (m_musicLabel.empty()) return;
    SoundMgr *sm = SoundMgr::GetInstance();
    std::map<jet::String, int>::iterator it = m_musicSoundHandles.find(m_musicLabel);
    if (it != m_musicSoundHandles.end() && it->second != -1) {
        if (sm->FindEmitterHandleBySoundHandle(it->second) != vox::EmitterHandle()) {
            sm->Resume(it->second, 200);
            return;
        }
    }
    m_musicSoundHandles[m_musicLabel] = sm->Play(m_musicLabel, 200);
}
void GS_Gameplay::SetProcessingRevive(bool v) { m_processingRevive = v; }

// @0x4fdf74: the minion is revived (the end of the revive sequence). A boss gameplay ends with the revive, the level is told, the camera is
// snapped back to the gameplay's, and the run's end is unlocked again.
void GS_Gameplay::ReviveMinion() {
    GS_Gameplay *gs = GetInstance();
    GameLevel *level = GameLevel::GetInstance();
    if (gs->IsBossGameplayMode()) {
        level->GetBossGameplay()->Reset();
        gs->ForceEndBossGameplayMode();
    }
    level->OnMinionRevive();
    CameraMgr::GetInstance()->GetCurrentController()->Update(99999);
    gs->m_endGameLocked = false;
}

// @0x4fe000: the revive's fade: the level's post effect fades out (500 ms), holds, and fades back in (500 ms); ReviveMinion runs when it is
// faded out. The end of the run is locked meanwhile.
void GS_Gameplay::BeginRevivingSequence() {
    GameLevel::GetInstance()->SetCurrentPostEffect(GameLevel::GetInstance()->GetPostFxMaterial(), true, GameLevel::k_fadeFx_defaultTechnique, 500, 0, 500,
                                                   jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, &GS_Gameplay::ReviveMinion);
    m_endGameLocked = true;
}

// Verified structure @0x4fcc88, conservatively trimmed. The real function also touches
// WeeklyChallengeMgr/MissionMgr/ChallengeMgr/IGNotificationMgr/AchievementsMgr/MessagesMgr/
// LevelSequenceGraph -- all real classes in this tree, but wiring seven meta-progression subsystems
// together in one pass (each with its own unverified internals) is too large a bite to take reliably
// here, and none of them are what actually makes the level run. Kept only the part that does: tell the
// level script gameplay is starting, restart the run-time session, flip to substate 2 (so this doesn't
// run again), advance the level once, and push the real in-game HUD (Menu_Ingame, already ported this
// session) so it's visible during gameplay instead of staying on whatever menu was showing. The skipped
// calls are meta-progression bookkeeping (challenges, missions, achievements, notifications) that would
// need their own disassembly+verification pass -- left for later rather than guessed at.
void GS_Gameplay::UpdateSubStateInit() {
    LevelDef::GetInstance()->OnInitGamePlay();
    RestartSessions();
    m_subState = 2;
    // The sound of the level starts before the level's first update (@0x4fcd18), as the .so does
    InitSound();
    GameLevel::GetInstance()->Update(false);
    MenuMgr::GetInstance()->PushMenu(Menu_Ingame::k_menuName);
    // @0x4fcd7c..0x4fce78: the missions of the run, the multiplier record, the pending notifications of the missions and achievements,
    // the costume manager's reset flag (cleared: the wear guard of SetEquippedCostume is up during the run, OnMinionNoRevive sets it again)
    // and the level sequence's static data
    MissionMgr::GetInstance()->RefreshGameMissions();
    Statistics *st = Statistics::GetInstance();
    st->SetStatValueIfBetter(StatisticsSaveData::k_statScope_multiplierCountValue, st->GetScoreMultiplier());
    MissionMgr::GetInstance()->ForceActiveMissionUpdate();
    MissionMgr::GetInstance()->OnStartGameShowPendingIGNotifies();
    AchievementsMgr::GetInstance()->OnStartGameShowPendingIGNotifies();
    CostumeMgr::GetInstance()->ClearWasReset();
    LevelSequenceGraph::ResetStaticData();
    // Not in the real UpdateSubStateInit (SetPlayer is called from elsewhere in the real binary, not
    // traced this session) -- added here as the most logical place in this tree's current call graph,
    // since LaunchEvents() (still unported, see InputMgr.cpp) null-checks m_player and does nothing
    // without it, and this is the first point a level's Minion is guaranteed to exist.
    InputMgr::GetInstance()->SetPlayer(GameLevel::GetInstance()->GetMinion());
}

// @0x4fcee4: the level runs while the HUD's resume countdown is not on (Menu_Ingame::IsCountdownEnabled, the .so's field 0x1FC), the
// end of the run is not locked, and the game is not paused (Game+0x318)
void GS_Gameplay::UpdateSubStatePlaying() {
    if (Menu_Ingame *hud = GetInterface())
        if (hud->IsCountdownEnabled()) return;
    if (m_endGameLocked) return;
    if (Game::GetInstance()->IsPaused()) return;
    GameLevel::GetInstance()->Update(true);
}

// @0x4fe0cc: the resume countdown is over: the sound effects group is resumed at once (SoundMgr::ResumeGroup with no fade, as the .so calls it)
void GS_Gameplay::OnResumeCountdownEnded() { SoundMgr::GetInstance()->ResumeGroup(SoundMgr::k_sfxGroupLabel, 0); }

// @0x4fcb40: the death timer runs down by the update time of the frame, in whole ms (Application+0x18 is the update time in us, the .so
// divides it by 1000 with an unsigned division, which is what Application::GetUpdateTime() returns).
void GS_Gameplay::UpdateTimers() { m_endGameTimer -= (float)Game::GetInstance()->GetUpdateTime(); }

// @0x4fc8ec: the end of the run. Nothing happens outside the action phase (Game::IsInActionPhase @0x5181c4), while the death timer runs,
// once the end has been locked (+0x70), or while the minion is alive (Minion::IsDead @0x577230 reads its state id 0x1e). Then the end starts:
// the flag is set (+0x25), the end screen is pushed (Game::GoToEndScreen @0x52c5b0), the gameplay sound stops, and the profile is marked for
// saving. The score (Statistics type 0 of the score scope) raises the best score kept in +0x74 when it is higher.
// Not ported, offline: the .so then builds the list of online friends to notify (LeaderboardMgr::GetNumUsersLoaded/GetUserScore/IsPlayer
// and FriendsMgr::SendSocialPNToUsers). That list comes from the leaderboards, which are empty without a connection, so it stays empty here.
void GS_Gameplay::CheckGameplayEnd() {
    if (!Game::GetInstance()->IsInActionPhase()) return;
    if (m_endGameTimer > 0.0f) return;
    if (m_endGameLocked) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion || !minion->IsDead()) return;
    m_endGame = true;
    Game::GetInstance()->GoToEndScreen(true);
    StopSound();
    const long long score = Statistics::GetInstance()->GetInt64Value(-1, StatisticsSaveData::k_statScope_score, 0);
    if (score > m_previousBestScore) m_previousBestScore = score;
    Game::GetInstance()->UpdateProfile();
}

// @0x4fcb88: once the gameplay is started (and not locked), the ambience and the music of the labels are played. The music is played
// when the label has no sound yet: the map is read with operator[], which creates a missing entry with the value 0 (not -1), so
// only a label whose entry is -1 starts.
void GS_Gameplay::InitSound() {
    if (m_endGameLocked) return;
    m_soundInitialized = true;
    if (m_ambienceSoundHandle == -1 && !m_ambienceSoundLabel.empty()) ApplyAmbianceSound(&m_ambienceSoundLabel, true);
    if (m_musicSoundHandles[m_musicLabel] != -1) return;
    if (m_musicLabel.empty()) return;
    ApplyMusic(&m_musicLabel, true);
}

// @0x4fdb38: the ambience label changes (or is asked to play again): the sound playing the old label stops in 200 ms and the new label
// plays in 200 ms when it has one. Nothing happens for the same label that is not asked to play.
void GS_Gameplay::ApplyAmbianceSound(jet::String *label, bool play) {
    if (m_ambienceSoundLabel == *label && !play) return;
    if (m_ambienceSoundHandle != -1) {
        SoundMgr::GetInstance()->Stop(m_ambienceSoundHandle, 200);
        m_ambienceSoundHandle = -1;
    }
    m_ambienceSoundLabel = *label;
    if (!m_ambienceSoundLabel.empty()) m_ambienceSoundHandle = SoundMgr::GetInstance()->Play(m_ambienceSoundLabel, 200);
}

// @0x4fcc00: the gameplay's sound stops once (InitSound set it): the two factory boss music labels stop in 500 ms, and the ambience
// sound stops in 500 ms too and loses its handle.
void GS_Gameplay::StopSound() {
    if (!m_soundInitialized) return;
    m_soundInitialized = false;
    SoundMgr *sm = SoundMgr::GetInstance();
    sm->StopAll(k_factoryBossMusicStartLabel, 500);
    sm->StopAll(k_factoryBossMusicEndLabel, 500);
    if (m_ambienceSoundHandle < 0) return;
    sm->Stop(m_ambienceSoundHandle, 500);
    m_ambienceSoundHandle = -1;
}

// Verified @0x4fc8dc: only restarts m_currentRunSession, not m_runSession -- not a typo, the real binary
// really does only touch the one field.
void GS_Gameplay::RestartSessions() { if (m_currentRunSession) m_currentRunSession->Restart(); }

// Verified @0x4fc7b4: looks up (or creates) a "RUN_SESSION" tracking session and a "CURRENT_RUN_SESSION"
// one right after it, same GetSession-then-AddSession-if-null pattern for both.
void GS_Gameplay::InitSessions() {
    game::common::SessionTrackingMgr *sm = game::common::SessionTrackingMgr::GetInstance();
    m_runSession = sm->GetSession("RUN_SESSION");
    if (!m_runSession) m_runSession = sm->AddSession("RUN_SESSION");
    m_currentRunSession = sm->GetSession("CURRENT_RUN_SESSION");
    if (!m_currentRunSession) m_currentRunSession = sm->AddSession("CURRENT_RUN_SESSION");
}
