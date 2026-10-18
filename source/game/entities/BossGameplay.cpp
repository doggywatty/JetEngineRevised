// source/game/entities/BossGameplay.cpp
#include "BossGameplay.h"
#include "Boss.h"
#include "BossMeena.h"
#include "BossMacho.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "Puppet.h"
#include "BackgroundData.h"
#include "Collision3d.h"
#include "camera/CameraEntity.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../gamestates/GS_Gameplay.h"
#include "../gamemanagers/BackgroundMgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "../input/InputMgr.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../Player.h"
#include "camera/cameramgr.h"
#include "project.h"
#include "core/util.h"
#include "scene/scenemgr.h"

jet::String BossGameplay::k_tmplName("BossGameplay");
const jet::String BossGameplay::k_bossRefParam("BossRef");
const jet::String BossGameplay::k_levelTemplateListParam("LevelTemplateList");
const jet::String BossGameplay::k_bossCameraParam("BossCamera");
const jet::String BossGameplay::k_cameraTransitionTimeParam("CameraTransitionTime");
const jet::String BossGameplay::k_environmentPoolParam("EnvironmentPool");
const jet::String BossGameplay::k_postGameplaylnvulnerabilityTimeParam("PostGameplaylnvulnerabilityTime");
const jet::String BossGameplay::k_minionSpeedParam("MinionSpeed");
const jet::String BossGameplay::k_returnLaneOffsetParam("ReturnLaneOffset");
const jet::String BossGameplay::k_hasTutorialParam("HasTutorial");
const jet::String BossGameplay::k_tutorialLevelTemplateParam("TutorialLevelTemplate");
const jet::String BossGameplay::k_emptyStr;

// the squared distance (of 25) between the boss and the minion under which the boss gameplay's plane is checked (@0x5a58c4)
static const float k_closeDistanceSq = 25.0f;

// @0x5a5174
BossGameplay::BossGameplay(const clara::Template *t)
    : GameEntity(t), m_state(BossGameplay::BossGameplayState_Waiting), m_bossRef(0), m_boss(0), m_bossPuppet(0), m_sequenceIdx(0),
      m_cameraEntity(0), m_cameraTransitionTime(0), m_minionSpeed(-1.0f), m_invulnerabilityTime(0.0f), m_returnLaneOffset(0.0f),
      m_winReturnOffsetInLane(0.0f), m_pacesetter(0), m_oldPacesetter(0), m_levelTemplateMgr(0), m_minionSavedA(0), m_minionSavedB(0),
      m_tutorialStarted(false), m_hasTutorial(false), m_tutorialTemplateId(0), m_tutorialBegun(false) {}

// @0x5a5218: the level template manager, the pacesetter, the boss and its puppet go; the level forgets the gameplay
BossGameplay::~BossGameplay() {
    delete m_levelTemplateMgr;
    delete m_pacesetter;
    delete m_boss;
    delete m_bossPuppet;
    GameLevel *level = GameLevel::GetInstance();
    if (level && level->GetBossGameplay() == this) level->SetBossGameplay(0);
}

// @0x5a5360
void BossGameplay::Init() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::Init();
}

// @0x5a5370: the references and the parameters of the gameplay; it is updated and rendered as an entity of the level
void BossGameplay::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::PostInit();
    clara::Project *project = clara::Project::GetInstance();
    if (const clara::Param *p = FindParamByName(k_bossRefParam)) m_bossRef = project->FindEntityByPath(p->GetAsPath(0));
    m_sequences.clear();
    const clara::Param *levelTemplates = FindParamByName(k_levelTemplateListParam);
    for (unsigned i = 0; i < levelTemplates->GetComponentCount(); ++i) m_sequences.push_back(levelTemplates->GetAsU32(i));
    if (const clara::Param *p = FindParamByName(k_bossCameraParam)) m_cameraEntity = static_cast<CameraEntity*>(project->FindEntityByPath(p->GetAsPath(0)));
    GetParam(k_cameraTransitionTimeParam, m_cameraTransitionTime);
    GetParam(k_postGameplaylnvulnerabilityTimeParam, m_invulnerabilityTime);
    const clara::Param *pool = FindParamByName(k_environmentPoolParam);
    for (unsigned i = 0; i < pool->GetComponentCount(); ++i)
        m_backgroundArray.push_back(static_cast<BackgroundData*>(project->FindEntityByPath(pool->GetAsPath(i))));
    GetParam(k_minionSpeedParam, m_minionSpeed);
    GetParam(k_returnLaneOffsetParam, m_returnLaneOffset);
    GetParam(k_hasTutorialParam, m_hasTutorial);
    // the tutorial's template is read as the 16-bit value the data gives
    if (m_hasTutorial) {
        if (const clara::Param *p = FindParamByName(k_tutorialLevelTemplateParam)) m_tutorialTemplateId = (unsigned short)p->GetAsU32(0);
    }
    RegisterForUpdate(true);
    RegisterForRender(true);
}

