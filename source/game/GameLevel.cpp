// source/game/GameLevel.cpp
#include "GameLevel.h"
#include "gamemanagers/CostumeMgr.h"
#include "gamemanagers/BonusUpgradeMgr.h"
#include "game/utils/ControlledRandom.h"
#include "Game.h"
#include "CGameConfig.h"
#include "entities/GameEntity.h"
#include "entities/GameEntityCreator.h"
#include "entities/InteractiveObjectTemplate.h"
#include "entities/LevelDef.h"
#include "entities/LightDef.h"
#include "entities/MaterialDef.h"
#include "entities/Minion.h"
#include "entities/Boss.h"
#include "entities/BossGameplay.h"
#include "entities/LargeMinionGameplay.h"
#include "gamemanagers/BackgroundMgr.h"
#include "online/tracking/SessionTrackingMgr.h"
#include "Statistics.h"
#include "entities/gameitem/GameItem.h"
#include "entities/Collision3d.h"
#include "entities/ObstacleTemplate.h"
#include "entities/ShadowLightPoint.h"
#include "entities/Deco3d.h"
#include "physics/bullet/bulletphysicsfactory.h"
#include "physics/physicsworld.h"
#include "entities/Mission.h"
#include "entities/MissionSet.h"
#include "gamemanagers/AchievementsMgr.h"
#include "gamemanagers/MissionMgr.h"
#include "entities/Pacesetter.h"
#include "entities/SafeMinionPoint.h"
#include "entities/PlatformInstance.h"
#include "gamemanagers/FlareMgr.h"
#include "gamemanagers/PowerUpMgr.h"
#include "sound/soundmgr.h"
#include "camera/cinematiccamera.h"
#include "dynamicobjects/EffectMgr.h"
#include "dynamicobjects/ModelPool.h"
#include "gamemanagers/SkyDomeMgr.h"
#include "gamemanagers/ClaraFilesMgr.h"
#include "gamemanagers/PainterMgr.h"
#include "gamestates/GameState.h"
#include "gamestates/GS_Gameplay.h"
#include "interfaces/menus/Menu_Ingame.h"
#include "Statistics.h"
#include "colorprofiles.h"
#include "leveltemplate/LevelSequenceGraph.h"
#include "leveltemplate/GameplayTemplateInstance.h"
#include "gamemanagers/BackgroundMgr.h"
#include "gamemanagers/LocationsMgr.h"
#include "entities/LocationInfoEntity.h"
#include "Player.h"
#include "interfaces/menus/Menu_Base.h"
#include "interfaces/menus/MenuMgr.h"
#include "interfaces/menus/Menu_ResultEndScreen.h"
#include "interfaces/menus/Menu_ChallengeResult.h"
#include "challenges/ChallengeMgr.h"
#include "entities/MinionCostume.h"
#include "scripting/ScriptFunctions.h"
#include "camera/cameramgr.h"
#include "luavm.h"
#include "particlemgr.h"
#include "project.h"
#include "scene/camera.h"
#include "scene/scenemgr.h"
#include "stream/istream.h"
#include "gamemanagers/MinionSpeedMgr.h"
#include "leveltemplate/GameplayTemplateInstance.h"
#include "system.h"
#include "video/display.h"
#include "video/driver.h"
#include "video/painter.h"
#include <algorithm>

float GameLevel::s_cameraFar = 0;
std::vector<Mission*> GameLevel::s_missionList;
std::vector<MissionSet*> GameLevel::s_missionSetList;
ControlledRandom *GameLevel::s_randomizer[4] = { 0, 0, 0, 0 };
LevelSequenceGraphMgr *GameLevel::s_levelSequenceGraphMgr = 0;
const jet::String GameLevel::k_fadeFx_defaultTechnique("postfx_glow_fade.xml");
const jet::String GameLevel::k_fadeFx_glowTechnique("postfx_glow_fade.xml");
// GameLevel::k_whiteTransitionSoundLabel (static initializer @0x55869c): the sound of a glow fade (SetCurrentPostEffect @0x53c9d4)
static const jet::String k_whiteTransitionSoundLabel("ev_sfx_white_transition");

// Game::k_shadowTagName @0xd0e5ac: the tag of the passes that draw into the shadow map
const jet::String &GameLevel::GetShadowTagName() {
    static const jet::String k_shadowTagName("shadowmap");
    return k_shadowTagName;
}

static void Remove(std::vector<GameEntity*> &v, GameEntity *e) {
    std::vector<GameEntity*>::iterator it = std::find(v.begin(), v.end(), e);
    if (it != v.end()) v.erase(it);
}

