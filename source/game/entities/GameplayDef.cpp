// source/game/entities/GameplayDef.cpp
#include "GameplayDef.h"
#include "game/online/store/Price.h"
#include "JumpDefinition.h"
#include "Minion.h"
#include "gameitem/GameItem.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../actors/statemachine/StateMachine.h"
#include "param.h"
#include "project.h"

jet::String GameplayDef::k_tmplName("GameplayDef");
const jet::String GameplayDef::k_cameraOffsetParam("CameraChangeOffset");
const jet::String GameplayDef::k_accelerationParam("Acceleration");
const jet::String GameplayDef::k_JumpDefinitionsParam("JumpDefinitions");
const jet::String GameplayDef::k_jumpDefinitionMaxSpeedParam("maxSpeed");
const jet::String GameplayDef::k_jumpDefinitionParamNames[3] = {jet::String("JumpDefininition"), jet::String("FallJumpDefininition"),
                                                                jet::String("FallFromGroundJumpDefininition")};
const jet::String GameplayDef::k_jumpImpulseParam("JumpImpulse");
const jet::String GameplayDef::k_jumpFrontDistanceParam("JumpFrontDistance");
const jet::String GameplayDef::k_jumpUpDistanceParam("JumpUpDistance");
const jet::String GameplayDef::k_dashDownGravityParam("DashDownGravity");
const jet::String GameplayDef::k_slideDistanceParam("SlideDistance");
const jet::String GameplayDef::k_changeLaneTimeParam("ChangeLaneTime");
const jet::String GameplayDef::k_changeLaneMaxSpeedParam("ChangeLaneMaxSpeed");
const jet::String GameplayDef::k_bulletTimeTrackTransTimePgtParam("bulletTimeTrackTransTimePgt");
const jet::String GameplayDef::k_postShieldInvulnerabilityTimeParam("PostShieldInvulnerabilityTime");
const jet::String GameplayDef::k_postFreezeGunInvulnerabilityTimeParam("PostFreezeInvulnerabilityTime");
const jet::String GameplayDef::k_postSledgeInvulnerabilityTimeParam("PostSledgeInvulnerabilityTime");
const jet::String GameplayDef::k_sledgeGravityMultiplierParam("SledgeGravityMultiplier");
const jet::String GameplayDef::k_ffwFlyingAltitudeParam("FFWFlyingAltitude");
const jet::String GameplayDef::k_maxDespicablePointsArrayParam("MaxDespicablePointsArray");
const jet::String GameplayDef::k_freezingGunBulletEffectParam("freezingGunBulletEffect");
const jet::String GameplayDef::k_DespicablePointsOnFreezeObjectsParam("DespicablePointsOnFreezeObjects");
const jet::String GameplayDef::k_despBarDepletionPerSecParam("DespBarDepletion_PerSec");
const jet::String GameplayDef::k_despBarBananaSafeTimeParam("DespBarDepletion_BananaSafeTime");
const jet::String GameplayDef::k_despBarDespicableSafeTimeParam("DespBarDepletion_DespicableSafeTime");
const jet::String GameplayDef::k_itemsTotalProbabilityParam("ItemsTotalProbability");
const jet::String GameplayDef::k_minimumItemSpaceParam("MinimumItemSpace");
const jet::String GameplayDef::k_maximumItemSpaceParam("MaximumItemSpace");
const jet::String GameplayDef::k_deathRevivePricesParam("DeathRevivePrices");
const jet::String GameplayDef::k_deathReviveFreePriceParam("DeathReviveFreePrice");
const jet::String GameplayDef::k_deathReviveIncentiveParams[6] = {jet::String("DeathReviveIncentive_ChallengePtg"), jet::String("DeathReviveIncentive_topFriendPtg"),
    jet::String("DeathReviveIncentive_NextFriendPtg"), jet::String("DeathReviveIncentive_MissionPtg"), jet::String("DeathReviveIncentive_ScorePtg"),
    jet::String("DeathReviveIncentive_ScoreMaxDiff")};
const jet::String GameplayDef::k_initialSpaceParam("InitialSpace");