// @0x5a58c4: the boss gameplay's update. While it waits, it starts when the level has no other boss gameplay, and the minion's pacesetter
// reaches the boss's plane (the plane through the boss, facing the minion's pacesetter's direction) in front of the minion's place; the
// final flash is made when the gameplay is final. The minion's pacesetter is updated at the end.
void BossGameplay::Update() {
    GameEntity::Update();
    if (m_state == BossGameplayState_Waiting) {
        UpdateWaiting();
    } else if (m_state == BossGameplayState_Final) {
        GS_Gameplay::GetInstance()->MakeBossFinalFlash();
        m_state = BossGameplayState_Done;
    }
    if (m_pacesetter) m_pacesetter->Update();
}

// the waiting part of Update (@0x5a58c4): the boss gameplay is running when it is the level's one; before that it starts once the boss
// plane is passed by the pacesetter of the minion. The mount dismount of the .so (GameLevel::ForceDismountActiveMount @0x53fc10, for a minion
// near the boss while a mount is active) is not ported: the port keeps no active mount.
void BossGameplay::UpdateWaiting() {
    GameLevel *level = GameLevel::GetInstance();
    if (level->GetBossGameplay()) {
        if (level->GetBossGameplay() == this) m_state = BossGameplay::BossGameplayState_Running;
        return;
    }
    Minion *minion = level->GetMinion();
    Pacesetter *ps = minion->GetPacesetter();
    if (!ps || minion->IsFrozenRun() || minion->IsCannon()) return;
    const math::vec3<float> offset = GetPosition() - minion->GetPosition();
    if (offset.getLengthSq() >= k_closeDistanceSq) return;
    // the plane of the boss, facing the direction of the minion's pacesetter
    const math::vec3<float> normal = ps->GetRotation() * jet::scene::SceneMgr::GetFrontVector();
    math::plane<float> plane(GetPosition(), normal);
    if (plane.getDistanceTo(ps->GetPosition()) <= 0.0f) return;
    m_state = BossGameplay::BossGameplayState_Running;
    level->SetBossGameplay(this);
    GS_Gameplay::GetInstance()->MakeBossInitialFlash();
}

// @0x5a5814: the pacesetter of the gameplay, from the Pacesetter template, named "BossPacesetter"
void BossGameplay::CreatePacesetter() {
    m_pacesetter = new Pacesetter(clara::Project::GetInstance()->FindTemplateByName(Pacesetter::k_tmplName));
    m_pacesetter->SetName(jet::String("BossPacesetter"));
    m_pacesetter->Init();
}

// @0x5a5698: the boss of the data's reference is made and follows the reference's data; the first linked puppet of the reference is made
// with it and linked to the boss. The kind is the template name of the reference: BossMeena, BossMacho, or a plain Boss otherwise.
void BossGameplay::CreateBoss() {
    const clara::Template *t = m_bossRef->GetTemplate();
    if (m_bossRef->GetTemplateName().Equals(BossMeena::k_tmplName)) m_boss = new BossMeena(t);
    else if (m_bossRef->GetTemplateName().Equals(BossMacho::k_tmplName)) m_boss = new BossMacho(t);
    else m_boss = new Boss(t);
    m_boss->ConstructFrom(dynamic_cast<GameEntity*>(m_bossRef));
    clara::Entity *ref = static_cast<clara::Entity*>(m_bossRef);
    for (unsigned i = 0; i < ref->GetLinkedEntitiesCount(); ++i) {
        clara::Entity *e = ref->GetLinkedEntity(i);
        if (e && e->GetTemplateName().Equals(Puppet::k_tmplName)) {
            m_bossPuppet = new Puppet(e->GetTemplate());
            m_bossPuppet->ConstructFrom(dynamic_cast<GameEntity*>(e));
            m_bossPuppet->LinkToBoss(m_boss);
            return;
        }
    }
}