// @0x510630: a camera at the origin, the world and menu scene managers, the camera manager and the managers of a level
GameLevel::GameLevel(bool isMovieLevel)
    : m_updatingEntities(false), m_minion(0), m_pacesetter(0), m_templateMgr(this), m_levelTemplateMgr(&m_templateMgr), m_tutorialCompleted(false), m_entityToSpawnCount(-1), m_fluffy(0), m_rocket(0), m_launcher(0), m_gameplayDef(0), m_largeMinionGameplay(0), m_fluffyGameplay(0), m_rocketGameplay(0),
      m_levelDef(0), m_physicsFactory(0), m_physicsWorld(0), m_runTimeMs(0), m_luaVM(0), m_loadingLibs(false), m_initDone(false), m_postFxMaterial(0), m_postEffect(0), m_fadeState(1), m_nextPostEffect(0), m_fadeFx(0), m_fadeT(0),
      m_fadeInTime(0), m_fadeHoldTime(0), m_fadeOutTime(0), m_fadeInInterp(0), m_fadeOutInterp(0), m_postFxEnabled(true), m_postFxDone(0), m_fogOn(0.0f, 0.0f, 0),
      m_fogColor(math::vec4<float>(0, 0, 0, 0), math::vec4<float>(0, 0, 0, 0), 0), m_fogDepthTime(0), m_fogDepthElapsed(0), m_fogDepthAnim(false), m_camera(0), m_cameraMgr(0),
      m_playerDistance(0), m_isMovieLevel(isMovieLevel) {
    for (int i = 0; i < 4; i++) {
        if (s_randomizer[i]) continue;
        s_randomizer[i] = new ControlledRandom;
        if (i) s_randomizer[0]->AddLinked(s_randomizer[i]);
    }
    for (int i = 0; i < 2; i++) m_fogDepth[i] = m_fogDepthFrom[i] = m_fogDepthTo[i] = 0;
    const PlatformToggles &t = CGameConfig::Current();
    s_cameraFar = t.backgroundFarPlane;
    m_camera = new jet::scene::Camera(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
    m_camera->SetNearDistance(0.5f);
    m_camera->SetFarDistance(s_cameraFar);
    for (int i = 0; i < 2; i++) {
        m_sceneMgr[i] = new jet::scene::SceneMgr();
        m_sceneMgr[i]->SetCamera(m_camera);
    }
    m_sceneMgr[SceneMgr_World]->SetDistanceCulling(t.sceneMgrCullingRadius, t.sceneMgrCullingDistance);
    m_sceneMgr[SceneMgr_World]->SetScreenRadiusCulling(t.sceneMgrCullingScreenPixels);
    m_cameraMgr = new CameraMgr(m_camera);
    new BackgroundMgr(!isMovieLevel);
    new EffectMgr;  // @0x535910
    new ModelPool;  // @0x535920
    ps::ParticleMgr *pm = new ps::ParticleMgr;
    pm->Init();
    pm->SetComputeLocalspacePeriod(7);
    pm->SetMinDistance(0.3f);
    pm->SetMaxPoolSize(10);
    new FlareMgr;
    new SkyDomeMgr;
    new LocationsMgr;
    new MinionSpeedMgr;  // @0x535900
    FlareMgr::GetInstance()->Enable(true);
}

GameLevel::~GameLevel() {
    Game::GetInstance()->Save(false, false);  // @0x5363d0
    delete LocationsMgr::GetInstance();
    delete MinionSpeedMgr::GetInstance();
    delete SkyDomeMgr::GetInstance();
    delete FlareMgr::GetInstance();
    delete EffectMgr::GetInstance();
    delete ModelPool::GetInstance();
    delete ps::ParticleMgr::GetInstance();
    delete BackgroundMgr::GetInstance();
    delete m_physicsFactory;
    delete m_cameraMgr;
    delete m_postEffect;
    delete m_nextPostEffect;
    delete m_fadeFx;
    for (ExtraPostFx &e : m_extraPostFx) delete e.effect;
    for (int i = 0; i < 2; i++) delete m_sceneMgr[i];
    delete m_camera;
    delete m_luaVM;
}

// @0x5121ec: the libraries (when the project has none yet), then the level files
void GameLevel::Load(const std::map<jet::String, jet::stream::IStream*> &libs, const std::map<jet::String, jet::stream::IStream*> &levels) {
    static GameEntityCreator creator;
    m_loadingLibs = true;
    clara::Project *p = clara::Project::GetInstance();
    if (p->GetLibraryCount() == 0)
        for (auto &l : libs) {
            if (p->GetLibraryCount() == 0) p->LoadLibraries(*l.second, creator);
            else p->AddLibraries(*l.second, creator);
        }
    m_loadingLibs = false;
    for (auto &l : levels) p->LoadClaraFile(*l.second, creator);
}

// @0x5122e4: the Lua machine, once
void GameLevel::Init() {
    if (m_initDone) return;
    m_luaVM = new LuaVM;
    RegisterScriptFunctions(m_luaVM);
    m_initDone = true;
    m_pacesetter = new Pacesetter(clara::Project::GetInstance()->FindTemplateByName(Pacesetter::k_tmplName));
    m_pacesetter->SetName(jet::String("minionPacestter"));
    m_pacesetter->Init();
}

// @0x51240c: everything below the movie test (@0x536fe8 on the movie flag at +0x66C) is a level's run only; a movie level keeps none of it
void GameLevel::PostInit() {
    BackgroundMgr::GetInstance()->PostInit();
    m_pacesetter->PostInit();
    if (!m_isMovieLevel) {
        if (m_levelDef) {
            MenuMgr *mm = MenuMgr::GetInstance();
            mm->SetCameraEntity(m_levelDef->GetUIDefaultCamera3D());
            mm->SetDefault3dLightFrom(dynamic_cast<LightDef*>(m_levelDef->GetUIDefaultLight3D()));
        }
        CostumeMgr::GetInstance()->Reset();
        MissionMgr::GetInstance()->LoadGameplayDef(m_gameplayDef);
        MissionMgr::GetInstance()->Reset(s_missionList, s_missionSetList);
        AchievementsMgr::GetInstance()->Reset();
        BonusUpgradeMgr::GetInstance()->Reset();
        LocationsMgr::GetInstance()->Reset();  // @0x5370c4: the location configs for the level's gameplay
    }
    m_fadeState = 1;
    RemoveEntities();
    if (m_levelDef) m_levelDef->DefineLevel();
    m_pacesetter->SetCurrentGameplayTemplateInstance(m_templateMgr.GetCurrentLevelTemplateInstance());
    MinionSpeedMgr::GetInstance()->PostInit();  // @0x537264
    GameItem::ResetLastCreatedDistance();       // @0x537228
    Game::GetInstance()->SetCurvatureParams(math::vec2<float>(0.0f, 0.0f), 0);
    Game::GetInstance()->SetTimeSpeed(1.0f, 0, -1);
}

// @0x539408: the Lua StartGamePlay: the run begins. The camera and the speed of the first template are the level's, the minion follows the
// pacesetter from where it stands, and the time of the run counts from now
void GameLevel::StartGameplay() {
    Game *game = Game::GetInstance();
    game->SetGameplayStarted(true);
    GameplayTemplateInstance *first = m_levelTemplateMgr->GetCurrentLevelTemplateInstance();
    first->ApplyCameraValue(-1);
    first->ApplySpeedValue();
    BackgroundMgr::GetInstance()->ApplyCurrentBackgroundSound();
    m_pacesetter->RegisterFollower(m_minion);
    m_pacesetter->SetLanePosition(m_minion->GetPosition(), 0, (LaneIndex)3, 0);
    if (!m_levelTemplateMgr->GetLevelTemplateInstanceList().empty()) {
        SetNextCameraChangeInfo();
        SetNextMinionStateChangeInfo();
    }
    if (game->GetStartDistance()) m_cameraMgr->Update(99999);
    m_runTimeMs = 0;
}

void GameLevel::AddAnimatedEntity(GameEntity *e, const jet::String &animation, int nodeCount) {
    Animable a = { e, animation, nodeCount };
    m_animatedEntities.push_back(a);
}

// @0x5162f0: the entities the level keeps by their kind; library entities do not count as the level's own
void GameLevel::EntityCreated(clara::DataEntity *e) {
    if (!e) return;
    const jet::String &n = e->GetTemplateName();  // @0x53b1b4: the template's own name (an actor's base name is "Actor")
    if (n.Equals(Minion::k_tmplName)) {
        if (!m_loadingLibs && !m_minion) m_minion = static_cast<Minion*>(e);
    } else if (n.Equals("Fluffy"))
        m_fluffy = e;
    else if (n.Equals("Rocket"))
        m_rocket = e;
    else if (n.Equals("GameplayDef"))
        m_gameplayDef = e;
    else if (n.Equals("FluffyGameplay"))
        m_fluffyGameplay = e;
    else if (n.Equals("LargeMinionGameplay"))
        m_largeMinionGameplay = e;
    else if (n.Equals("RocketGameplay"))
        m_rocketGameplay = e;
    else if (n.Equals("Launcher")) {
        if (!m_loadingLibs) m_launcher = e;
    } else if (n.Equals(Mission::k_tmplName)) {
        if (std::find(s_missionList.begin(), s_missionList.end(), static_cast<Mission*>(e)) == s_missionList.end())
            s_missionList.push_back(static_cast<Mission*>(e));
    } else if (n.Equals(MissionSet::k_tmplName)) {
        if (std::find(s_missionSetList.begin(), s_missionSetList.end(), static_cast<MissionSet*>(e)) == s_missionSetList.end())
            s_missionSetList.push_back(static_cast<MissionSet*>(e));
    }
}

// @0x51473c, 0x5147a0: an entity is queued to leave a list the frame it is removed in
void GameLevel::RegisterForUpdate(GameEntity *e, bool add) {
    if (!add) {
        m_entitiesToRemoveFromUpdate.push_back(e);
        Remove(m_entitiesToUpdateBackup, e);
    } else
        m_entitiesToUpdate.push_back(e);
}

void GameLevel::RegisterForRender(GameEntity *e, bool add) {
    if (!add) {
        m_entitiesToRemoveFromRender.push_back(e);
        Remove(m_entitiesToRenderBackup, e);
    } else
        m_entitiesToRender.push_back(e);
}

void GameLevel::RegisterVolatileEntity(GameEntity *e, bool add) {
    if (add) m_volatileEntities.push_back(e);
    else Remove(m_volatileEntities, e);
}

// @0x5165e0: what was asked to leave the update and render lists
void GameLevel::RemoveEntities() {
    while (!m_entitiesToRemoveFromUpdate.empty()) {
        Remove(m_entitiesToUpdate, m_entitiesToRemoveFromUpdate.front());
        std::swap(m_entitiesToRemoveFromUpdate.front(), m_entitiesToRemoveFromUpdate.back());
        m_entitiesToRemoveFromUpdate.pop_back();
    }
    while (!m_entitiesToRemoveFromRender.empty()) {
        Remove(m_entitiesToRender, m_entitiesToRemoveFromRender.front());
        std::swap(m_entitiesToRemoveFromRender.front(), m_entitiesToRemoveFromRender.back());
        m_entitiesToRemoveFromRender.pop_back();
    }
}

// @0x516c28: volatile entities out of the scene are queued, then at most 2 (all if asked) are deleted
void GameLevel::UpdateEntitiesDeletion(bool all) {
    for (GameEntity *e : m_entitiesToUpdate)
        if (e->GetPersistenceType() == GameEntity::PersistenceTypeDef::Volatile && (all || e->IsOutOfScene()))
            m_entitiesToDelete.insert(e);
    int n = 0;
    bool any = false;
    while (!m_entitiesToDelete.empty() && (all || n < 2)) {
        GameEntity *e = *m_entitiesToDelete.begin();
        delete e;
        m_entitiesToDelete.erase(e);
        n++;
        any = true;
    }
    if (any) RemoveEntities();
}

GameLevel::GameplayHooks *GameLevel::s_gameplay = 0;
bool GameLevel::s_allProbabilityMode = false;

// @0x51a114: only the gameplay state plays them
void GameLevel::SetSound(jet::String *ambience, jet::String *music) {
    GameState *s = GameState::GetCrtState();
    if (s && s->GetStateID() == 1 && s_gameplay) s_gameplay->ApplySoundSettings(ambience, music);
}

// @0x51a15c
void GameLevel::SetCurrentMusicGain(float g) {
    GameState *s = GameState::GetCrtState();
    if (s && s->GetStateID() == 1 && s_gameplay) s_gameplay->SetCurrentMusicGain(g);
}

// @0x51b970 (the bapple spot of the new location joins with BappleMgr)
void GameLevel::OnLocationChanged(int from, int to) {
    Statistics::GetInstance()->Location_IncreaseEnterLocationCount(to);
    if (from != -1) m_minion->SetLocationEnterDistance(m_minion->GetTraveledDistance());
}

// the provider of the level template manager: the graph of the location says which template is next (the cannon mode overrides come
// with the gameplay)
void GameLevel::IncSequenceNumber() { s_levelSequenceGraphMgr->IncSequenceNumber(); }
int GameLevel::GetNextSequenceNumber() { return s_levelSequenceGraphMgr->GetNextSequenceNumber(); }
GameEntity *GameLevel::GetNextCamera() {
    return dynamic_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByName(s_levelSequenceGraphMgr->GetCameraName()));
}
int GameLevel::GetNextCameraTransitionTime() { return s_levelSequenceGraphMgr->GetCameraTransitionTime(); }
const SpeedDef *GameLevel::GetNextSpeed() { return s_levelSequenceGraphMgr->GetSpeed(); }
const jet::String &GameLevel::GetNextLocation() { return s_levelSequenceGraphMgr->GetLocation(); }
const jet::String &GameLevel::GetNextNodeLabel() { return s_levelSequenceGraphMgr->GetNextNodeLabel(); }
const jet::String &GameLevel::GetNextNodeId() { return s_levelSequenceGraphMgr->GetNextNodeId(); }
int GameLevel::GetNextNodeGraphId() { return s_levelSequenceGraphMgr->GetNextNodeGraphId(); }
void GameLevel::OnNodeCompleted(const int &graphId, const jet::String &nodeId) { s_levelSequenceGraphMgr->OnNodeCompleted(graphId, nodeId); }
bool GameLevel::IsNodeOnSpecialTree(const jet::String &id) const { return s_levelSequenceGraphMgr && s_levelSequenceGraphMgr->IsNodeInASpecialTree(id); }