// the points of the run (stored in the Game)
static const char *const k_scoreParams[Game::Score_Count] = {"ScoreMeterMultiplier", "ScoreBananaMultiplier", "ScoreDespicableMultiplier", "ScoreNearmiss",
                                                              "ScoreLocationChange", "ScoreBossBeaten", "ScorePassAFriend", "ScoreBeatYourBestDistance",
                                                              "ScoreBeatYourOwnBestScore", "ScoreFreezeGunDestroyObstacle"};

// @0x5682fc
GameplayDef::GameplayDef(const clara::Template *t) : GameEntity(t) {}

// @0x568360
GameplayDef::~GameplayDef() {}

// @0x5683f8: every value of the data is taken
void GameplayDef::PostInit() {
    GameEntity::PostInit();
    NotifyParamChanged(k_cameraOffsetParam);
    NotifyParamChanged(k_accelerationParam);
    NotifyParamChanged(k_JumpDefinitionsParam);
    NotifyParamChanged(k_jumpImpulseParam);
    NotifyParamChanged(k_jumpFrontDistanceParam);
    NotifyParamChanged(k_jumpUpDistanceParam);
    NotifyParamChanged(k_dashDownGravityParam);
    NotifyParamChanged(k_slideDistanceParam);
    NotifyParamChanged(k_changeLaneTimeParam);
    NotifyParamChanged(k_changeLaneMaxSpeedParam);
    NotifyParamChanged(k_bulletTimeTrackTransTimePgtParam);
    NotifyParamChanged(k_postShieldInvulnerabilityTimeParam);
    NotifyParamChanged(k_postFreezeGunInvulnerabilityTimeParam);
    NotifyParamChanged(k_postSledgeInvulnerabilityTimeParam);
    NotifyParamChanged(k_sledgeGravityMultiplierParam);
    NotifyParamChanged(k_ffwFlyingAltitudeParam);
    NotifyParamChanged(k_maxDespicablePointsArrayParam);
    NotifyParamChanged(k_freezingGunBulletEffectParam);
    NotifyParamChanged(k_DespicablePointsOnFreezeObjectsParam);
    NotifyParamChanged(k_despBarDepletionPerSecParam);
    NotifyParamChanged(k_despBarBananaSafeTimeParam);
    NotifyParamChanged(k_despBarDespicableSafeTimeParam);
    NotifyParamChanged(k_itemsTotalProbabilityParam);
    NotifyParamChanged(k_minimumItemSpaceParam);
    NotifyParamChanged(k_maximumItemSpaceParam);
    NotifyParamChanged(k_initialSpaceParam);
    NotifyParamChanged(k_deathRevivePricesParam);
    NotifyParamChanged(k_deathReviveFreePriceParam);
    for (const jet::String &p : k_deathReviveIncentiveParams) NotifyParamChanged(p);
    for (const char *p : k_scoreParams) NotifyParamChanged(jet::String(p));
}

// the minion's physics value ChangeablePhysicsDef `which` from the float param
void GameplayDef::ApplyPhysicsValue(int which, const jet::String &param) const {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    float v = 0;
    GetParam(param, v);
    if (minion) minion->ChangePhysicsValue(which, v);
}