// @0x5a5ad8: the boss gameplay starts. The level's template manager is the gameplay's own, the boss and its pacesetter are made, the minion
// keeps its distances aside, the tutorial starts when it is due, the sequence is the last one, the camera of the gameplay is set, and the
// minion moves from its pacesetter to the gameplay's pacesetter, which keeps its speed. The boss behaviour is then on.
void BossGameplay::Start() {
    m_levelTemplateMgr = new LevelTemplateMgr(this);
    GameLevel *level = GameLevel::GetInstance();
    level->SetCurrentLevelTemplateMgr(m_levelTemplateMgr);
    CreatePacesetter();
    CreateBoss();
    Minion *minion = level->GetMinion();
    m_minionSavedA = minion->GetDistanceBase();
    m_minionSavedB = minion->GetDistanceRun();
    minion->SetDistanceBase(0);
    minion->SetDistanceRun(0);
    m_tutorialStarted = false;
    if (m_hasTutorial) m_tutorialStarted = !IsTutorialCompleted();
    if (m_tutorialStarted) BeginTutorial();
    m_sequenceIdx = (int)(m_sequences.size()) - 1;
    BackgroundMgr::GetInstance()->EnableLargeMinionBackground(true);
    m_levelTemplateMgr->CacheInitialLevelSequence();
    m_levelTemplateMgr->InitLevelParts();
    CameraController *cc = m_cameraEntity->GetCameraController();
    CameraMgr::GetInstance()->SetCamera(cc, m_cameraTransitionTime, 0);
    minion->SetCurrentJumpDefinition(0);
    m_oldPacesetter = minion->GetPacesetter();
    m_oldPacesetter->UnregisterFollower(minion);
    m_oldPacesetter->SetPaused(true);
    m_pacesetter->RegisterFollower(minion);
    m_pacesetter->SetCurrentGameplayTemplateInstance(m_levelTemplateMgr->GetCurrentLevelTemplateInstance());
    boost::shared_ptr<PathCommon> path = m_levelTemplateMgr->GetCurrentLevelTemplateInstance()->GetLanePath(Lane_Center);
    const math::vec3<float> lastPos = path->GetLastInterpolatedPosition();
    m_pacesetter->SetLanePosition(lastPos, 0.0f, Lane_Center, 0);
    m_pacesetter->ApplyCurrentLaneTransform();
    m_pacesetter->SetPaused(false);
    m_pacesetter->SetSpeed(m_oldPacesetter->GetSpeed(), 0);
    m_boss->EnableBossBehavior(true);
    m_winReturnOffsetInLane = 0.0f;
    PowerUpMgr::GetInstance()->DisableAll(true);
}

// @0x5a5d34: the boss gameplay ends. The minion gets its distances back and its invulnerability time, the templates' camera is applied and
// the parts of the level are cleaned, the minion is back on the old pacesetter (with the return offset as its forced step), the boss, its
// puppet, the template manager and the pacesetter go, and the level is given back (its tutorial is completed, its input is reset).
void BossGameplay::End() {
    GameLevel *level = GameLevel::GetInstance();
    Minion *minion = level->GetMinion();
    minion->SetInvulnerable((int)m_invulnerabilityTime);
    minion->SetDistanceBase(m_minionSavedA);
    minion->SetDistanceRun(m_minionSavedB);
    BackgroundMgr::GetInstance()->EnableLargeMinionBackground(false);
    level->GetLevelTemplateMgr()->GetCurrentLevelTemplateInstance()->ApplyCameraValue(0);
    m_levelTemplateMgr->CleanLevelParts();
    minion->SetCurrentJumpDefinition(0);
    m_pacesetter->UnregisterFollower(minion);
    m_oldPacesetter->RegisterFollower(minion);
    m_oldPacesetter->SetPaused(false);
    m_oldPacesetter->ForceStep(m_winReturnOffsetInLane);
    m_oldPacesetter->Update();
    minion->Update();
    minion->SetCurrentJumpDefinition(0);
    delete m_boss;
    delete m_bossPuppet;
    delete m_levelTemplateMgr;
    delete m_pacesetter;
    m_boss = 0;
    m_bossPuppet = 0;
    m_levelTemplateMgr = 0;
    m_pacesetter = 0;
    if (level->GetBossGameplay() == this) level->SetBossGameplay(0);
    level->SetTutorialCompleted(true);
    InputMgr::GetInstance()->Reset();
}

