// source/game/Game.h
#ifndef SOURCE_GAME_GAME_H
#define SOURCE_GAME_GAME_H

#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "application.h"
#include "core/delayer.h"
#include "jsingleton.h"
#include "smath.h"
#include "stream/istreamfactory.h"
#include "stream/ifilesystem.h"
#include "video/rendertarget.h"
#include "CGameConfig.h"
#include "localization/lang.h"
#include "GFXConfig.h"
#include "recorddb.h"
#include "core/protectedstorage.h"

namespace jet::scene { class Camera; class SceneMgr; class ModelBase; }
namespace clara { class DataEntity; class Movie; class Group; class Folder; }
class GameLevel;
class Price;

// The game application: its file systems, the managers every state shares, and the frame loop that updates the
// current state in fixed steps and renders it. Not ported yet: the online services (Gaia, social, DLC, tracking,
// store), the sound manager, the popup/waiting-screen/cheat managers and the menu music.
// the thresholds of the death's incentives: the ratio of the run's score to a friend's or the best one (the top friend, the next friend, the
// score, the mission and the challenge), and the score difference that shows the score incentive whatever the ratio
struct DeathReviveIncentive {
    float challengePtg = 0, topFriendPtg = 0, nextFriendPtg = 0, missionPtg = 0, scorePtg = 0;
    int scoreMaxDiff = 0;
};

class Game : public jet::Application, public Singleton<Game> {  // sizeof 0x810
public:
    Game();
    virtual ~Game();
    virtual void OnDisplayAttached(unsigned);
    virtual void OnDisplayDetached(unsigned);
    virtual void Init();
    virtual void Shutdown();
    virtual void OnUpdate();
    // @0x52c210: the settings and the profile are written
    virtual void OnSaveUserData();
    void PostInit();
    // the fonts of the language: the device policy and the GUI fonts (@0x50a39c)
    void LoadFonts(babel::Lang);
    void PostInitRefreshGFX();
    // the frame: a step of the simulation, and the drawing of the state
    void FrameUpdate(unsigned dt);
    void FrameRender();
    // called with every finished frame before it is shown (a host that captures frames uses it)
    void SetFrameDoneCallback(std::function<void()> f) { m_frameDone = f; }
    // host input
    void OnPointerPressed(int id, int x, int y);
    void OnPointerDragged(int id, int x, int y);
    void OnPointerReleased(int id, int x, int y);
    void OnPointerCancelled(int id, int x, int y);
    void OnKeyPressed(int);
    void OnKeyReleased(int);

    // the game's own time: the speed it runs at, easing to target over ms and holding it for hold ms (-1 for ever)
    void SetTimeSpeed(float target, int transitionMs, int holdMs, std::function<void()> onDone = nullptr);
    float GetTimeSpeed() const { return m_timeSpeed.GetCurrentValue(); }
    bool IsTimeSpeedChanging() const { return m_timeSpeed.IsUpdating(); }
    float GetUpdateTimeNotScaled() const { return 1000.0f / (float)m_updateStep; }
    int GetUpdateStep() const { return m_updateStep; }
    // the points the run gives (GameplayDef::NotifyParamChanged @0x568a40 stores them at +0x580..): per meter, banana, despicable point, near miss,
    // location change, boss beaten, friend passed, own best distance beaten, own best score beaten, obstacle destroyed by the freeze gun
    enum ScoreValue { Score_Meter, Score_Banana, Score_Despicable, Score_NearMiss, Score_LocationChange, Score_BossBeaten, Score_PassAFriend,
                      Score_BeatBestDistance, Score_BeatBestScore, Score_FreezeGunDestroyObstacle, Score_Count };
    int GetScoreValue(ScoreValue v) const { return m_scoreValues[v].GetValue(); }
    void SetScoreValue(ScoreValue v, int points) { m_scoreValues[v] = points; }
    // the despicable points bar (the GameplayDef values): the points each level of the despicable multiplier takes (the level is the statistic's
    // despicable multiplier, the last level holds once it is past them all): Game::MaxDespPointsArray_Reset/Add/GetValue @0x530b64/@0x530b9c/@0x530c48;
    // the points the bar empties per second, and the time (ms) it waits after a despicable action before it empties
    void MaxDespPointsArray_Reset();
    void MaxDespPointsArray_Add(float points);
    float MaxDespPointsArray_GetValue();
    void SetDespBarDepletionPerSec(float v) { m_despBarDepletionPerSec = v; }
    float GetDespBarDepletionPerSec() const { return m_despBarDepletionPerSec.GetValue(); }
    void SetDespBananaSafeTime(float ms) { m_despBananaSafeTime = ms; }
    float GetDespBananaSafeTime() const { return m_despBananaSafeTime.GetValue(); }  // 0x748 (ms)
    void SetDespicableSafeTime(float ms) { m_despicableSafeTime = ms; }
    float GetDespicableSafeTime() const { return m_despicableSafeTime.GetValue(); }
    int GetUpdateCounter() const { return m_updateCounter; }
    int GetRenderCounter() const { return m_renderCounter; }