// @0x568a40: a value of the data changed: the minion, the camera and the state machines take it
void GameplayDef::NotifyParamChanged(const jet::String &name) {
    GameLevel *level = GameLevel::GetInstance();
    Minion *minion = level->GetMinion();
    float f = 0;
    int i = 0;
    if (name.Equals(k_cameraOffsetParam)) {
        GetParam(name, f);
        level->SetCameraChangeOffset(f);
    } else if (name.Equals(k_accelerationParam))
        ApplyPhysicsValue(0, name);
    else if (name.Equals(k_JumpDefinitionsParam))
        LoadJumpDefinitions(FindParamByName(name));
    else if (name.Equals(k_jumpImpulseParam))
        ApplyPhysicsValue(1, name);
    else if (name.Equals(k_jumpFrontDistanceParam))
        ApplyPhysicsValue(2, name);
    else if (name.Equals(k_jumpUpDistanceParam))
        ApplyPhysicsValue(3, name);
    else if (name.Equals(k_dashDownGravityParam))
        ApplyPhysicsValue(4, name);
    else if (name.Equals(k_slideDistanceParam))
        ApplyPhysicsValue(5, name);
    else if (name.Equals(k_changeLaneTimeParam))
        ApplyPhysicsValue(6, name);
    else if (name.Equals(k_changeLaneMaxSpeedParam))
        ApplyPhysicsValue(7, name);
    else if (name.Equals(k_bulletTimeTrackTransTimePgtParam)) {
        GetParam(name, f);
        StateMachine::SetBulletTimeTrackTransitionTimePgt(f * 0.01f);
    } else if (name.Equals(k_postShieldInvulnerabilityTimeParam)) {
        GetParam(name, i);
        if (minion) minion->SetPostShieldInvulnerabilityTime(i);
    } else if (name.Equals(k_postFreezeGunInvulnerabilityTimeParam)) {
        GetParam(name, i);
        if (minion) minion->SetPostFreezeGunInvulnerabilityTime(i);
    } else if (name.Equals(k_postSledgeInvulnerabilityTimeParam)) {
        GetParam(name, f);
        if (minion) minion->SetPostSledgeInvulnerabilityTime((int)f);
    } else if (name.Equals(k_sledgeGravityMultiplierParam))
        ApplyPhysicsValue(8, name);
    else if (name.Equals(k_ffwFlyingAltitudeParam))
        ApplyPhysicsValue(9, name);
    else if (name.Equals(k_maxDespicablePointsArrayParam)) {
        // the points of each level of the despicable multiplier (the array is read again each time it changes)
        Game *game = Game::GetInstance();
        game->MaxDespPointsArray_Reset();
        if (const clara::Param *p = FindParamByName(name))
            for (unsigned k = 0; k < p->GetComponentCount(); k++) game->MaxDespPointsArray_Add(p->GetAsFloat(k));
    } else if (name.Equals(k_DespicablePointsOnFreezeObjectsParam)) {
        // the points a frozen obstacle gives (@0x568a40 stores the float in the minion at 0x2C4)
        GetParam(name, f);
        if (minion) minion->SetFreezeDespicablePoints(f);
    } else if (name.Equals(k_freezingGunBulletEffectParam)) {
        // the freeze ray's bullet effect is the minion's (@0x568a40 stores the entity the path names in the minion at 0x4C8)
        if (minion)
            if (const clara::Param *p = FindParamByName(name))
                minion->SetFreezingBulletEffect(reinterpret_cast<const EffectDef*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0))));
    } else if (name.Equals(k_despBarDepletionPerSecParam)) {
        GetParam(name, f);
        Game::GetInstance()->SetDespBarDepletionPerSec(f);
    } else if (name.Equals(k_despBarBananaSafeTimeParam)) {
        GetParam(name, f);
        Game::GetInstance()->SetDespBananaSafeTime(f * 1000.0f);
    } else if (name.Equals(k_despBarDespicableSafeTimeParam)) {
        GetParam(name, f);
        Game::GetInstance()->SetDespicableSafeTime(f * 1000.0f);
    } else if (name.Equals(k_itemsTotalProbabilityParam)) {
        GetParam(name, f);
        GameItem::SetTotalProbabilityValue(f);
    } else if (name.Equals(k_minimumItemSpaceParam)) {
        GetParam(name, f);
        GameItem::SetMinimumItemSpace(f);
    } else if (name.Equals(k_maximumItemSpaceParam)) {
        GetParam(name, f);
        GameItem::SetMaximumItemSpace(f);
    } else if (name.Equals(k_initialSpaceParam)) {
        GetParam(name, f);
        GameItem::SetInitialSpace(f);
    } else if (name.Equals(k_deathRevivePricesParam)) {
        // @0x56a334: the revive prices are the entities the list names, in order (Game::DeathRevive_AddPrice)
        Game::GetInstance()->DeathRevive_ResetPrices();
        if (const clara::Param *p = FindParamByName(name)) {
            clara::Project *project = clara::Project::GetInstance();
            for (unsigned i = 0; i < p->GetComponentCount(); i++)
                Game::GetInstance()->DeathRevive_AddPrice(static_cast<Price*>(project->FindEntityByPath(p->GetAsPath(i))));
        }
    } else if (name.Equals(k_deathReviveFreePriceParam)) {
        // @0x56a3bc: the free revive's price is the entity the parameter names (Game::DeathRevive_SetFreeRevivePrice)
        if (const clara::Param *p = FindParamByName(name))
            Game::GetInstance()->DeathRevive_SetFreeRevivePrice(static_cast<Price*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0))));
    } else if (name.Equals(k_deathReviveIncentiveParams[5])) {
        // @0x56a574: the score difference that shows the score incentive
        GetParam(name, i);
        Game::GetInstance()->SetDeathReviveScoreMaxDiff(i);
    } else if (name.Equals(k_deathReviveIncentiveParams[0]) || name.Equals(k_deathReviveIncentiveParams[1]) || name.Equals(k_deathReviveIncentiveParams[2]) ||
               name.Equals(k_deathReviveIncentiveParams[3]) || name.Equals(k_deathReviveIncentiveParams[4])) {
        // @0x56a420..0x56a52c: the ratios of the incentives, stored in the Game's block in the order of the parameters
        for (int k = 0; k < 5; k++)
            if (name.Equals(k_deathReviveIncentiveParams[k])) {
                GetParam(name, f);
                Game::GetInstance()->SetDeathReviveIncentivePtg(k, f);
            }
    } else
        for (int k = 0; k < Game::Score_Count; k++)
            if (name.Equals(k_scoreParams[k])) {
                GetParam(name, i);
                Game::GetInstance()->SetScoreValue((Game::ScoreValue)k, i);
            }
}