// @0x51acac: the way the minion took at the last bifurcation
int GameLevel::GetFinalBifurcation() const {
    GameplayTemplateInstance *g = m_templateMgr.GetCurrentLevelTemplateInstance();
    return g ? g->GetFinalBifurcation() : 1;
}

// @0x517428
void GameLevel::DestroyGraphMgr() {
    delete s_levelSequenceGraphMgr;
    s_levelSequenceGraphMgr = 0;
}

// @0x51b5dc: the libraries of the locations the run uses stay loaded, the others go
void GameLevel::UpdateUsedDynamicLibraries() {
    ClaraFilesMgr *m = ClaraFilesMgr::GetInstance();
    m->ClearUsedLocations();
    m->AddUsedLocation(ELocation((ELocationDef::type)s_levelSequenceGraphMgr->GetNextNodeGraphId()));
    if (const GameplayTemplateInstance *c = m_templateMgr.GetCurrentLevelTemplateInstance()) m->AddUsedLocation(c->GetLocation());
    for (const GameplayTemplateInstance *g : m_templateMgr.GetLevelTemplateInstanceList()) m->AddUsedLocation(g->GetLocation());
    for (const LevelTemplateMgr::LevelTemplateData *d : m_templateMgr.GetLevelTemplateCache()) m->AddUsedLocation(d->location);
    for (int loc : BackgroundMgr::GetInstance()->GetLocationPoolArray()) m->AddUsedLocation(ELocation((ELocationDef::type)loc));
    m->EndUpdateUsedLocations();
    m->LoadLibraries();
    m->UnloadLibraries();
    if (m->UpdateUnloadingLibraries((int)(Game::GetInstance()->GetUpdateTimeUs() / 1000), -1, 0)) OnLocationTransitionLoadingFinished();
}

// @0x5179f8: the first templates are cached, the run may start further along, the first templates are made and the first background applies
void GameLevel::InitLevelStructure() {
    m_templateMgr.CacheInitialLevelSequence();
    if (unsigned start = Game::GetInstance()->GetStartDistance()) {
        const float f = (float)start * 0.9f;
        const unsigned meters = f > 0 ? (unsigned)f : 0;
        const unsigned covered = m_templateMgr.AdvanceLevelSequence(meters);
        m_minion->SetDistanceBase(meters - covered);
    }
    m_templateMgr.InitLevelParts();
    BackgroundMgr::GetInstance()->ApplySkyDome();
    BackgroundMgr::GetInstance()->ApplyFogValue();
    BackgroundMgr::GetInstance()->ApplyLensFlare();
}

// @0x51746c: the graphs of the locations the player has data for, the one of the player's location the current one
void GameLevel::LoadLevelSequence(const jet::String &firstGraph) {
    Game *game = Game::GetInstance();
    if (game->GetStartMode() != 0) DestroyGraphMgr();
    if (!s_levelSequenceGraphMgr) s_levelSequenceGraphMgr = new LevelSequenceGraphMgr;
    LevelSequenceGraphMgr *mgr = s_levelSequenceGraphMgr;
    Player *player = Player::GetInstance();
    int location = player->GetCurrentLocation();
    bool hidden = false;
    LocationsMgr *locations = LocationsMgr::GetInstance();
    if (LocationInfoEntity *info = locations->GetLocationInfo(location)) {
        if (info->IsHidden()) {
            location = info->GetFallbackLocationId();
            LocationInfoEntity *fallback = locations->GetLocationInfo(location);
            if (fallback && fallback->IsHidden()) location = 0;
            hidden = true;
        }
    }
    jet::String file = firstGraph;
    if (file.size() == 0) {
        static const char *const k_graphs[6] = { "levelsequence2_labo.graphml", "levelsequence2_resi.graphml", "levelsequence2_macho.graphml",
                                                 "levelsequence2_beach.graphml", "levelsequence2_rnt.graphml", "levelsequence2_mall.graphml" };
        for (int loc = 0; loc < 6; loc++) {
            if (mgr->HasLevelSequenceGraph(loc)) continue;
            if (!game->HasDLCDataFor(loc, false)) {
                if (location == loc) location = 0;
                continue;
            }
            mgr->AddLevelSequenceGraph(loc, jet::String(k_graphs[loc]));
        }
    }
    mgr->Reset();
    const int mode = game->GetStartMode();
    if (mode >= 2 && mode <= 6) location = mode - 2;
    else if (mode == 7) {
        player->SetCurrentLocation(5);
        location = 5;
    }
    if ((mode >= 2 && mode <= 6) || hidden) player->SetCurrentLocation(location);
    mgr->SetCurrentLevelSequenceGraph(location);
    UpdateUsedDynamicLibraries();
    InitLevelStructure();
    game->SetStartMode(0);
}

// @0x537944
PhysicsFactory *GameLevel::GetPhysicsFactory() {
    if (!m_physicsFactory) m_physicsFactory = new BulletPhysicsFactory;
    return m_physicsFactory;
}

// @0x537980: the world, made with the factory the first time, steps twice a frame
PhysicsWorld *GameLevel::GetPhysicsWorld() {
    if (!m_physicsWorld) {
        m_physicsWorld = GetPhysicsFactory()->CreateWorld();
        m_physicsWorld->SetNumberOfSimulationSteps(2);
    }
    return m_physicsWorld;
}

// A pair is still in the world while both its bodies are: BulletPhysicsWorld::RemoveBody (@0x66e168) erases the pairs of a body that
// leaves the world. Only the pointers are compared here, so a pair of a body that was deleted is never read.
static bool IsPairInWorld(const std::vector<CollisionPair> &pairs, const CollisionPair &p) {
    for (size_t i = 0; i < pairs.size(); i++)
        if (pairs[i].bodyA == p.bodyA && pairs[i].bodyB == p.bodyB) return true;
    return false;
}

