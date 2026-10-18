// source/game/entities/LargeMinionGameplay.cpp
#include "LargeMinionGameplay.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "SafeMinionPoint.h"
#include "Runner.h"
#include "BackgroundData.h"
#include "LocationConfig.h"
#include "camera/CameraEntity.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../gamestates/GS_Gameplay.h"
#include "../gamemanagers/BackgroundMgr.h"
#include "../gamemanagers/CostumeMgr.h"
#include "../gamemanagers/GameSoundMgr.h"
#include "../gamemanagers/LocationsMgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../leveltemplate/LevelTemplateInstance.h"
#include "../leveltemplate/SpeedDef.h"
#include "../utils/ControlledRandom.h"
#include "project.h"
#include "MaterialDef.h"
#include "Bonus.h"
#include "BonusSet.h"
#include "MinionCostume.h"
#include "../gamemanagers/CostumeMgr.h"
#include "camera/cameramgr.h"
#include "core/util.h"

const jet::String LargeMinionGameplay::k_tmplName("LargeMinionGameplay");
const jet::String LargeMinionGameplay::k_largeMinionGameplayWave_tmplName("LargeMinionGameplayWave");
const jet::String LargeMinionGameplay::k_largeMinionGameplaySequence_tmplName("LargeMinionGameplaySequence");
const jet::String LargeMinionGameplay::k_largeMinionGameplayConfig_tmplName("LargeMinionGameplayConfig");
const jet::String LargeMinionGameplay::k_paramSpecificCamera("SpecificCamera");
const jet::String LargeMinionGameplay::k_paramCameraTransitionTime("CameraTransitionTime");
const jet::String LargeMinionGameplay::k_paramPostGamplaylnerabilityTime("PostGamplaylnerabilityTime");
const jet::String LargeMinionGameplay::k_paramPostFx("PostFX");
const jet::String LargeMinionGameplay::k_paramUnbalanceTime("UnbalanceTime");
const jet::String LargeMinionGameplay::k_paramBackgroundSound("BackgroundSound");
const jet::String LargeMinionGameplay::k_paramAttackLaneRange("AttackLaneRange");
const jet::String LargeMinionGameplay::k_paramRunnerDistanceToHit("RunnerDistanceToHit");
const jet::String LargeMinionGameplay::k_paramRunnerAccelerationDistance("RunnerAccelerationDistance");
const jet::String LargeMinionGameplay::k_paramEnableDebug("LargeMinionEnableDebug");
const jet::String LargeMinionGameplay::k_paramEnableSliders("LargeMinionEnableSliders");
const jet::String LargeMinionGameplay::k_paramTutorialGuiObject("tutorialGuiObject");
const jet::String LargeMinionGameplay::k_paramMinionSpeed("MinionSpeed");
const jet::String LargeMinionGameplay::k_emptyStr("");

// the .so's literals and the constants of the mode
static const float k_speedTimeScale = 0.001f;           // @0x56fbe8 (UpdateRunnerSpawn: the acceleration distance over the speed, to ms)
static const int k_modeLane = 1;                        // @0x570108 (Start: the lane the pacesetter takes, the middle one)
static const int k_modeItemType = 4;                    // @0x5701e8 (Start: the GameItemType the mode's time comes from)

// @0x56f528
LargeMinionGameplay::LargeMinionGameplay(const clara::Template *t)
    : GameEntity(t), m_gameplayTime(0), m_cameraEntity(0), m_cameraTransitionTime(0), m_postGameplayInvulnerabilityTime(0), m_config(0),
      m_postFx(0), m_minionSpeed(0), m_gameplayTimeCount(0), m_tiltTarget(0), m_currentTilt(0), m_currentSequence(0), m_runnerSpawnTimer(0),
      m_runnerWaveIdx(0), m_balanceState(0), m_pacesetter(0), m_oldPacesetter(0), m_levelTemplateMgr(0), m_savedDistanceBase(0),
      m_savedDistanceRun(0), m_tutorialGuiObject(0), m_unbalanceTime(0), m_attackLaneRange(0), m_runnerDistanceToHitSq(0),
      m_runnerAccelerationDistance(0), m_enableDebug(false), m_enableSliders(false), m_backgroundSoundId(-1)
      {
    m_levelTemplateMgr = new LevelTemplateMgr(this);
}

// @0x56f5dc
LargeMinionGameplay::~LargeMinionGameplay() {
    delete m_levelTemplateMgr;
    if (m_pacesetter) delete m_pacesetter;
}

// @0x56f6ac: the two debug flags are read once when the entity is set up
void LargeMinionGameplay::Init() {
    NotifyParamChanged(k_paramEnableDebug);
    NotifyParamChanged(k_paramEnableSliders);
}