    // file systems: the archives added come first, then the paths
    void AddZipFileSystem(const jet::String &path);
    std::shared_ptr<jet::stream::IStreamFactory> FileSystem_AddPath(const jet::String &path, const jet::String &ext);
    std::shared_ptr<jet::stream::IStreamFactory> FileSystem_AddPathInFront(const jet::String &path, const jet::String &ext);
    std::shared_ptr<jet::stream::IStreamFactory> FileSystem_CreateStreamFactoryFromPath(const jet::String &path, const jet::String &ext);
    bool FileSystem_IsFile(const jet::String &path);
    bool FileSystem_IsDirectory(const jet::String &path);
    jet::stream::IStream *FileSystem_CreateStreamFromPath(const jet::String &path);
    // adds name -> game version to the launch parameters
    void SetGameParam(const jet::String &name, const jet::String &file);

    // the display variants of an entity, movie, group or folder: the same name with each device suffix added
    const std::vector<jet::String> &GetDeviceGFXSuffixes() const { return m_gfx->suffixes; }
    clara::DataEntity *GetDeviceSpecificEntity(clara::DataEntity*);
    clara::DataEntity *GetDeviceSpecificEntity(const jet::String &name);
    clara::Movie *GetDeviceSpecificMovie(clara::Movie*);
    clara::Group *GetDeviceSpecificGroup(clara::Group*);
    clara::Folder *GetDeviceSpecificFolder(clara::Folder*);

    // the horizontal fov of a vertical one on this display
    float GetHFovFor(float fovY, bool useDesignRatio) const;
    // road curvature shader parameters, eased over ms
    void SetCurvatureParams(const math::vec2<float>&, unsigned ms);
    math::vec2<float> GetCurvatureParams() const { return m_curvature; }

    // shadow maps
    void InitShadowMapData();
    void RenderShadowMap(jet::scene::SceneMgr*);
    static const jet::String &GetShadowTagName();
    const jet::String &GetAppVersionName() const { return m_versionName; }