// the minion made after the data was read has the values too
void GameplayDef::ApplyToMinion() const {
    static const struct { int which; const jet::String *param; } k_values[] = {
        {0, &k_accelerationParam}, {1, &k_jumpImpulseParam}, {2, &k_jumpFrontDistanceParam}, {3, &k_jumpUpDistanceParam}, {4, &k_dashDownGravityParam},
        {5, &k_slideDistanceParam}, {6, &k_changeLaneTimeParam}, {7, &k_changeLaneMaxSpeedParam}, {8, &k_sledgeGravityMultiplierParam},
        {9, &k_ffwFlyingAltitudeParam}};
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion) return;
    for (const auto &v : k_values) ApplyPhysicsValue(v.which, *v.param);
    int i = 0;
    float f = 0;
    GetParam(k_postShieldInvulnerabilityTimeParam, i);
    minion->SetPostShieldInvulnerabilityTime(i);
    i = 0;
    GetParam(k_postFreezeGunInvulnerabilityTimeParam, i);
    minion->SetPostFreezeGunInvulnerabilityTime(i);
    GetParam(k_postSledgeInvulnerabilityTimeParam, f);
    minion->SetPostSledgeInvulnerabilityTime((int)f);
}

// @0x56af94: the configurations of the data: for each the speed it holds up to and its jump, fall and fall from ground definitions
void GameplayDef::LoadJumpDefinitions(const clara::Param *p) {
    if (!p) return;
    clara::Project *project = clara::Project::GetInstance();
    for (unsigned i = 0; i < p->GetComponentCount(); i++) {
        clara::DataEntity *e = p->GetAsEntity(i);
        JumpDefinitionsConfig c = {0, {0, 0, 0}};
        e->GetParam(k_jumpDefinitionMaxSpeedParam, c.maxSpeed);
        for (int k = 0; k < 3; k++) {
            clara::DataEntity *def = project->FindEntityByPath(e->FindParamByName(k_jumpDefinitionParamNames[k])->GetAsPath(0));
            c.definitions[k] = def && def->GetTemplateName().Equals(JumpDefinition::k_tmplName) ? static_cast<JumpDefinition*>(def) : 0;
        }
        m_jumpConfig.push_back(c);
    }
}

// @0x56af28: the configurations are in order of speed: the first one whose speed is above the given one (else the last) gives its definition
JumpDefinition *GameplayDef::GetJumpDefinition(JumpDefinitionType type, float speed) const {
    const JumpDefinitionsConfig *found = 0;
    float previousMax = -1.0f;
    for (const JumpDefinitionsConfig &c : m_jumpConfig) {
        if (previousMax > speed) break;
        found = &c;
        previousMax = c.maxSpeed;
    }
    return found ? found->definitions[type] : 0;
}
