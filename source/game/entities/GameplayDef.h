// source/game/entities/GameplayDef.h
#ifndef SOURCE_GAME_ENTITIES_GAMEPLAYDEF_H
#define SOURCE_GAME_ENTITIES_GAMEPLAYDEF_H

#include <vector>
#include "GameEntity.h"

class JumpDefinition;

// The tuning of the run (the "GameplayDef" template): how the minion accelerates, jumps, slides and changes lane, the jump definitions
// it follows at each speed, the camera change offset and the time it stays invulnerable after a shield or the freeze gun. 0xE8 bytes in the
// .so. Not ported (they belong to the systems that use them): the despicable bar's freeze values, the collision effects,
// the fast forward cameras, the prices and the tutorials. The values of the missions are read by MissionMgr::LoadGameplayDef.
class GameplayDef : public GameEntity {
public:
    enum JumpDefinitionType { Jump = 0, Fall = 1, FallFromGround = 2 };
    explicit GameplayDef(const clara::Template*);
    virtual ~GameplayDef();
    virtual void PostInit();
    virtual void NotifyParamChanged(const jet::String&);
    // the jump definition of the type that the configuration for the speed holds (the first whose top speed is above it, else the last)
    JumpDefinition *GetJumpDefinition(JumpDefinitionType, float speed) const;
    // gives the minion the values of the data it has not got yet
    void ApplyToMinion() const;
    static jet::String k_tmplName;
private:
    // one configuration: up to which speed it holds, and its three jump definitions
    struct JumpDefinitionsConfig { float maxSpeed; JumpDefinition *definitions[3]; };
    void LoadJumpDefinitions(const clara::Param*);
    void ApplyPhysicsValue(int which, const jet::String &param) const;
    std::vector<JumpDefinitionsConfig> m_jumpConfig;  // 0xAC (0x38 in the original layout)
    static const jet::String k_cameraOffsetParam, k_accelerationParam, k_JumpDefinitionsParam, k_jumpDefinitionMaxSpeedParam, k_jumpDefinitionParamNames[3],
        k_jumpImpulseParam, k_jumpFrontDistanceParam, k_jumpUpDistanceParam, k_dashDownGravityParam, k_slideDistanceParam, k_changeLaneTimeParam,
        k_changeLaneMaxSpeedParam, k_bulletTimeTrackTransTimePgtParam, k_postShieldInvulnerabilityTimeParam, k_postFreezeGunInvulnerabilityTimeParam,
        k_postSledgeInvulnerabilityTimeParam, k_sledgeGravityMultiplierParam, k_ffwFlyingAltitudeParam, k_maxDespicablePointsArrayParam, k_freezingGunBulletEffectParam, k_DespicablePointsOnFreezeObjectsParam, k_despBarDepletionPerSecParam,
        k_despBarBananaSafeTimeParam, k_despBarDespicableSafeTimeParam, k_itemsTotalProbabilityParam, k_minimumItemSpaceParam, k_maximumItemSpaceParam,
        k_initialSpaceParam, k_deathRevivePricesParam, k_deathReviveFreePriceParam, k_deathReviveIncentiveParams[6];
};

#endif
