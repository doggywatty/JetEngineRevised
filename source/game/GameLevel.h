// source/game/GameLevel.h
#ifndef SOURCE_GAME_GAMELEVEL_H
#define SOURCE_GAME_GAMELEVEL_H

#include <map>
#include <set>
#include <vector>
#include <unordered_set>
#include "jsingleton.h"
#include "smath.h"
#include "video/color.h"
#include "video/posteffect.h"
#include "core/pstring.h"
#include "core/delayer.h"
#include "core/interputils.h"
#include "gameenums.h"
#include "leveltemplate/LevelTemplateMgr.h"
#include "leveltemplate/leveltemplatesequenceprovider.h"

namespace jet::scene { class Camera; class SceneMgr; }
namespace jet::stream { class IStream; }
namespace clara { class DataEntity; class Entity; }
class GameEntity;
class LevelDef;
class LuaVM;
class CameraMgr;
class CameraController;
class MaterialDef;
class Minion;
class Boss;
class BossGameplay;
class Mission;
class MissionSet;
class ControlledRandom;
class LevelSequenceGraphMgr;
class Pacesetter;
class SafeMinionPoint;
class GameplayTemplateInstance;
class PhysicsWorld;
class PhysicsFactory;
class InteractiveObjectTemplate;

// The loaded level: the entities it updates and renders each frame, the world and menu scene managers they draw
// into, the camera, the Lua machine of its scripts, the fog and the post effect; Render() is the frame.
// Not ported yet (they come with the gameplay): the level sequence, the pacesetter and templates, the managers
// that reset with a new level, the cinematic camera, physics, effects and trails.
class GameLevel : public Singleton<GameLevel>, public LevelTemplateSequenceProvider {  // sizeof 0x690
public:
    enum SceneMgrId { SceneMgr_World = 0, SceneMgr_Menu = 1 };
    explicit GameLevel(bool isMovieLevel);
    virtual ~GameLevel();

    // the clara files of the level: libraries first (when none are loaded), then the level files
    void Load(const std::map<jet::String, jet::stream::IStream*> &libs, const std::map<jet::String, jet::stream::IStream*> &levels);
    // the script machine, once
    void Init();
    void PostInit();
    void Update(bool fullUpdate);
    void Render();
    // @0x53f3d8: a death always offers the revive (GS_EndGame pushes Menu_DeathRevive)
    bool CanRevive() const { return true; }
    // @0x53ef1c: the run ended without a revive: its bananas reach the player, its statistics close and the result screen comes up
    void OnMinionNoRevive();
    // @0x53ee5c: the minion is revived (GS_Gameplay::ReviveMinion): the death's effects go, the camera is the gameplay's again, the minion runs
    // again, the revive is counted, and the entities that update are told
    void OnMinionRevive();
    // @0x53db38: the camera of the gameplay: the camera change of the first template waiting, else the current template's camera value
    void RestoreGameplayCamera(int);
    // @0x53d4b0: the statistics of the end of a run (distance and score records, the costumes count); the game is saved when a record
    // changed, unless the argument is true
    void UpdateEndGameStatistics(bool);

    // entities as the creator makes them: the ones the level keeps by their kind
    void EntityCreated(clara::DataEntity*);
    void RegisterForUpdate(GameEntity*, bool);
    void RegisterForRender(GameEntity*, bool);
    void RegisterVolatileEntity(GameEntity*, bool);
    // entities whose model animates this frame: entity, animation name, node count (@0x51463c)
    struct Animable { GameEntity *entity; jet::String animation; int nodeCount; };
    void AddAnimatedEntity(GameEntity*, const jet::String &animation, int nodeCount);
    const std::vector<Animable> &GetAnimatedEntities() const { return m_animatedEntities; }
    // how far the player has run: the base of "pending to delete" entities going out of the scene
    unsigned GetPlayerDistance() const { return m_playerDistance; }
    void SetPlayerDistance(unsigned d) { m_playerDistance = d; }