    const PlatformToggles &GetPlatformToggles() const { return CGameConfig::Current(); }
    const GFXConfigStruct *GetGFXConfig() const { return m_gfx; }
    // @0x408: the design width over the display's aspect ratio (the width the 3D menus are laid out for)
    float GetDesignWidthScale() const { return m_designWidthScale; }
    // @0x404: the width the GUI is laid out for (the GFX config entry's design width)
    float GetDesignWidth() const { return m_designWidth; }
    // @0x425: the revive menu of a death is up (Menu_DeathRevive::OnEnter, OnQuit); the effects of the level wait for it
    bool IsInDeathRevive() const { return m_inDeathRevive; }
    void SetInDeathRevive(bool b) { m_inDeathRevive = b; }
    // The revive price of a death (Game::DeathRevive_* in the .so). The prices are the gameplay definition's list DeathRevivePrices and the
    // free one DeathReviveFreePrice; the revives of the run, the costume's free revives and the revive bonus pick one of them.
    void DeathRevive_ResetPrices();
    void DeathRevive_AddPrice(Price *p) { m_deathRevivePrices.push_back(p); }
    void DeathRevive_SetFreeRevivePrice(Price *p) { m_deathReviveFreePrice = p; }
    Price *DeathRevive_GetRevivePrice();                    // @0x530dd4
    int DeathRevive_GetRevivePriceType();                   // @0x530eb8: 0 free of the bonus, 1 the bonus revive, 2 the revives owned, 3 the price
    bool DeathRevive_IsFreeRevivePrice(const Price *p) const { return p == m_deathReviveFreePrice; }  // @0x530db4
    // the thresholds of the death's incentives (GameplayDef's DeathReviveIncentive_* params)
    const DeathReviveIncentive &GetDeathReviveIncentive() const { return m_deathReviveIncentive; }
    // which: 0 challenge, 1 top friend, 2 next friend, 3 mission, 4 score
    void SetDeathReviveIncentivePtg(int which, float v);
    void SetDeathReviveScoreMaxDiff(int v) { m_deathReviveIncentive.scoreMaxDiff = v; }
    GFXConfig *GetGFXConfigData() { return m_gfxConfig; }
    bool IsPaused() const { return m_paused; }
    void SetPaused(bool p) { m_paused = p; }
    // The music of the menus is a stack of the handles of SoundMgr: a new music pauses the one under it, and when it is
    // popped it stops and the one under it resumes
    int GetMenuMusic() const { return m_menuMusic.empty() ? -1 : m_menuMusic.back(); }  // @0x4f5010
    void PlayMenuMusic(const jet::String &label);  // @0x532188
    void SetMenuMusicState(const jet::String &state);  // @0x5321a8
    void StopMenuMusic();  // @0x532228
    void PushMenuMusic(const jet::String &label);  // @0x532254
    void PopMenuMusic();  // @0x532368
    void PauseMenuMusic();  // @0x532404
    void ResumeMenuMusic();  // @0x532478
    void OnGameMusicPlay() {}  // @0x53252c
    // The sound part of Game::ApplyGameSettings @0x52ee00: the saved volumes are those of the groups of the soundpack
    void ApplySoundSettings();
    // Not in the PDB header. Verified @0x4fc6d4 (GS_Gameplay::UpdateState): a mutex-guarded bool on the
    // Game instance, set from elsewhere (likely a native pause-button/lifecycle callback on another
    // thread -- not traced this session, nothing in this tree generates that callback yet) and consumed
    // once by GS_Gameplay to push GS_Pause. No real mutex here since nothing in this tree sets the flag
    // from another thread yet; add one if/when that callback is wired up.
    void RequestPause() { m_pauseRequested = true; }
    bool ConsumePauseRequest() { bool v = m_pauseRequested; m_pauseRequested = false; return v; }
    // Verified @0x4fc720 (tail call at the end of the same block): also not in the PDB header. Real
    // meaning ("go to in-game-menu"?) not confirmed -- just bookkeeping for now, nothing reads it yet.
    void SetGoToIGM(bool b) { m_goToIGM = b; }
    bool IsInActionPhase() const { return m_isInActionPhase; }
    void SetIsInActionPhase(bool b) { m_isInActionPhase = b; }
    bool IsInLoadingPhase() const { return m_isInLoadingPhase; }
    void SetIsInLoadingPhase(bool b) { m_isInLoadingPhase = b; }
    bool IsGameplayStarted() const { return m_gameplayStarted; }
    void SetGameplayStarted(bool b) { m_gameplayStarted = b; }
    bool IsCinematic() const { return m_isCinematic; }
    void SetIsCinematic(bool b) { m_isCinematic = b; }
    bool GetPaintGUI() const;
    void SetPaintGUI(bool);
    // what the game was started to do: 0 the main menu, 1 .. 8 a level (the data's start modes)
    int GetStartMode() const { return m_startMode; }
    void SetStartMode(int m) { m_startMode = m; }
    // the fraction of the shadow map size and the downsampled render target (0 when not used)
    const boost::shared_ptr<jet::video::RenderTarget> &GetShadowRT() const { return m_shadowRT; }
    const boost::shared_ptr<jet::video::RenderTarget> &GetDownsampleRT() const { return m_downsampleRT; }
    jet::scene::Camera *GetShadowCamera() const { return m_shadowCamera; }
    void GoToMainMenu();
    void OnMainMenuLoaded();
    // what is in memory for the level is released (@0x50cdec)
    static void OnUnloadClaraData();
    // the loading screen is up (as GS_Loading sets it)
    void SetLoadingScreen(bool b) { m_loadingScreen = b; }
    bool IsLoadingScreen() const { return m_loadingScreen; }
    void SetDisplayedInitialScreens(bool b) { m_showingInitialScreens = b; }
    bool IsShowingInitialScreens() const { return m_showingInitialScreens; }
    void SetWaitingForLoad(bool b) { m_waitingForLoad = b; }
    bool IsWaitingForLoad() const { return m_waitingForLoad; }
    static const unsigned k_maxUpdates = 10;
    static const unsigned k_updateTime = 17;
    // the platform models of randomized templates stay loaded until the level goes
    void AddRandomizerModel(const boost::shared_ptr<jet::scene::ModelBase> &m) { m_randomizerModels.insert(m); }
    void FreeRandomizerModels();
    // whether the data of a location is there (the first and third ones are part of the game; the revival keeps the others as plain data)
    bool HasDLCDataFor(int location, bool addPath);
    // @0x533c34: the downloaded DLC archives (<dlc root>/<asset>.jpk) go in front of the data; true when one was added.
    // removeOld drops the archives added before, stopAtMissingIndexed adds nothing when an indexed asset is not there
    bool AddDlcFilesToFileSystem(bool removeOld, bool stopAtMissingIndexed);
    static const jet::String k_dlcRootFolder;
    static const jet::String k_dlc_file_extension;
    // the assets downloaded when needed (Game::k_onDemandDlcAssetNames @0xd0e5cc, the names of the table of contents)
    enum OnDemandDlcAssets { OnDemandDlc_FontJa, OnDemandDlc_FontZh, OnDemandDlc_FontKo, OnDemandDlc_FontTrRu, OnDemandDlc_ApkUpdate, OnDemandDlc_OptionalCostumes };
    // what a download is tracked as having been asked from (the values the costume panel asks with)
    enum DlcTrackingLocation { DlcTracking_None = 0, DlcTracking_MainMenu = 1, DlcTracking_EndScreen = 2, DlcTracking_Shop = 3 };
    static const jet::String k_onDemandDlcAssetNames[6];
    // @0x533904 and the DLCManager queries it makes (the manhattan download client is not ported): an asset is requested
    // once asked for with DownloadOnDemandDlc, ready when its archive is in the DLC folder; the download cannot go on
    // without the client, so a request stays at no progress
    void DownloadOnDemandDlc(const jet::String &asset, DlcTrackingLocation);  // @0x5337dc
    // @0xd0e5e4: the assets of the data of the locations (the table of contents' names)
    static const jet::String k_locationDlcAssetNames[6];
    void DownloadLocationDlc(int location, DlcTrackingLocation t) { DownloadOnDemandDlc(k_locationDlcAssetNames[location], t); }  // @0x533838
    void CancelLocationDlc(int location) { m_onDemandDlcRequested.erase(k_locationDlcAssetNames[location]); }  // @0x53378c
    float GetLocationDlcProgress(int location) const { return GetOnDemandDlcProgress(k_locationDlcAssetNames[location]); }  // @0x533944
    bool IsLocationDlcRequested(int location) const { return IsOnDemandDlcRequested(k_locationDlcAssetNames[location]); }
    unsigned GetLocationDlcSize(int location) const { return GetOnDemandDlcSize(k_locationDlcAssetNames[location]); }
    float GetOnDemandDlcProgress(const jet::String &asset) const;
    bool IsOnDemandDlcRequested(const jet::String &asset) const;
    bool IsOnDemandDlcReady(const jet::String &asset) const;
    // the size of the download (the table of contents' "size", what the manhattan statistics report)
    unsigned GetOnDemandDlcSize(const jet::String &asset) const;
    // @0x288: the table of contents of the DLC was received from the server (Game::OnAvailableOnServer), never here
    bool IsDlcAvailableOnServer() const { return m_dlcAvailableOnServer; }
    // @0x530f98: the online profile is to be sent again
    void UpdateProfile() { m_mustUpdateProfile = true; }
    // @0x5325d0: AnticheatingManager::IsBannedFromSocial (nobody is banned here) and the account of the device is not a restricted one
    bool AreSocialFeaturesEnabled() const { return !isInRestrictedAccount; }
    static bool isInRestrictedAccount;  // Game::isInRestrictedAccount
    // @0x47f464: the downloads of the on demand DLC were started and none is left; the download client (DLCManager) sets
    // the two values and is not ported, so the downloads never start
    bool IsDlcOnDemandFinished() const { return m_dlcOnDemandStarted && m_dlcOnDemandPending <= 0; }
    // @0x2a0: the language the settings page asked the DLC for, or switched to
    babel::Lang GetDlcLanguage() const { return m_dlcLanguage; }
    void SetDlcLanguage(babel::Lang l) { m_dlcLanguage = l; }
    // @0x534: the online services are up (what also turns the price promotions on), never here
    bool AreOnlineServicesReady() const { return m_pricePromotionsOn; }
    // the online services answer here when they are ported; the headless tests set it to open the menus that need them
    void SetOnlineServicesReady(bool b) { m_pricePromotionsOn = b; }
    // @0x534: set by the online pricing when a promotion is on; never here, so no promotions
    bool ArePricePromotionsOn() const { return m_pricePromotionsOn; }
    // @0xd0e5a8: 1 when a popup asks for the shop, 2 once the main menu opened it: the shop then shows its costumes tab
    static int GoToShop;
    // The save game: the managers' records in a RecordDB (binary, XTEA, written 5 times) under the user folder
    // (jet::System::GetUserFolder). The managers saved are the ones this tree has; FriendsMgr, RedeemCodeMgr,
    // FacebookAtLaunchMgr, WeeklyChallengeMgr, DailyTokenMgr, TauntsMgr, OnlinePlayerData, MessagesMgr,
    // AnticheatingManager, RandomUsersMgr and BappleMgr have no save data here and their records are not written
    // or read, and neither are the cloud save (CloudSaveGameMgr) and the users cache (OnlineUsersMgr).
    // @0x52ddf8: with immediate the profile is written now, else it is written after the next update (the managers call
    // it with false after a change); a save that cannot be done (less than 1 MB free, or the game is not up yet) is dropped.
    // (the second flag, which the update hands back, is not read)
    void Save(bool immediate, bool);
    // @0x52c5b0: the end-of-run screen (GS_EndGame) is pushed over the gameplay. The argument is not read by the .so: it always pushes with keepPrev.
    void GoToEndScreen(bool);
    // @0x52e5e8: the managers take their records from the save read at PostInit
    void Load();
    // @0x52edac: there is more than size bytes of room for the saves
    bool CheckFreeSpace(unsigned size);
    // @0x52db68: a save file (or settings file) that cannot be read is moved to <file>_cr and deleted
    void ValidateSaveData();
    bool IsSavePending() const { return m_savePending; }
    clara::RecordDB &GetSaveDB() { return m_saveDB; }
    unsigned GetStartDistance() const { return m_startDistance; }
    void SetStartDistance(unsigned d) { m_startDistance = d; }
private:
    void SaveToRecordDB(clara::RecordDB&, bool backup);   // @0x52df14
    bool LoadFromRecordDB(clara::RecordDB&, bool backup);  // @0x52e65c
    clara::RecordDB m_saveDB;      // 0x458: the profile, "savegame"
    bool m_saveEnabled = true;     // 0x4e8
    bool m_postInitDone = false;   // 0x268: the managers are up, the profile can be written
    bool m_savePending = false;    // 0x4e9: Save(false) was asked, the next update writes
    bool m_savePendingFlag = false;  // 0x4ea
    void ApplyGameSettings();
    std::set<boost::shared_ptr<jet::scene::ModelBase>> m_randomizerModels;  // 0x564
    std::vector<std::shared_ptr<jet::stream::IFileSystem>> m_zipFileSystems;  // 0x540
    bool m_pricePromotionsOn = false;  // 0x534
    bool m_dlcAvailableOnServer = false;  // 0x288
    bool m_dlcOnDemandStarted = false;  // 0x28a
    int m_dlcOnDemandPending = 0;  // 0x290
    babel::Lang m_dlcLanguage;  // 0x2a0
    bool m_mustUpdateProfile = false;  // 0x427
    std::set<jet::String> m_onDemandDlcRequested;
    GFXConfig *m_gfxConfig;  // 0x450
    const GFXConfigStruct *m_gfx;  // 0x454
    float m_designWidth = 0;  // 0x404
    float m_designWidthScale;  // 0x408
    bool m_inDeathRevive = false;  // 0x425
    std::vector<Price*> m_deathRevivePrices;       // 0x798: the revive prices in order; the last one is used past the end of the list
    Price *m_deathReviveFreePrice = 0;             // 0x7a4: the price of a free revive (its price is not paid)
    DeathReviveIncentive m_deathReviveIncentive;   // 0x7d0..0x7f0
    jet::String m_versionName;
    // the frame loop
    long long m_lastTime;  // 0x300
    int m_updateStep;  // 0x308
    int m_scaledStep;  // 0x2fc
    int m_updateCounter, m_renderCounter;  // 0x38c, 0x390
    bool m_paused;  // 0x318
    std::vector<int> m_menuMusic;  // 0x2b4
    bool m_pauseRequested = false;  // not in the PDB header, see RequestPause() above
    bool m_goToIGM = false;  // not in the PDB header, see SetGoToIGM() above
    bool m_oneUpdateOnly;  // 0x319
    bool m_keepPending;  // 0x31a
    jet::core::Delayer<float, jet::core::SmoothStep> m_timeSpeed;  // 0x31c
    float m_debugTimeRatio;  // 0x334
    int m_timeSpeedHold;  // 0x338
    std::function<void()> m_timeSpeedDone;  // 0x33c
    // what state of the game this is
    bool m_isInActionPhase;  // 0x41b
    bool m_isInLoadingPhase;  // 0x41d
    bool m_isCinematic;  // 0x41c
    bool m_gameplayStarted;  // 0x424
    jet::core::ProtectedStorage<int> m_scoreValues[Score_Count];  // 0x580
    std::vector<jet::core::ProtectedStorage<float> > m_maxDespPoints;  // 0x710: the points of each level of the despicable multiplier
    jet::core::ProtectedStorage<float> m_despBarDepletionPerSec;  // 0x720
    jet::core::ProtectedStorage<float> m_despBananaSafeTime;      // 0x748 (ms)
    jet::core::ProtectedStorage<float> m_despicableSafeTime;      // 0x770 (ms)
    bool m_loadingScreen;  // 0x41a
    bool m_showingInitialScreens;  // 0x2b2
    bool m_waitingForLoad;
    std::function<void()> m_frameDone;
    int m_startMode;  // 0x2f4
    unsigned m_startDistance;  // 0x350: where a run starts, 0 for the beginning
    // road curvature
    math::vec2<float> m_curvature, m_curvatureFrom, m_curvatureTo;  // 0x434, 0x42c, 0x43c
    int m_curvatureTime, m_curvatureElapsed;  // 0x444, 0x448
    bool m_curvatureAnim;  // 0x44c
    // shadow map
    boost::shared_ptr<jet::video::RenderTarget> m_shadowRT, m_downsampleRT;  // 0x550, 0x55c
    jet::scene::Camera *m_shadowCamera;  // 0x558
};

#endif