// @0x56f6f8: the mode's params, its post effect and tutorial, and its pacesetter. It updates and renders, but it is invisible and disabled
// until the mode starts.
void LargeMinionGameplay::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::PostInit();
    clara::Project *project = clara::Project::GetInstance();
    if (const clara::Param *p = FindParamByName(k_paramSpecificCamera)) m_cameraEntity = static_cast<CameraEntity*>(project->FindEntityByPath(p->GetAsPath(0)));
    GetParam(k_paramCameraTransitionTime, m_cameraTransitionTime);
    GetParam(k_paramPostGamplaylnerabilityTime, m_postGameplayInvulnerabilityTime);
    GetParam(k_paramBackgroundSound, m_backgroundSound);
    GetParam(k_paramUnbalanceTime, m_unbalanceTime);
    GetParam(k_paramAttackLaneRange, m_attackLaneRange);
    GetParam(k_paramRunnerDistanceToHit, m_runnerDistanceToHitSq);
    m_runnerDistanceToHitSq *= m_runnerDistanceToHitSq;
    GetParam(k_paramRunnerAccelerationDistance, m_runnerAccelerationDistance);
    GetParam(k_paramMinionSpeed, m_minionSpeed);
    if (const clara::Param *p = FindParamByName(k_paramPostFx)) m_postFx = static_cast<MaterialDef*>(project->FindEntityByPath(p->GetAsPath(0)));
    if (const clara::Param *p = FindParamByName(k_paramTutorialGuiObject)) m_tutorialGuiObject = project->FindEntityByPath(p->GetAsPath(0));
    RegisterForUpdate(true);
    RegisterForRender(true);
    SetVisible(false);
    SetEnabled(false);
    m_pacesetter = new Pacesetter(project->FindTemplateByName(Pacesetter::k_tmplName));
    m_pacesetter->SetName(jet::String("LargeMinionPacestter"));
    m_pacesetter->Init();
}

// @0x56fa8c: the runner waves of the mode, then the time of the mode. (The balance of the minion's tilt and the bapple spawn of the .so are not
// ported yet; see re/TODO.md.)
void LargeMinionGameplay::Update() {
    GameEntity::Update();
    UpdateRunnerSpawn(Game::GetInstance()->GetUpdateStep());
    m_pacesetter->Update();
    UpdateTimeEnd();
}

// @0x570090: the render is the entity's own
void LargeMinionGameplay::Render() { GameEntity::Render(); }

// @0x56fb04: the wave of runners comes when the time to the next wave is out. Each runner of the wave is put on the minion's pacesetter
// at the wave's first distance plus the distance between the runners, in the wave's lane (1: a side lane at random), and it runs at the
// wave's speed over the acceleration distance at the minion's speed. The time to the next wave is taken from the wave that spawned.
void LargeMinionGameplay::UpdateRunnerSpawn(int ms) {
    m_runnerSpawnTimer -= ms;
    if (m_runnerSpawnTimer > 0) return;
    const LargeMinionGameplayWave &wave = m_currentSequence->waves[m_runnerWaveIdx];
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    const float firstDistance = jet::core::Randf(wave.firstRunnerDistance.x, wave.firstRunnerDistance.y);
    const float distanceBetween = jet::core::Randf(wave.distanceBetweenRunner.x, wave.distanceBetweenRunner.y);
    const int count = jet::core::Rand((int)wave.numMinions.x, (int)wave.numMinions.y);
    const float speedPgt = jet::core::Randf(wave.speedPgt.x, wave.speedPgt.y);
    int lane = wave.lane;
    if (lane == 1) lane = (jet::core::Rand() & 1) ? 2 : 0;
    const int speedTime = (int)(m_runnerAccelerationDistance / (minion->GetPacesetter()->GetTargetSpeed() * k_speedTimeScale));
    const std::vector<Runner*> &runners = m_config->runners;
    if (getenv("JET_DBGMINION")) fprintf(stderr, "large minion wave %u: %d runner(s) in lane %d\n", (unsigned)m_runnerWaveIdx, count, lane);
    for (int i = 1; i <= count; i++) {
        const float distance = firstDistance + (float)i * distanceBetween;
        Runner *source = runners[jet::core::Rand() % runners.size()];
        math::vec3<float> pos;
        math::quat<float> rot;
        minion->GetPacesetter()->GetAdvancedTransformInPath(distance, pos, rot, (LaneIndex)lane);
        Runner *runner = new Runner(source->GetTemplate());
        runner->SetSpawnLane(lane);
        runner->ConstructFrom(source);
        runner->SetSpeedPgt(speedPgt, speedTime);
        runner->SetLargeMinionRunner(true);
        runner->SetTransform(pos, rot);
        AddCreatedEntity(runner);
    }
    m_runnerWaveIdx = (m_runnerWaveIdx + 1) % m_currentSequence->waves.size();
    m_runnerSpawnTimer = jet::core::Rand((int)wave.nextWaveTime.x, (int)wave.nextWaveTime.y);
}