// @0x5379b8: a pair is told to the collision of each body, each with the other. A callback can take an item (GameItem::Collect ->
// BeginLargeMinionGameplay -> UpdateEntitiesDeletion) and delete its collision, so the loop runs on a copy of the pairs and tells a pair
// only while the world still holds it: the pairs of a removed body are dead, and the body's collision may be freed.
void GameLevel::UpdateCollisionPairs() {
    PhysicsWorld *w = GetPhysicsWorld();
    const std::vector<CollisionPair> pairs(w->GetCollisionPairs());
    for (size_t i = 0; i < pairs.size(); i++) {
        const CollisionPair &p = pairs[i];
        if (!IsPairInWorld(w->GetCollisionPairs(), p)) continue;
        Collision3d *a = static_cast<Collision3d*>(p.bodyA->GetUserData());
        Collision3d *b = static_cast<Collision3d*>(p.bodyB->GetUserData());
        a->CollidesWith(b, p);
        if (!IsPairInWorld(w->GetCollisionPairs(), p)) continue;
        b->CollidesWith(a, p);
    }
}

// @0x537934
void GameLevel::UpdateInput() {
    if (m_minion) m_minion->UpdateInput();
}

// @0x516a68 (the level-part cleanup joins once LevelTemplateMgr exists)
void GameLevel::DeleteVolatileEntities(bool all) {
    GameLevel *l = GetInstance();
    if (!l) return;
    if (all) l->RemoveEntities();
    l->UpdateEntitiesDeletion(true);
    if (!all) return;
    if (!l->m_entitiesToUpdateBackup.empty()) {
        l->RemoveEntities();
        l->m_entitiesToUpdate = l->m_entitiesToUpdateBackup;
        l->m_entitiesToRender = l->m_entitiesToRenderBackup;
        l->m_entitiesToUpdateBackup.clear();
        l->m_entitiesToRenderBackup.clear();
        l->UpdateEntitiesDeletion(true);
    }
    std::vector<GameEntity*> vol(l->m_volatileEntities);
    for (GameEntity *e : vol) delete e;
}

// @0x51289c's entity pass: every registered entity updates once, on a copy so they can register others
void GameLevel::UpdateEntities() {
    RemoveEntities();
    m_updatingEntities = true;
    std::vector<GameEntity*> list(m_entitiesToUpdate);
    for (size_t i = 0; i < list.size(); i++) list[i]->Update();
    m_updatingEntities = false;
    RemoveEntities();
}

// @0x51289c: the project's movies, the entities, the scripts and the camera
void GameLevel::Update(bool fullUpdate) {
    Game *g = Game::GetInstance();
    int dt = (int)(g->GetUpdateTimeUs() / 1000);
    GameEntity::s_frameDt = g->IsPaused() ? 0 : dt;
    m_camera = m_cameraMgr->GetSceneCamera();
    if (!g->IsPaused()) {
        clara::Project::GetInstance()->Update(dt);
        // the time of the run counts while the minion goes on (not while it falls in a pit: the .so's Minion+0x262)
        if (!m_isMovieLevel && !m_minion->IsCrashed() && !m_minion->IsDead() && !m_minion->IsDying() && !m_minion->IsFallingInPit()) m_runTimeMs += dt;
    }
    if (fullUpdate) {
        // the physics steps, what touches is told, then the minion reads the input
        if (!g->IsPaused()) {
            GetPhysicsWorld()->Update(dt);
            UpdateCollisionPairs();
            RemoveEntities();
        }
        UpdateInput();
    }
    BackgroundMgr::GetInstance()->Update();
    RemoveEntities();
    UpdateEntities();
    if (!IsFluffyMode()) {
        UpdateEntitiesDeletion(false);
        m_templateMgr.UpdateLevelParts();
        RemoveEntities();
    }
    PowerUpMgr::GetInstance()->Update();  // @0x5375c4: the power ups run down on every update, the pause included
    if (m_luaVM) m_luaVM->Update((unsigned)dt);
    FlareMgr::GetInstance()->Update((unsigned)dt);  // @0x5375e4
    UpdateCamera(dt, false);
    ClaraFilesMgr::GetInstance()->UpdateUnloadingLibraries(dt, 1, 500);
}

// @0x512ca4 (the .so's UpdateCamera is @0x537810): the camera manager runs when the cinematic camera is on for the render's pass (true) and
// off for the update's (false). With a cinematic camera, the minion's transform is given to it first. The time is zero while the game is
// paused, in a cinematic, or while the resume countdown of the HUD runs.
void GameLevel::UpdateCamera(int dt, bool cinematicPass) {
    Game *g = Game::GetInstance();
    if (g->IsPaused() || g->IsCinematic()) dt = 0;
    else if (GS_Gameplay *gs = GS_Gameplay::GetInstance()) {
        if (Menu_Ingame *hud = gs->GetInterface())
            if (hud->IsCountdownEnabled()) dt = 0;
    }
    if (IsCinematicCameraEnabled() != cinematicPass) return;
    if (m_cinematicCamera && m_minion) static_cast<CinematicCamera*>(m_cinematicCamera)->SetLocalTransform(m_minion->GetPosition(), m_minion->GetRotation());
    m_cameraMgr->Update((unsigned)dt);
}

bool GameLevel::IsCamera(const clara::Entity *e) const {
    if (!e) return false;
    const jet::String &n = e->GetTemplateName();
    return n.Equals("CameraDirect") || n.Equals("CameraCinematic") || n.Equals("CameraFollow") || n.Equals("CameraFluffy");
}

// @0x51aabc
void GameLevel::SetCamera(CameraController *c, unsigned ms) { m_cameraMgr->SetCamera(c, ms, 0); }

void GameLevel::SetFog(bool on, unsigned ms) {
    m_fogOn.SetDelay((int)ms);
    m_fogOn.SetTargetValue(on ? 1.0f : 0.0f);
}

void GameLevel::SetFogColor(const jet::video::Color &c, unsigned ms) {
    m_fogColor.SetDelay((int)ms);
    m_fogColor.SetTargetValue(math::vec4<float>(c.r, c.g, c.b, c.a));
}

// @0x5187bc: the depths blend from where they are to the new ones
void GameLevel::SetFogDepth(float start, float end, unsigned ms) {
    m_fogDepthTime = (int)ms;
    if (ms == 0) {
        m_fogDepthAnim = false;
        m_fogDepthElapsed = 0;
        m_fogDepthTo[0] = m_fogDepth[0];
        m_fogDepthTo[1] = m_fogDepth[1];
    }
    if ((int)ms < 1) {
        m_fogDepth[0] = m_fogDepthTo[0] = start;
        m_fogDepth[1] = m_fogDepthTo[1] = end;
        m_fogDepthAnim = false;
    } else if (m_fogDepthTo[0] != start || m_fogDepthTo[1] != end) {
        m_fogDepthElapsed = 0;
        m_fogDepthFrom[0] = m_fogDepth[0];
        m_fogDepthFrom[1] = m_fogDepth[1];
        m_fogDepthTo[0] = start;
        m_fogDepthTo[1] = end;
        m_fogDepthAnim = !(m_fogDepth[0] == start && m_fogDepth[1] == end);
    }
}

// @0x513bdc: the driver's fog follows the level's, fading with the "on" factor
void GameLevel::RenderFog() {
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    const int dt = (int)Game::GetInstance()->GetRenderTime();
    m_fogOn.Update(dt);
    m_fogColor.Update(dt);
    if (m_fogDepthAnim) {
        m_fogDepthElapsed += dt;
        if (m_fogDepthElapsed < m_fogDepthTime) {
            if (m_fogDepthElapsed < 0) {
                m_fogDepth[0] = m_fogDepthFrom[0];
                m_fogDepth[1] = m_fogDepthFrom[1];
            } else {
                float t = jet::core::SmoothStep((float)m_fogDepthElapsed / (float)m_fogDepthTime).v;
                m_fogDepth[1] = t * m_fogDepthTo[1] + (1.0f - t) * m_fogDepthFrom[1];
                m_fogDepth[0] = t * m_fogDepthTo[0] + (1.0f - t) * m_fogDepthFrom[0];
            }
        } else {
            m_fogDepth[0] = m_fogDepthTo[0];
            m_fogDepth[1] = m_fogDepthTo[1];
            m_fogDepthAnim = false;
        }
    }
    float on = m_fogOn.GetCurrentValue();
    bool enabled = on > 0.0f;
    drv->SetFog(enabled);
    if (enabled) {
        math::vec4<float> c = m_fogColor.GetCurrentValue();
        jet::video::Color col(c.x, c.y, c.z, c.w * on);
        drv->SetFogParams(col, m_fogDepth[0], m_fogDepth[1]);
    }
}