    LevelDef *GetLevelDef() const { return m_levelDef; }
    void SetLevelDef(LevelDef *l) { m_levelDef = l; }
    LuaVM *GetLuaVM() const { return m_luaVM; }
    void SetLuaVM(LuaVM *v) { m_luaVM = v; }
    Minion *GetMinion() const { return m_minion; }
    clara::DataEntity *GetLargeMinionGameplay() const { return m_largeMinionGameplay; }
    clara::DataEntity *GetFluffyGameplay() const { return m_fluffyGameplay; }
    clara::DataEntity *GetGameplayDef() const { return m_gameplayDef; }
    // GameplayDef::NotifyParamChanged: how far the camera change plane of a template is from the template's start
    void SetCameraChangeOffset(float o) { m_cameraChangeOffset = o; }
    float GetCameraChangeOffset() const { return m_cameraChangeOffset; }
    int GetSledgeType() const { return m_sledgeType; }
    // @0x53f3e0: the boss of the boss gameplay (null when none is on)
    Boss *GetBoss() const;
    BossGameplay *GetBossGameplay() const { return m_bossGameplay; }
    void SetBossGameplay(BossGameplay *b) { m_bossGameplay = b; }
    // the boss's state (Boss::SetBehaviorState 1 and 4 set it: @0x5a45e0)
    void SetBossBehaviorOn(bool b) { m_bossBehaviorOn = b; }
    bool IsBossBehaviorOn() const { return m_bossBehaviorOn; }
    void SetLoadingLibs(bool b) { m_loadingLibs = b; }
    bool IsLoadingLibs() const { return m_loadingLibs; }
    CameraMgr *GetCameraMgr() const { return m_cameraMgr; }
    // the entity is a level camera (Lua SetCamera only takes those) (@0x519738)
    bool IsCamera(const clara::Entity*) const;
    // switches the level to the camera, blending over ms
    void SetCamera(CameraController*, unsigned ms);

    // fog: on / off, colour and depths, each switching over a time in ms (@0x518654, 0x5186a0, 0x5187bc)
    void SetFog(bool on, unsigned ms);
    void SetFogColor(const jet::video::Color &c, unsigned ms);
    void SetFogDepth(float start, float end, unsigned ms);
    bool GetFog() const { return m_fogOn.GetTargetValue() > 0.0f; }
    void GetFogColor(jet::video::Color &c) const { c = jet::video::Color(m_fogColor.GetTargetValue().x, m_fogColor.GetTargetValue().y, m_fogColor.GetTargetValue().z, m_fogColor.GetTargetValue().w); }
    void GetFogDepths(float &start, float &end) const { start = m_fogDepthTo[0]; end = m_fogDepthTo[1]; }
    void RenderFog();

    // the post effect of the level: now, or faded to over fadeOutMs / holdMs / fadeInMs through a technique
    void SetCurrentPostEffect(jet::video::PostEffect*);
    void SetCurrentPostEffect(MaterialDef*, bool fade, const jet::String &technique, int fadeOutMs, int holdMs, int fadeInMs, jet::core::InterpolationDef::type outInterp,
                              jet::core::InterpolationDef::type inInterp, void (*onDone)());
    void SetPostEffectsEnabled(bool b) { m_postFxEnabled = b; }
    // a material blended in over the level's, counted by its users
    void AddExtraPostEffect(MaterialDef*, jet::core::Delayer<float, jet::core::QuadraticEaseIn>*, unsigned *);
    void RemoveExtraPostEffect(MaterialDef*);
    static const jet::String k_fadeFx_defaultTechnique;
    static const jet::String k_fadeFx_glowTechnique;