// @0x56fef4: the mode ends when its time is out: the game ends the mode, and the mode's sound moves to its stop state
void LargeMinionGameplay::UpdateTimeEnd() {
    m_gameplayTimeCount += Game::GetInstance()->GetUpdateStep();
    if (m_gameplayTimeCount < (int)m_gameplayTime) return;
    GS_Gameplay::GetInstance()->EndLargeMinionGameplay();
}

// @0x570094
const LargeMinionGameplay::LargeMinionGameplayConfig &LargeMinionGameplay::FindConfigToUse(const ELocation &location) const {
    return LocationsMgr::GetInstance()->GetLocationConfig(location.underlying())->GetLargeMinionConfig();
}

// @0x5700c0: the mode starts. The minion takes its large state under the mode's god mode, its distances are cleared and its pacesetter goes
// to the mode's pacesetter; a random sequence of the location is played and its first wave is timed.
void LargeMinionGameplay::Start(const ELocation &location) {
    SetVisible(true);
    PowerUpMgr::GetInstance()->DisableAll(true);
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    minion->SM_SetStateLabel(20);
    PowerUpMgr::GetInstance()->EnablePowerUp(EPowerUp(EPowerUpDef::GodMode), -1);
    m_savedDistanceBase = minion->GetDistanceBase();
    m_savedDistanceRun = minion->GetDistanceRun();
    minion->SetBlendDirections(false);
    m_currentTilt = 0;
    m_config = &FindConfigToUse(location);
    if (getenv("JET_DBGMINION")) fprintf(stderr, "large minion begins at location %d (%u sequences)\n", location.underlying(), (unsigned)m_config->sequences.size());
    m_currentSequence = &m_config->sequences[jet::core::Rand() % m_config->sequences.size()];
    m_currentLocation = k_emptyStr;
    BackgroundMgr::GetInstance()->EnableLargeMinionBackground(true);
    m_levelTemplateMgr->CacheInitialLevelSequence();
    m_levelTemplateMgr->InitLevelParts();
    m_gameplayTime = 1000 * BonusSet::GetInstance()->GetBonus((GameItemType)k_modeItemType, false)->GetAmount(-1) +
                     CostumeMgr::GetInstance()->GetEquippedCostumeRef()->GetExtraTimeForPowerUp((GameItemType)k_modeItemType);
    if (getenv("JET_DBGMINION")) fprintf(stderr, "large minion time %u ms\n", (unsigned)m_gameplayTime);
    m_gameplayTimeCount = 0;
    m_runnerSpawnTimer = jet::core::Rand((int)m_currentSequence->waves[0].nextWaveTime.x, (int)m_currentSequence->waves[0].nextWaveTime.y);
    m_runnerWaveIdx = 0;
    CameraController *cc = m_cameraEntity->GetCameraController();
    CameraMgr::GetInstance()->SetCamera(cc, m_cameraTransitionTime, 0);
    m_oldPacesetter = minion->GetPacesetter();
    m_oldPacesetter->UnregisterFollower(minion);
    m_oldPacesetter->SetPaused(true);
    m_pacesetter->RegisterFollower(minion);
    GameplayTemplateInstance *gti = m_levelTemplateMgr->GetCurrentLevelTemplateInstance();
    m_pacesetter->SetCurrentGameplayTemplateInstance(gti);
    const math::vec3<float> lanePos = gti->GetLanePath((LaneIndex)k_modeLane)->GetLastInterpolatedPosition();
    m_pacesetter->SetLanePosition(lanePos, 0, (LaneIndex)k_modeLane, 0);
    m_pacesetter->ApplyCurrentLaneTransform();
    m_pacesetter->SetPaused(false);
    m_pacesetter->SetSpeed(m_minionSpeed, 0);
    minion->SetCurrentJumpDefinition(0);
    m_backgroundSoundId = GameSoundMgr::GetInstance()->Play(m_backgroundSound, 0.0f, minion->GetPosition(), 0);
}