// @0x5146f4: black in a movie level, else the fog colour
math::vec4<float> GameLevel::GetClearColor() {
    if (m_isMovieLevel) return math::vec4<float>(0, 0, 0, 1);
    return m_fogColor.GetCurrentValue();
}

void GameLevel::SetCurrentPostEffect(jet::video::PostEffect *pe) {
    delete m_postEffect;
    m_postEffect = pe;
}

// @0x53c9d4 (the citation was 0x517d14, which is another function)
void GameLevel::SetCurrentPostEffect(MaterialDef *def, bool fade, const jet::String &technique, int fadeOutMs, int holdMs, int fadeInMs,
                                     jet::core::InterpolationDef::type outInterp, jet::core::InterpolationDef::type inInterp, void (*onDone)()) {
    if (m_postFxDone) {
        void (*f)() = m_postFxDone;
        m_postFxDone = 0;
        f();
    }
    m_postFxDone = onDone;
    m_postFxMaterial = def;
    jet::video::PostEffect *pe = 0;
    if (def) {
        jet::video::Material m;
        std::vector<std::vector<jet::String> > names;
        def->LoadMaterial(m, names);
        pe = new jet::video::PostEffect(m);
    }
    delete m_nextPostEffect;
    m_nextPostEffect = 0;
    if (!fade) {
        if (m_fadeState == 2) m_nextPostEffect = pe;
        else SetCurrentPostEffect(pe);
        return;
    }
    // a glow fade is announced by its sound (@0x53cac0: SoundMgr::Play of the white transition, 0 ms)
    if (technique == k_fadeFx_glowTechnique) SoundMgr::GetInstance()->Play(k_whiteTransitionSoundLabel, 0);
    const jet::String tech = MaterialDef::s_postFxEnabled ? technique : jet::String("basic.xml");
    if (!m_fadeFx) m_fadeFx = new jet::video::PostEffect(tech);
    else m_fadeFx->SetRenderTechnique(tech);
    m_nextPostEffect = pe;
    m_fadeInInterp = (int)inInterp;
    m_fadeOutInterp = (int)outInterp;
    m_fadeState = 2;
    m_fadeOutTime = (float)fadeOutMs * 0.001f;
    m_fadeInTime = (float)fadeInMs * 0.001f;
    m_fadeHoldTime = (float)holdMs * 0.001f;
}

// @0x517a94
void GameLevel::AddExtraPostEffect(MaterialDef *def, jet::core::Delayer<float, jet::core::QuadraticEaseIn> *delayer, unsigned *time) {
    for (ExtraPostFx &e : m_extraPostFx)
        if (e.def == def) {
            e.users++;
            return;
        }
    jet::video::Material m;
    std::vector<std::vector<jet::String> > names;
    def->LoadMaterial(m, names);
    ExtraPostFx e = { new jet::video::PostEffect(m), def, 1, delayer, time };
    m_extraPostFx.push_back(e);
}

// @0x517c30: the effect goes with its last user
void GameLevel::RemoveExtraPostEffect(MaterialDef *def) {
    for (size_t i = 0; i < m_extraPostFx.size(); i++)
        if (m_extraPostFx[i].def == def) {
            if (--m_extraPostFx[i].users == 0) {
                delete m_extraPostFx[i].effect;
                m_extraPostFx.erase(m_extraPostFx.begin() + i);
            }
            return;
        }
}

// @0x518010: the fades of the post effect, a step each frame
void GameLevel::UpdatePostEffectTransition(std::vector<jet::video::PostEffect*> &out) {
    static const jet::String k_fxTime("fxTime"), k_mixAmount("mixAmount");
    if (m_fadeState == 1) return;
    Game *g = Game::GetInstance();
    float dt = 0.0f;
    if (!g->IsPaused() && !g->IsCinematic()) dt = (float)g->GetRenderTime() * 0.001f;
    auto setUniform = [](jet::video::Material &m, unsigned pass, const jet::String &name, float v) {
        int idx = m.FindUniformIdxByName(pass, name);
        if (idx >= 0) m.SetUniform(pass, (unsigned)idx, jet::video::ShaderUniform(v));
    };
    switch (m_fadeState) {
    case 0: {
        m_fadeT -= dt;
        float f = jet::core::GetInterpolated(jet::core::Interpolation((int)m_fadeInInterp), math::clamp(m_fadeT / m_fadeInTime, 0.0f, 1.0f));
        setUniform(m_fadeFx->GetMaterial(), 0, k_fxTime, f);
        out.push_back(m_fadeFx);
        if (m_fadeT <= 0.0f) {
            m_fadeT = 0;
            m_fadeState = 1;
        }
        break;
    }
    case 2: {
        m_fadeT += dt;
        float f = jet::core::GetInterpolated(jet::core::Interpolation((int)m_fadeOutInterp), math::clamp(m_fadeT / m_fadeOutTime, 0.0f, 1.0f));
        setUniform(m_fadeFx->GetMaterial(), 0, k_fxTime, f);
        out.push_back(m_fadeFx);
        if (m_fadeOutTime <= m_fadeT) {
            m_fadeT = m_fadeHoldTime;
            m_fadeState = 3;
            SetCurrentPostEffect(m_nextPostEffect);
            m_nextPostEffect = 0;
            if (m_postFxDone) {
                void (*f)() = m_postFxDone;
                m_postFxDone = 0;
                f();
            }
        }
        break;
    }
    case 3:
        m_fadeT -= dt;
        out.push_back(m_fadeFx);
        if (m_fadeT <= 0.0f) {
            m_fadeT = m_fadeInTime;
            m_fadeState = 0;
        }
        break;
    default:
        break;
    }
}

// @0x513bdc's neighbours: the frame's passes
void GameLevel::InitRender() { LightMgr::GetInstance()->Process(); }

// @0x512ec4: the backgrounds and the entities draw themselves into the scene manager
void GameLevel::RenderEntities() {
    BackgroundMgr::GetInstance()->Render();
    std::vector<GameEntity*> list(m_entitiesToRender);
    for (GameEntity *e : list) e->Render();
    ModelPool::GetInstance()->Render(m_sceneMgr[SceneMgr_World], 0);  // @0x537a9c: the models of the level's effects
}

// @0x512f40
void GameLevel::RenderSceneMgrs() {
    m_sceneMgr[SceneMgr_World]->SetCamera(m_camera);
    m_sceneMgr[SceneMgr_World]->Render();
}

// @0x512f70: the particle systems seen from the camera
void GameLevel::RenderParticles() {
    jet::scene::Camera *cam = m_cameraMgr->GetSceneCamera();
    ps::CameraData cd;
    cd.position = cam->GetPosition();
    cd.front = cam->GetFrontVector();
    cd.up = cam->GetUpVector();
    cd.right = cam->GetRightVector();
    const math::vec2<unsigned int> viewport = cam->GetViewportSize();
    cd.width = (int)viewport.x;
    cd.height = (int)viewport.y;
    cd.fov = cam->GetFov();
    cd.orthogonal = cam->IsOrthogonal();
    Game *g = Game::GetInstance();
    unsigned dt = g->GetRenderTime();
    if (g->IsPaused() || g->IsCinematic()) dt = 0;
    ps::ParticleMgr::GetInstance()->Update(dt);
    ps::ParticleMgr::GetInstance()->Render(cd);
}

// @0x51313c: the menu over the scene, then the painters
void GameLevel::RenderGUI() {
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    drv->Flush();
    PainterMgr *pm = PainterMgr::GetInstance();
    jet::video::Painter &painter = pm->Get2DPainter();
    MenuMgr *mm = MenuMgr::GetInstance();
    if (mm->GetCurrentPlayingMenu()) mm->Render(painter);
    pm->Get3DPainter().Flush();
    painter.Flush();
}