// @0x5a5ecc
void BossGameplay::Reset() { m_state = BossGameplay::BossGameplayState_Waiting; }

// @0x5a6014: the boss's state machine goes to the state the killer names: the first name when the boss is defeated, the second one otherwise
// (the .so picks them this way); nothing is named when the first name is empty. The boss's return offset is the gameplay's, the boss
// behaviour turns off (the boss is dying), the level's tutorial flag goes off and the input is reset.
void BossGameplay::KillBoss(const jet::String &exitState, const jet::String &defeatedExitState) {
    const jet::String &state = m_boss->GetBehaviorState() == Boss::BossState_Defeated ? exitState : defeatedExitState;
    if (!exitState.empty()) m_boss->SM_SetState(state);
    m_winReturnOffsetInLane = m_returnLaneOffset;
    m_boss->DisableBossBehavior(true);
    GameLevel::GetInstance()->SetTutorialCompleted(false);
    InputMgr::GetInstance()->Reset();
}

// @0x5a6094: the tutorial of the boss is completed when its bit is set in the player's boss tutorials
bool BossGameplay::IsTutorialCompleted() const {
    return ((int)Player::GetInstance()->GetBossTutorials() >> m_boss->GetTutorialBit()) & 1;
}

// @0x5a60d0: the boss's bit is set in the player's boss tutorials
void BossGameplay::CompleteTutorial() {
    Player *p = Player::GetInstance();
    p->SetBossTutorials((short)(p->GetBossTutorials() | (1 << m_boss->GetTutorialBit())));
}

// @0x5a6234: the tutorial begins: the level's tutorial flag is cleared, the input is reset, and the gameplay knows it
void BossGameplay::BeginTutorial() {
    GameLevel::GetInstance()->SetTutorialCompleted(false);
    InputMgr::GetInstance()->Reset();
    m_tutorialBegun = true;
}

// @0x5a6274: the tutorial ends: the level's tutorial flag is set, the input is reset, the boss's tutorial is completed
void BossGameplay::EndTutorial() {
    GameLevel::GetInstance()->SetTutorialCompleted(true);
    InputMgr::GetInstance()->Reset();
    CompleteTutorial();
    m_tutorialBegun = false;
}

// @0x5ed8: the next sequence; the tutorial's sequence (the first one) ends the tutorial when it is left
void BossGameplay::IncSequenceNumber() {
    if (m_tutorialStarted && m_sequenceIdx == 0) {
        m_tutorialStarted = false;
        return;
    }
    m_sequenceIdx = (m_sequenceIdx + 1) % (int)(m_sequences.size());
}

// @0x5a5f1c
int BossGameplay::GetNextSequenceNumber() {
    if (m_tutorialStarted && m_sequenceIdx == 0) return m_tutorialTemplateId;
    return (int)m_sequences[m_sequenceIdx];
}

// @0x5a5f50
GameEntity *BossGameplay::GetNextCamera() { return m_cameraEntity; }

// @0x5a5f60
int BossGameplay::GetNextCameraTransitionTime() { return m_cameraTransitionTime; }

// @0x5a5f70: the speed is the minion's speed of the gameplay (not set by the data)
const SpeedDef *BossGameplay::GetNextSpeed() {
    m_speedDef.speed = m_minionSpeed;
    m_speedDef.isSet = false;
    return &m_speedDef;
}

// @0x5a5f8c: the location is the name of a background of the environment pool, picked at random
const jet::String &BossGameplay::GetNextLocation() {
    const int i = jet::core::Rand(0, (int)m_backgroundArray.size());
    return m_backgroundArray[i]->GetName();
}

// @0x5a5fbc
const jet::String &BossGameplay::GetNextNodeLabel() { return k_emptyStr; }

// @0x5a5fe0
const jet::String &BossGameplay::GetNextNodeId() { return k_emptyStr; }

// @0x5a6004
int BossGameplay::GetNextNodeGraphId() { return -1; }

// @0x5a5048: nothing is done when a node is completed
void BossGameplay::OnNodeCompleted(const int &, const jet::String &) {}

// @0x5a5ad4
void BossGameplay::Render() {}