// @0x57045c: the mode ends: the god mode and the minion's distances and pacesetter go back to the level, and the minion is invulnerable for
// the post mode time
void LargeMinionGameplay::End() {
    if (getenv("JET_DBGMINION")) fprintf(stderr, "large minion ends after %d ms\n", m_gameplayTimeCount);
    SetVisible(false);
    PowerUpMgr::GetInstance()->DisablePowerUp(EPowerUp(EPowerUpDef::GodMode), true);
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    minion->SetInvulnerable(m_postGameplayInvulnerabilityTime);
    minion->SetBlendDirections(true);
    minion->SetDistanceBase(m_savedDistanceBase);
    minion->SetDistanceRun(m_savedDistanceRun);
    BackgroundMgr::GetInstance()->EnableLargeMinionBackground(false);
    m_currentSequence = 0;
    m_currentLocation = k_emptyStr;
    GameLevel::GetInstance()->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance()->ApplyCameraValue(0);
    m_levelTemplateMgr->CleanLevelParts();
    CleanCreatedEntities();
    minion->SetCurrentJumpDefinition(0);
    m_pacesetter->UnregisterFollower(minion);
    m_oldPacesetter->RegisterFollower(minion);
    m_oldPacesetter->SetPaused(false);
    minion->SetCurrentJumpDefinition(0);
    // @0x570568..0x570608: the minion goes back to the nearest safe point 0 to 180 m ahead of it (any lane), if there is one: the old pacesetter
    // takes the point's position and lane, and the minion takes the point's position with the old pacesetter's rotation.
    if (SafeMinionPoint *p = GameLevel::GetInstance()->GetSafeMinionPoint(false, 0.0f, 180.0f)) {
        m_oldPacesetter->SetLanePosition(p->GetPosition(), 0, (LaneIndex)p->GetSpawnLane(), 0);
        minion->SetTransform(p->GetPosition(), m_oldPacesetter->GetRotation(), minion->GetScale());
    }
    m_oldPacesetter->Update();
    minion->Update();
}

// @0x570900: the entities the mode created are removed and deleted
void LargeMinionGameplay::CleanCreatedEntities() {
    while (!m_createdEntities.empty()) {
        GameEntity *e = m_createdEntities.front();
        m_createdEntities.erase(m_createdEntities.begin());
        delete e;
    }
}

// @0x570830: the entity is taken out of the list of the entities the mode created, once
void LargeMinionGameplay::RemoveCreatedEntity(GameEntity *e) {
    for (std::vector<GameEntity*>::iterator it = m_createdEntities.begin(); it != m_createdEntities.end(); ++it) {
        if (*it == e) {
            m_createdEntities.erase(it);
            return;
        }
    }
}

// @0x5707e4: an entity the mode creates is listed once, at the end of the list
void LargeMinionGameplay::AddCreatedEntity(GameEntity *e) {
    RemoveCreatedEntity(e);
    m_createdEntities.push_back(e);
}

// @0x57094c: the two debug flags follow their params when they change (the .so also logs the change, which the port does not)
void LargeMinionGameplay::NotifyParamChanged(const jet::String &name) {
    if (name.Equals(k_paramEnableDebug)) {
        GetParam(name, m_enableDebug);
    } else if (name.Equals(k_paramEnableSliders)) {
        GetParam(name, m_enableSliders);
    }
}

// @0x570734: the location of the mode is the name of its background, read on the first use of the sequence
const jet::String &LargeMinionGameplay::GetNextLocation() {
    if (m_currentLocation.size() == 0) {
        if (clara::DataEntity *bg = clara::Project::GetInstance()->FindEntityByPath(m_currentSequence->background))
            m_currentLocation = static_cast<BackgroundData*>(bg)->GetName();
    }
    return m_currentLocation;
}

// @0x570650: the sequence number is not advanced by the mode
void LargeMinionGameplay::IncSequenceNumber() {}

// @0x57065c
int LargeMinionGameplay::GetNextSequenceNumber() { return m_currentSequence->gameplayTemplateIndex; }

// @0x570670
GameEntity *LargeMinionGameplay::GetNextCamera() { return m_cameraEntity; }

// @0x570680
int LargeMinionGameplay::GetNextCameraTransitionTime() { return m_cameraTransitionTime; }

// @0x570690: the speed of the next template is the minion's pacesetter's target speed, not a speed set by the data
const SpeedDef *LargeMinionGameplay::GetNextSpeed() {
    static SpeedDef speed;
    if (Pacesetter *p = GameLevel::GetInstance()->GetMinion()->GetPacesetter()) {
        speed.speed = p->GetTargetSpeed();
        speed.isSet = false;
    }
    return &speed;
}

// @0x57078c: the mode has no node label
const jet::String &LargeMinionGameplay::GetNextNodeLabel() { return k_emptyStr; }

// @0x5707b0: the mode has no node id
const jet::String &LargeMinionGameplay::GetNextNodeId() { return k_emptyStr; }

// @0x5707d4: the mode has no node graph
int LargeMinionGameplay::GetNextNodeGraphId() { return -1; }

// @0x5617c8: a node is not completed by the mode
void LargeMinionGameplay::OnNodeCompleted(const int &, const jet::String &) {}