    // what the gameplay state gives the level while it exists (it sets s_gameplay)
    struct GameplayHooks {
        virtual ~GameplayHooks() {}
        virtual int GetMode() const = 0;  // 1: the fluffy dimension
        virtual void ApplySoundSettings(jet::String *ambience, jet::String *music) = 0;
        virtual void SetCurrentMusicGain(float) = 0;
        virtual void SetGameplaySoundGain(float) = 0;
        // the dynamic libraries of a location are loaded: the loading panel goes
        virtual void OnLocationTransitionLoadingFinished() = 0;
        // the location's dynamic libraries are being loaded (a pusher's exit point leads there): the loading panel comes up
        virtual void OnLocationTransitionLoadingStarted() = 0;
    };
    static GameplayHooks *s_gameplay;
    bool IsGameplayStateCreated() const { return s_gameplay != 0; }
    bool IsFluffyMode() const { return s_gameplay && s_gameplay->GetMode() == 1; }
    void SetSound(jet::String *ambience, jet::String *music);
    void SetCurrentMusicGain(float g);
    void SetGameplaySoundGain(float g) { if (s_gameplay) s_gameplay->SetGameplaySoundGain(g); }
    // how many entities of a template are made at a time (all while negative)
    int GetEntityToSpawnCount() const { return m_entityToSpawnCount; }
    void SetEntityToSpawnCount(int c) { m_entityToSpawnCount = c; }  // @0x53ecc4: a store to +0x630
    Pacesetter *GetPacesetter() const { return m_pacesetter; }
    LevelTemplateMgr *GetLevelTemplateMgr() const { return m_levelTemplateMgr; }
    void OnLocationChanged(int from, int to);
    void OnLocationTransitionLoadingFinished() { if (s_gameplay) s_gameplay->OnLocationTransitionLoadingFinished(); }
    // @0x5403b4: the loading panel comes up (the gameplay state shows it; Menu_Ingame::SetLoadingPanelVisible @0x4efce8 is not ported)
    void OnLocationTransitionLoadingStarted() { if (s_gameplay) s_gameplay->OnLocationTransitionLoadingStarted(); }
    int GetGameplayMode() const { return s_gameplay ? s_gameplay->GetMode() : -1; }
    // the run: the graph of the location, the level templates, the loaded dynamic libraries
    void LoadLevelSequence(const jet::String &firstGraph);
    void InitLevelStructure();
    void UpdateUsedDynamicLibraries();
    void DestroyGraphMgr();
    bool IsNodeOnSpecialTree(const jet::String &id) const;
    int GetFinalBifurcation() const;
    void OnTutorialCompleted() { m_tutorialCompleted = true; }
    // the boss gameplay's tutorial sets it (BossGameplay::BeginTutorial @0x5a6234, EndTutorial @0x5a6274, KillBoss @0x5a6014)
    void SetTutorialCompleted(bool b) { m_tutorialCompleted = b; }
    // LevelTemplateSequenceProvider
    virtual void IncSequenceNumber();
    virtual int GetNextSequenceNumber();
    virtual GameEntity *GetNextCamera();
    virtual int GetNextCameraTransitionTime();
    virtual const SpeedDef *GetNextSpeed();
    virtual const jet::String &GetNextLocation();
    virtual const jet::String &GetNextNodeLabel();
    virtual const jet::String &GetNextNodeId();
    virtual int GetNextNodeGraphId();
    virtual void OnNodeCompleted(const int &graphId, const jet::String &nodeId);