// @0x51331c: the post effects of the level
void GameLevel::RenderPostEffectStart() {
    static const jet::String k_nearFar("nearFar");
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    std::vector<jet::video::PostEffect*> list;
    UpdatePostEffectTransition(list);
    if (m_postFxEnabled && MaterialDef::s_postFxEnabled) {
        if (m_postEffect) {
            jet::video::Material &m = m_postEffect->GetMaterial();
            if (m.GetRenderPassCount() > 1) {
                int idx = m.FindUniformIdxByName(1, k_nearFar);
                if (idx >= 0) {
                    jet::scene::Camera *cam = m_cameraMgr->GetSceneCamera();
                    m.SetUniform(1, (unsigned)idx, jet::video::ShaderUniform(math::vec2<float>(cam->GetNearDistance(), cam->GetFarDistance())));
                }
            }
            list.push_back(m_postEffect);
        }
        for (ExtraPostFx &e : m_extraPostFx) list.push_back(e.effect);
        drv->BeginPostEffect(list, jet::video::PostFXAADef::None);
    }
}

// @0x513574: ends the post effects; without them the fades are quads over the screen
void GameLevel::RenderPostEffectEnd() {
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    drv->EndPostEffect(math::mat4<float>());
    if (MaterialDef::s_postFxEnabled || m_fadeState == 1) return;
    drv->Flush();
    unsigned color = 0xffffffff;
    float amount = 0;
    bool blend = true;
    switch (m_fadeState) {
    case 0:
        amount = jet::core::GetInterpolated(jet::core::Interpolation((int)m_fadeInInterp), math::clamp(m_fadeT / m_fadeInTime, 0.0f, 1.0f));
        color = 0x80ffffff;
        break;
    case 2:
        amount = jet::core::GetInterpolated(jet::core::Interpolation((int)m_fadeOutInterp), math::clamp(m_fadeT / m_fadeOutTime, 0.0f, 1.0f));
        color = 0x80ffffff;
        break;
    case 3: blend = false; break;
    default: break;
    }
    jet::video::Painter &p = PainterMgr::GetInstance()->GetGenericPainter();
    jet::video::Material m;
    m.SetRenderTechnique(jet::String("_primitive2"));
    jet::video::RenderState &rs = m.GetRenderState(0);
    rs.SetBlending(true);
    rs.SetBlendFormula(jet::video::BlendFormula(jet::video::BlendFormula::ComboDef::Alpha));
    rs.SetCulling(false);
    p.SetMaterial(m);
    p.SetTransform(math::mat4<float>());
    math::vec2<unsigned int> sz = jet::System::GetDisplay(0)->GetSize();
    float w = (float)sz.x, h = (float)sz.y;
    float a = blend ? amount * 255.0f : 255.0f;
    jet::video::Color c(1, 1, 1, std::max(0.0f, std::min(1.0f, a / 255.0f)));
    (void)color;
    p.FillQuad(jet::video::Painter::Vertex(math::vec2<float>(0, 0), c), jet::video::Painter::Vertex(math::vec2<float>(w, 0), c),
               jet::video::Painter::Vertex(math::vec2<float>(w, h), c), jet::video::Painter::Vertex(math::vec2<float>(0, h), c));
    p.Flush();
}

// @0x513da4: one frame of the level, then the menu
// @0x53d4b0: the run's records. The distance raises the greatest distance (Statistics::RaiseDistanceRecord), a better score raises the best
// score, and the save is written when a record changed and the caller did not ask to skip it.
void GameLevel::UpdateEndGameStatistics(bool skipSave) {
    Statistics *st = Statistics::GetInstance();
    bool changed = st->RaiseDistanceRecord(st->GetIntValue(-1, StatisticsSaveData::k_statScope_distance, 0));
    const long long score = st->GetScore();
    if (score > st->GetBestScore()) {
        st->SetStatValueIfBetter(StatisticsSaveData::k_statScope_bestScore, score);
        changed = true;
    }
    // The .so reads Statistics::GetScoreMultiplier twice and compares the two reads (the same function, so the same value): this
    // branch is kept as the .so has it, and is not taken while the multiplier does not change between the two reads.
    const int multiplier = st->GetScoreMultiplier();
    if (multiplier > st->GetScoreMultiplier()) {
        st->SetStatValueIfBetter(StatisticsSaveData::k_statScope_maxReachedScoreMultiplier, multiplier);
        changed = true;
        st->IncreaseStatistic(StatisticsSaveData::k_statScope_beatScoreMultiplier, 1);
    }
    const int costumesBought = st->GetIntValue(-1, StatisticsSaveData::k_statScope_costumesBuyCount, 0);
    st->SetLocationStatValue(-1, static_cast<EStatisticType>(0), StatisticsSaveData::k_statScope_costumesBuyCount, (long long)costumesBought);
    // The .so also calls WeeklyChallengeMgr::UpdateWeeklyChallengeScore (@0x42edc4) here. That manager is online (the weekly social
    // event) and is not ported (Game.cpp leaves it out too), so the call is not made.
    if (changed && !skipSave) Game::GetInstance()->Save(false, false);
}

// @0x53db38
void GameLevel::RestoreGameplayCamera(int value) {
    LevelTemplateMgr *mgr = m_levelTemplateMgr;
    if (mgr && !mgr->GetLevelTemplateInstanceList().empty()) {
        GameplayTemplateInstance *first = mgr->GetLevelTemplateInstanceList().front();
        if (first) {
            SetCameraChangeInfo(first);
            if (CheckCameraChange()) return;
        }
    }
    mgr->GetCurrentLevelTemplateInstance()->ApplyCameraValue(value);
}

// @0x53ee5c: the minion runs again after its revive. The camera is taken back to the gameplay's and snapped to it (Update(99999)).
void GameLevel::OnMinionRevive() {
    EffectMgr::GetInstance()->CleanUpAutoRemoveEffects(true);
    RestoreGameplayCamera(0);
    m_minion->Revive();
    m_cameraMgr->GetCurrentController()->Update(99999);
    Statistics::GetInstance()->IncreaseLocationStat(-1, StatisticsSaveData::k_statScope_reviveCount, 1);
    for (GameEntity *e : m_entitiesToUpdate) e->OnMinionRevive();
}

// @0x53ef1c: the run ends without a revive (Menu_DeathRevive's skip, or GS_EndGame when the level allows no revive). The player's current
// location becomes the background's, the run's bananas reach the player once (the costume's factor adds the extra bananas to the
// statistics only), the run's statistics close (UpdateEndGameStatistics), the run distance resets, the result screen is pushed (or the
// challenge result when a challenge is on), the last run values are stored and the costume manager is marked as reset.
// Not ported: MotionMgr::EnableMotionDevice(false) (the device motion is not in this tree); the leaderboard refresh of the logged-in players
// (no player is logged in to a network here: Menu_Social's login checks are false); the death event of the tracking (its TRACKING_RESULT
// enum is not in this tree).
void GameLevel::OnMinionNoRevive() {
    Statistics *st = Statistics::GetInstance();
    Player::GetInstance()->SetCurrentLocation(BackgroundMgr::GetInstance()->GetLocation());
    if (!m_endRunBananasAdded) {
        m_endRunBananasAdded = true;
        const float bananas = (float)st->GetIntValue(-1, StatisticsSaveData::k_statScope_bananas, 0);
        const MinionCostume *costume = CostumeMgr::GetInstance()->GetEquippedCostumeRef();
        const float total = bananas * (costume ? costume->GetExtraBananasFactor() : 1.0f);
        Player::GetInstance()->AddBananas((int)total);
        st->AddBananas((int)(total - bananas), false);
    }
    st->IncreaseStatistic(StatisticsSaveData::k_statScope_playXRuns, 1);
    UpdateEndGameStatistics(false);
    Game::GetInstance()->SetStartDistance(0);
    MenuMgr *menus = MenuMgr::GetInstance();
    Menu_ResultEndScreen *result = static_cast<Menu_ResultEndScreen*>(menus->GetMenu(Menu_ResultEndScreen::k_menuName));
    result->SetFirstShow();
    const jet::String *screen = &Menu_ResultEndScreen::k_menuName;
    ChallengeMgr *challenges = ChallengeMgr::GetInstance();
    if (challenges->Challenge_IsActive()) {
        if (challenges->Challenge_IsCompleted()) st->IncreaseStatistic(StatisticsSaveData::k_statScope_challengesWonCount, 1);
        screen = &Menu_ChallengeResult::k_menuName;
    }
    menus->PushMenu(*screen);
    st->UpdateLastRunValues();
    CostumeMgr::GetInstance()->MarkWasReset();
}