    // The run along the templates: where the camera and the minion's state change, and the level moving back to the origin
    // (they call the sequence's templates: the Minion's own sledge states and the cinematic camera join with their ports)
    bool CheckCameraChange();
    // @0x53bc9c: the minion starts a sledge at the begin plane of a bonus template and ends it at the end plane
    void CheckMinionStateChange();
    void SetCameraChangeInfo(GameplayTemplateInstance*);
    // @0x539408 (the Lua StartGamePlay)
    void StartGameplay();
    void SetNextCameraChangeInfo();
    // @0x53e96c: the template manager of the level is the one given; its current template's camera change is the level's
    void SetCurrentLevelTemplateMgr(LevelTemplateMgr*);
    // @0x53de84 / @0x53e000: the boss gameplay starts (the level's entities wait out of the lists, the minion's deco and collision come back)
    // and ends (the entities are given back)
    void BeginBossGameplay();
    void EndBossGameplay();
    // @0x53dba0 / @0x53dd54: the large minion mode starts (its sequences become the level's templates, the minion and the runners are the
    // entities that stay) and ends (the level's own templates come back, the entities are given back)
    void BeginLargeMinionGameplay();
    void EndLargeMinionGameplay();
    // @0x53f6dc: the session of the mode starts
    void StartLargeMinionSession();
    void SetNextMinionStateChangeInfo();
    // @0x53f490: the template that holds the camera change is gone
    void OnLevelTemplateInstanceDelete(const GameplayTemplateInstance*);
    // @0x53f470
    void OnAdvanceLevelTemplateList();
    // @0x53e89c: moves what is volatile (but the platforms), the minion, the pacesetter and the planes
    void Translate(const math::vec3<float>&);
    // @0x53f500: a camera that blends over ms for the speed of 10, takes as long as that speed needs at the pacesetter's
    void SetCameraRelativeToMinionSpeed(CameraController*, unsigned ms, float referenceSpeed);
    // @0x53d724
    bool IsCinematicCameraEnabled();
    // @0x53ecf4: the places the minion can be put back on (a point adds itself when its template is made and goes with it)
    void RegisterSafePoint(SafeMinionPoint*, bool add);
    const std::vector<SafeMinionPoint*> &GetSafePoints() const { return m_safePoints; }
    // @0x53f32c: the safe point with the least distance ahead of the minion's traveled distance, in (minDistance, maxDistance); the landable
    // ones only when landableOnly. Null when none is in range.
    SafeMinionPoint *GetSafeMinionPoint(bool landableOnly, float minDistance, float maxDistance);
    void RemoveEntities();
    void UpdateEntities();
    // the entities made for the run (they go when the player is past them)
    const std::vector<GameEntity*> &GetVolatileEntities() const { return m_volatileEntities; }
    const std::vector<GameEntity*> &GetEntitiesToRender() const { return m_entitiesToRender; }
    void DeleteEntityLater(GameEntity *e) { m_entitiesToDelete.insert(e); }
    // the interactive objects that live (they add themselves when made and go when destroyed)
    void AddInteractiveObject(InteractiveObjectTemplate*);
    void RemoveInteractiveObject(InteractiveObjectTemplate*);
    // tells every one of them that the gameplay changed (their post effects go)
    void NotifyInteractiveObjectGamePlayChanged();
    void UpdateEntitiesDeletion(bool all);
    static void DeleteVolatileEntities(bool all);
    void RenderSceneMgrs();
    void UpdateCamera(int dt, bool cinematicPass);
    math::vec4<float> GetClearColor();
    jet::scene::Camera *GetCamera() { return m_camera; }
    // @0x537944: the level's physics factory (Bullet's), made when first asked
    PhysicsFactory *GetPhysicsFactory();
    // @0x537980: the level's physics world (made by the physics factory when first asked), stepped twice a frame
    PhysicsWorld *GetPhysicsWorld();
    // @0x5379b8: every pair of collisions that touch is told to both
    void UpdateCollisionPairs();
    // @0x537934: the minion reads the input
    void UpdateInput();
    // how long the run has gone on, in ms (not counted while the minion is crashed, dying or dead)
    long long GetRunTimeMs() const { return m_runTimeMs; }
    void SetCamera(jet::scene::Camera *c) { m_camera = c; }
    bool IsMovieLevel() const { return m_isMovieLevel; }
    // @0x53ecd4 / @0x53ece4: the perks and checkpoint panels of the HUD are switched by Menu_Ingame::RestartGame
    void ShowPerksUI(bool b) { m_perksUIShown = b; }
    void ShowCheckpointUI(bool b) { m_checkpointUIShown = b; }
    // the post effect definition the level shows now (what a screen fade of an effect fades back to)
    MaterialDef *GetPostFxMaterial() const { return m_postFxMaterial; }
    bool IsUpdatingEntities() const { return m_updatingEntities; }
    static jet::scene::SceneMgr *GetSceneMgr(SceneMgrId id) { return GetInstance()->m_sceneMgr[id]; }
    static jet::scene::SceneMgr *GetMenuSceneMgr() { return GetInstance()->m_sceneMgr[SceneMgr_Menu]; }
    // the tag shadow models are drawn with (Game::k_shadowTagName)
    static const jet::String &GetShadowTagName();
    // the missions and sets of missions the level files define (@0xffffbfa4, @0xffffbfa8 in the GOT)
    static std::vector<Mission*> s_missionList;
    static std::vector<MissionSet*> s_missionSetList;
    static float s_cameraFar;
    // [0] the level's, [1] the spawn probabilities', [2] the decos' and [3] a fourth one, the others reseeded from [0]
    static ControlledRandom *s_randomizer[4];
    static LevelSequenceGraphMgr *s_levelSequenceGraphMgr;
    // every entity appears whatever its probability (a cheat)
    static bool s_allProbabilityMode;
private:
    friend class Launcher;
    void InitRender();
    void EndRender() {}
    void RenderEntities();
    void RenderParticles();
    void RenderGUI();
    void RenderPostEffectStart();
    void RenderPostEffectEnd();
    void UpdatePostEffectTransition(std::vector<jet::video::PostEffect*> &out);
    struct ExtraPostFx { jet::video::PostEffect *effect; MaterialDef *def; int users; jet::core::Delayer<float, jet::core::QuadraticEaseIn> *delayer; unsigned *time; };
    std::vector<GameEntity*> m_volatileEntities;  // 0x6C
    std::vector<GameEntity*> m_entitiesToUpdate;  // 0x78
    std::vector<GameEntity*> m_entitiesToRender;  // 0x84
    std::vector<GameEntity*> m_entitiesToUpdateBackup;  // 0x90
    std::vector<GameEntity*> m_entitiesToRenderBackup;  // 0x9C
    std::vector<GameEntity*> m_entitiesToRemoveFromUpdate;  // 0xA8
    std::vector<GameEntity*> m_entitiesToRemoveFromRender;  // 0xB4
    std::unordered_set<GameEntity*> m_entitiesToDelete;  // 0xC0
    bool m_updatingEntities;  // 0xD8
    Minion *m_minion;  // 0x20C
    Pacesetter *m_pacesetter;  // 0x210
    LevelTemplateMgr m_templateMgr;  // 0xE0
    LevelTemplateMgr *m_levelTemplateMgr;  // 0x1B0
    std::vector<SafeMinionPoint*> m_safePoints;  // 0x29C
    std::map<SafeMinionPoint*, unsigned> m_safePointIndices;  // 0x2AC
    bool m_cameraChangeDone = true;  // 0x1B8
    math::plane<float> m_cameraChangePlane;  // 0x1BC: the camera of the template takes over when the camera passes it
    GameplayTemplateInstance *m_cameraChangeInstance = 0;  // 0x1CC
    bool m_sledgeBegun = true, m_sledgeEnded = true;  // 0x1D0, 0x1E8
    math::plane<float> m_sledgeBeginPlane, m_sledgeEndPlane;  // 0x1D4, 0x1EC
    int m_sledgeType = 0;  // 0x1E4
    BossGameplay *m_bossGameplay = 0;  // 0x448: the boss gameplay of the level (its boss is at 0x98 of it)
    bool m_bossBehaviorOn = false;     // 0x400: the boss's behaviour is on (Boss::SetBehaviorState)
    float m_cameraChangeOffset = 0;  // 0x1FC
    bool m_fluffyModeCamera = false, m_fastForwardCamera = false;  // 0x3C0, 0x3DC
    CameraController *m_cinematicCamera = 0;  // 0x454
    bool m_tutorialCompleted;  // 0x260
    int m_entityToSpawnCount;  // 0x630
    clara::DataEntity *m_fluffy, *m_rocket, *m_launcher, *m_gameplayDef;  // 0x228, 0x22C, 0x230, 0x238
    clara::DataEntity *m_largeMinionGameplay, *m_fluffyGameplay, *m_rocketGameplay;  // 0x43C, 0x440, 0x444
    LevelDef *m_levelDef;  // 0x234
    PhysicsFactory *m_physicsFactory;  // 0x200
    PhysicsWorld *m_physicsWorld;  // 0x204
    long long m_runTimeMs;  // 0x628
    LuaVM *m_luaVM;  // 0x23C
    bool m_loadingLibs;  // 0x240
    bool m_initDone;  // 0x69
    // the post effect
    MaterialDef *m_postFxMaterial;  // 0x2DC
    jet::video::PostEffect *m_postEffect;  // 0x2E0
    int m_fadeState;  // 0x2E4: 0 fading in, 1 none, 2 fading out, 3 holding, 4 and 5 mixing the level's effect in and out
    jet::video::PostEffect *m_nextPostEffect;  // 0x2E8
    jet::video::PostEffect *m_fadeFx;  // 0x2EC
    float m_fadeT, m_fadeInTime, m_fadeHoldTime, m_fadeOutTime;  // 0x2F0, 0x2F4, 0x2F8, 0x2FC
    int m_fadeInInterp, m_fadeOutInterp;  // 0x300, 0x304
    bool m_postFxEnabled;  // 0x308
    void (*m_postFxDone)();  // 0x30C
    std::vector<ExtraPostFx> m_extraPostFx;  // 0x310
    jet::core::Delayer<float, jet::core::SmoothStep> m_fogOn;  // 0x320
    jet::core::Delayer<math::vec4<float>, jet::core::SmoothStep> m_fogColor;  // 0x338
    float m_fogDepth[2], m_fogDepthFrom[2], m_fogDepthTo[2];  // 0x37c 0x374 0x384
    int m_fogDepthTime, m_fogDepthElapsed;  // 0x38c 0x390
    bool m_fogDepthAnim;  // 0x394
    jet::scene::Camera *m_camera;  // 0x3AC
    jet::scene::SceneMgr *m_sceneMgr[2];  // 0x3B0
    CameraMgr *m_cameraMgr;
    std::vector<Animable> m_animatedEntities;  // 0x618
    unsigned m_playerDistance;
    bool m_isMovieLevel;  // 0x66C
    bool m_perksUIShown = false;       // 0x66D
    bool m_checkpointUIShown = false;  // 0x66E
    bool m_endRunBananasAdded = false;  // 0x6A (.so): the run's bananas reach the player once, in OnMinionNoRevive
    std::set<InteractiveObjectTemplate*> m_interactiveObjects;  // 0x670
};

#endif