void GameLevel::Render() {
    RemoveEntities();
    m_animatedEntities.clear();
    Game *g = Game::GetInstance();
    Menu_Base *menu = MenuMgr::GetInstance()->GetCurrentPlayingMenu();
    if (!menu || menu->RenderGameInBG()) {
        InitRender();
        RenderFog();
        RenderEntities();
        UpdateCamera((int)g->GetRenderTime(), true);
        // @0x538e5c: the effects of the level go on at the speed the frames are drawn, except while the run counts down to start
        if (!g->IsPaused()) {
            GS_Gameplay *gameplay = GS_Gameplay::GetInstance();
            if (!gameplay || !gameplay->GetInterface()->IsCountdownEnabled()) EffectMgr::GetInstance()->Update(0, (int)g->GetRenderTime());
        }
        EffectMgr::GetInstance()->Render(0);
        g->RenderShadowMap(m_sceneMgr[SceneMgr_World]);
        jet::video::Driver *drv = jet::video::Driver::GetCurrent();
        if (g->GetDownsampleRT()) {
            drv->SetCrtRenderTarget(g->GetDownsampleRT());
            drv->Flush();
        } else
            drv->SetCrtRenderTarget(jet::System::GetDisplay(0)->GetRenderTarget());
        RenderPostEffectStart();
        RenderSceneMgrs();
        RenderParticles();
        RenderPostEffectEnd();
        FlareMgr::GetInstance()->Render();  // @0x538d34
        RenderGUI();
        EndRender();
    } else
        RenderGUI();
}

// @0x53fce4
void GameLevel::AddInteractiveObject(InteractiveObjectTemplate *o) { m_interactiveObjects.insert(o); }

// @0x53fd84
void GameLevel::RemoveInteractiveObject(InteractiveObjectTemplate *o) { m_interactiveObjects.erase(o); }

// @0x53fe04
void GameLevel::NotifyInteractiveObjectGamePlayChanged() {
    for (InteractiveObjectTemplate *o : m_interactiveObjects)
        if (o) o->OnGamePlayChanged();
}

static const float k_degToRad = 0.017453292f;

// where a template starts and the way it leads: the plane through the start (moved offset along it) that faces along the template
static void TemplateStartPlane(const GameplayTemplateInstance *g, float offset, math::plane<float> &plane) {
    math::vec3<float> start(0, 0, 0);
    float angle = 0;
    g->GetStartAnchor(start, angle);
    const math::quat<float> rot(math::quat<float>::axisZ, angle * k_degToRad);
    const math::vec3<float> dir = math::transform(rot.getAsMat3(), math::vec3<float>(1, 0, 0));
    plane.setPlane(start + dir * offset, dir);
}

// @0x53bdc0
void GameLevel::SetCameraChangeInfo(GameplayTemplateInstance *g) {
    m_cameraChangeInstance = g;
    m_cameraChangeDone = false;
    TemplateStartPlane(g, m_cameraChangeOffset, m_cameraChangePlane);
}

// @0x53bdb0: the camera of the next template (the first of the list) takes over
void GameLevel::SetNextCameraChangeInfo() { SetCameraChangeInfo(m_levelTemplateMgr->GetLevelTemplateInstanceList().front()); }

// @0x53e96c
void GameLevel::SetCurrentLevelTemplateMgr(LevelTemplateMgr *mgr) {
    m_levelTemplateMgr = mgr;
    m_cameraChangeInstance = mgr->GetCurrentLevelTemplateInstance();
    if (!mgr->GetLevelTemplateInstanceList().empty()) SetNextCameraChangeInfo();
}

// @0x53de84: the boss gameplay begins. The entities of the update and render lists are kept aside (the obstacles and collisions leave the
// world), the minion's deco and collision are registered again, the boss gameplay starts and the templates are removed faster.
void GameLevel::BeginBossGameplay() {
    NotifyInteractiveObjectGamePlayChanged();
    EffectMgr::GetInstance()->CleanUpAutoRemoveEffects(true);
    UpdateEntitiesDeletion(false);
    RemoveEntities();
    m_entitiesToUpdateBackup = m_entitiesToUpdate;
    m_entitiesToRenderBackup = m_entitiesToRender;
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it) {
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->RemoveFromWorld();
        if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(*it)) o->OnWorldChange();
    }
    m_entitiesToUpdate.clear();
    m_entitiesToRender.clear();
    RegisterForRender(m_minion, true);
    RegisterForRender(m_minion->GetDeco(), true);
    RegisterForRender(ShadowLightPoint::GetInstance(), true);
    if (Deco3d *dome = SkyDomeMgr::GetInstance()->GetDome()) RegisterForRender(dome, true);
    RegisterForUpdate(m_minion, true);
    RegisterForUpdate(m_minion->GetDeco(), true);
    RegisterForUpdate(m_minion->GetCollision(), true);
    m_minion->GetCollision()->AddToWorld();
    m_bossGameplay->Start();
    RemoveEntities();
    m_levelTemplateMgr->SetFastForward(true);
}

// @0x53e000: the boss gameplay ends: the templates go back to normal speed, the level's own template manager is the current one again, the
// boss gameplay ends, and the entities kept aside by BeginBossGameplay are given back (the collisions join the world again)
void GameLevel::EndBossGameplay() {
    m_levelTemplateMgr->SetFastForward(false);
    EffectMgr::GetInstance()->CleanUpAutoRemoveEffects(true);
    SetCurrentLevelTemplateMgr(&m_templateMgr);
    RemoveEntities();
    m_bossGameplay->End();
    RemoveEntities();
    DeleteVolatileEntities(false);
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it) {
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->RemoveFromWorld();
        if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(*it)) o->OnWorldChange();
    }
    m_entitiesToUpdate = m_entitiesToUpdateBackup;
    m_entitiesToRender = m_entitiesToRenderBackup;
    m_entitiesToUpdateBackup.clear();
    m_entitiesToRenderBackup.clear();
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it)
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->AddToWorld();
}

// @0x53dba0: the large minion mode begins. The same entities are kept aside as the boss gameplay keeps (the obstacles and collisions leave the
// world, the minion's deco and collision are registered again), then the mode's level template manager becomes the level's current one and
// the mode starts on the background's location; its session starts and the templates are removed faster.
void GameLevel::BeginLargeMinionGameplay() {
    EffectMgr::GetInstance()->CleanUpAutoRemoveEffects(true);
    NotifyInteractiveObjectGamePlayChanged();
    UpdateEntitiesDeletion(false);
    RemoveEntities();
    m_entitiesToUpdateBackup = m_entitiesToUpdate;
    m_entitiesToRenderBackup = m_entitiesToRender;
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it) {
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->RemoveFromWorld();
        if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(*it)) o->OnWorldChange();
    }
    m_entitiesToUpdate.clear();
    m_entitiesToRender.clear();
    RegisterForRender(m_minion, true);
    RegisterForRender(m_minion->GetDeco(), true);
    RegisterForRender(ShadowLightPoint::GetInstance(), true);
    if (Deco3d *dome = SkyDomeMgr::GetInstance()->GetDome()) RegisterForRender(dome, true);
    RegisterForUpdate(m_minion, true);
    RegisterForUpdate(m_minion->GetDeco(), true);
    RegisterForUpdate(m_minion->GetCollision(), true);
    m_minion->GetCollision()->AddToWorld();
    LargeMinionGameplay *lmg = static_cast<LargeMinionGameplay*>(m_largeMinionGameplay);
    SetCurrentLevelTemplateMgr(lmg->GetLevelTemplateMgr());
    lmg->Start(ELocation(BackgroundMgr::GetInstance()->GetLocation()));
    StartLargeMinionSession();
    RemoveEntities();
    m_levelTemplateMgr->SetFastForward(true);
}

// @0x53dd54: the large minion mode ends: the templates go back to normal speed and to the level's own manager, the mode and its session end,
// and the entities kept aside by BeginLargeMinionGameplay are given back (the collisions join the world again). The next template's minion
// state change is set up when there is one.
void GameLevel::EndLargeMinionGameplay() {
    EffectMgr::GetInstance()->CleanUpAutoRemoveEffects(true);
    m_levelTemplateMgr->SetFastForward(false);
    SetCurrentLevelTemplateMgr(&m_templateMgr);
    static_cast<LargeMinionGameplay*>(m_largeMinionGameplay)->End();
    // @0x53f798 (EndLargeMinionSession) is not ported: it sends the mode's minigame tracking event (GameTrackingMgr::SendMinigameInteractionEvent,
    // which has no body in the port and a different signature), and the session itself is not otherwise changed there.
    RemoveEntities();
    DeleteVolatileEntities(false);
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it) {
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->RemoveFromWorld();
        if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(*it)) o->OnWorldChange();
    }
    m_entitiesToUpdate = m_entitiesToUpdateBackup;
    m_entitiesToRender = m_entitiesToRenderBackup;
    m_entitiesToUpdateBackup.clear();
    m_entitiesToRenderBackup.clear();
    for (std::vector<GameEntity*>::iterator it = m_entitiesToUpdate.begin(); it != m_entitiesToUpdate.end(); ++it)
        if (Collision3d *c = dynamic_cast<Collision3d*>(*it)) c->AddToWorld();
    if (!m_levelTemplateMgr->GetLevelTemplateInstanceList().empty()) SetNextMinionStateChangeInfo();
}

// @0x53f6dc: the session of the large minion mode starts over, and the statistics of the mode's type (6) are reset. The .so also keeps the run
// session's running time here for the mode's minigame tracking event, which is not in the port (see EndLargeMinionSession).
void GameLevel::StartLargeMinionSession() {
    game::common::SessionTrackingMgr::GetInstance()->AddSession("LARGE_MINION")->Restart();
    Statistics::GetInstance()->ResetStatisticsFor(6);
}

// @0x53bea0: a bonus template after the current one starts the minion's sledge at its start, the current bonus template followed by a
// regular one ends it at its start (each offset by the template's own distance)
void GameLevel::SetNextMinionStateChangeInfo() {
    GameplayTemplateInstance *next = m_levelTemplateMgr->GetLevelTemplateInstanceList().front();
    GameplayTemplateInstance *current = m_levelTemplateMgr->GetCurrentLevelTemplateInstance();
    const bool nextBonus = next->IsBonusTemplate();
    if (!current->IsBonusTemplate() && nextBonus) {
        m_sledgeBegun = false;
        TemplateStartPlane(next, next->GetBeginMinionSledgeOffset(), m_sledgeBeginPlane);
        m_sledgeType = next->GetSledgeType();
    } else if (current->IsBonusTemplate() && !nextBonus) {
        m_sledgeEnded = false;
        TemplateStartPlane(next, next->GetEndMinionSledgeOffset(), m_sledgeEndPlane);
    } else
        m_sledgeBegun = m_sledgeEnded = true;
}

// @0x53d724
bool GameLevel::IsCinematicCameraEnabled() { return m_cinematicCamera && m_cinematicCamera == m_cameraMgr->GetCurrentController(); }

// @0x53bc9c: the sledge begins when the minion (not frozen, riding, sledging nor large, and no boss) passes the begin plane, and ends when it
// passes the end plane. (The original also requires the field at 0x214 to be zero; the constructor is the only writer of it in the .so.)
void GameLevel::CheckMinionStateChange() {
    GameLevel *level = GameLevel::GetInstance();
    if (!m_sledgeBegun && level && level->m_minion) {
        Minion *m = level->m_minion;
        // (the .so tests the boss gameplay (+0x448) itself, not the boss in it: a gameplay without a boss yet holds it too)
        if (!m->IsFrozenRun() && !m->IsRiding() && !m->IsSledgeMode() && !m->IsLargeMinion() && !level->m_bossGameplay) {
            if (m_sledgeBeginPlane.getDistanceTo(m->GetPosition()) > 0) {
                m->BeginSledge();
                m_sledgeBegun = true;
            }
        }
    }
    if (m_sledgeEnded) return;
    if (m_sledgeEndPlane.getDistanceTo(m_minion->GetPosition()) > 0) {
        m_minion->EndSledge();
        m_sledgeEnded = true;
    }
}

// @0x53bbec: the template's own camera takes over once the scene camera has passed its start (not in the fluffy dimension, the fast
// forward, the cinematic camera nor with the large minion)
bool GameLevel::CheckCameraChange() {
    if (m_cameraChangeDone || m_fluffyModeCamera || m_fastForwardCamera || IsCinematicCameraEnabled()) return false;
    // the large minion mode (GS_Gameplay's current mode, +0x5C in the .so): the camera stays put
    if (GS_Gameplay *gs = GS_Gameplay::GetInstance()) if (gs->GetCurrentMode() == GS_Gameplay::Mode_LargeMinion) return false;
    if (!(m_cameraChangePlane.getDistanceTo(m_camera->GetPosition()) > 0)) return false;
    if (m_cameraChangeInstance) {
        m_cameraChangeInstance->ApplyCameraValue(-1);
        m_cameraChangeInstance = 0;
    }
    m_cameraChangeDone = true;
    return true;
}

// @0x53f490
void GameLevel::OnLevelTemplateInstanceDelete(const GameplayTemplateInstance *g) {
    if (m_cameraChangeDone || !m_cameraChangeInstance) return;
    if (m_cameraChangeInstance == g) {
        m_cameraChangeInstance = 0;
        m_cameraChangeDone = true;
    }
}

// @0x53f470
void GameLevel::OnAdvanceLevelTemplateList() { s_levelSequenceGraphMgr->OnAdvanceLevelTemplateList(); }

// @0x53e89c
void GameLevel::Translate(const math::vec3<float> &d) {
    for (GameEntity *e : m_volatileEntities)
        if (e && !e->GetTemplateName().Equals(PlatformInstance::k_tmplName) && !e->IsLinked()) e->Translate(d);
    m_minion->Translate(d);
    m_pacesetter->Translate(d);
    m_cameraChangePlane.translate(d);
    m_sledgeBeginPlane.translate(d);
    m_sledgeEndPlane.translate(d);
}

// @0x53f500
void GameLevel::SetCameraRelativeToMinionSpeed(CameraController *c, unsigned ms, float referenceSpeed) {
    if (ms && m_minion) {
        const float speed = m_minion->GetPacesetter() ? m_minion->GetPacesetter()->GetSpeed() : 0.0f;
        if (speed > referenceSpeed) ms = (unsigned)(((float)ms * referenceSpeed) / speed);
    }
    SetCamera(c, ms);
}

// @0x53ecf4
void GameLevel::RegisterSafePoint(SafeMinionPoint *p, bool add) {
    if (add) {
        m_safePoints.push_back(p);
        m_safePointIndices[p] = 0;
        return;
    }
    m_safePoints.erase(std::remove(m_safePoints.begin(), m_safePoints.end(), p), m_safePoints.end());
    m_safePointIndices.erase(p);
}

// @0x53f32c: the traveled distance is the minion's base plus its run distance (@0x500, @0x504). The point's distance in run (GameEntity +0x74)
// is taken relative to it, and the point with the least distance in (minDistance, maxDistance) wins. With landableOnly, a point that is not
// landable (SafeMinionPoint +0xAD) is skipped.
SafeMinionPoint *GameLevel::GetSafeMinionPoint(bool landableOnly, float minDistance, float maxDistance) {
    const float traveled = (float)m_minion->GetTraveledDistance();
    SafeMinionPoint *result = 0;
    float best = 3.4028234663852886e+38f;  // @0x53f3d4 (FLT_MAX)
    for (std::vector<SafeMinionPoint*>::iterator it = m_safePoints.begin(); it != m_safePoints.end(); ++it) {
        SafeMinionPoint *p = *it;
        if (landableOnly && !p->GetIsLandable()) continue;
        const float d = p->GetDistanceInRun() - traveled;
        if (d > minDistance && d < maxDistance && d < best) {
            best = d;
            result = p;
        }
    }
    return result;
}

// @0x53f3e0: the level's boss is the boss of its boss gameplay (none while there is no boss gameplay)
Boss *GameLevel::GetBoss() const { return m_bossGameplay ? m_bossGameplay->GetBoss() : 0; }
