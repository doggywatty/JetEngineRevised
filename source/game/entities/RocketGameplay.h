// source/game/entities/RocketGameplay.h
#ifndef SOURCE_GAME_ENTITIES_ROCKETGAMEPLAY_H
#define SOURCE_GAME_ENTITIES_ROCKETGAMEPLAY_H

class RocketGameplay : public GameEntity {  // sizeof 0x118
public:
    RocketGameplay(const clara::Template*);
    virtual ~RocketGameplay();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void NotifyParamChanged(const jet::String&);
    void Start();
    void End();
    void Exit();
    int GetSafeLandingNodeCount();
    void GenerateInitialEntities();
    bool GenerateEntitiesInRow(int, int);
    void NotifyNodesAdvanced(int);
    void ForceDismount(bool);
    int GetTotalPlayTime();
    int GetRemainingTime();
    void StartTutorial();
private:
    float ComputeUniformValue();
    void SetPostFXState(RocketGameplay::RadialBlurPostFXState);
    void UpdatePostFxState(unsigned);
    void InitCameraValues();
    void UpdateGeneration();
    void ClearGeneratedEntities();
    void UpdateEndGame();
    void SetState(RocketGameplay::RocketGameplayStates);
    void InitTilemapPools();
public:
    static jet::String k_tmplName;
private:
    unsigned m_gameplayTime;  // 0x90
    unsigned m_gameplayTimeCount;  // 0x94
    int m_postFxTimeIntro;  // 0x98
    int m_postFxTimeOutro;  // 0x9C
    float m_postFxMinShift;  // 0xA0
    float m_postFxMaxShift;  // 0xA4
    RocketGameplay::RadialBlurPostFXState m_postFxState;  // 0xA8
    RocketGameplay::RocketGameplayStates m_state;  // 0xAC
    bool m_cameraValuesSet;  // 0xB0
    float m_cameraLeft;  // 0xB4
    float m_cameraRight;  // 0xB8
    float m_screenWidth;  // 0xBC
    std::list<GameEntity *,std::allocator<GameEntity *> > m_generatedEntities;  // 0xC0
    GameEntity *m_rocketBananaReference;  // 0xD8
    float m_backupAcceleration;  // 0xDC
    int m_safeLandingNodeCount;  // 0xE0
    bool m_canLand;  // 0xE4
    bool m_forceDismount;  // 0xE5
    bool m_disengageOnSafeMinionPoint;  // 0xE6
    int m_currentRow;  // 0xE8
    int m_nodesAdvancedByRocket;  // 0xEC
    int m_rowsToGenerate;  // 0xF0
    int m_rowsGenerated;  // 0xF4
    int m_tileLength;  // 0xF8
    Tilemap::TilemapPoolType m_currentPool;  // 0xFC
    std::vector<std::vector<Tilemap *,std::allocator<Tilemap *> >,std::allocator<std::vector<Tilemap *,std::allocator<Tilemap *> > > > m_tilemapPools;  // 0x100
    GuiObject *m_tutorialGuiObject;  // 0x114
    static const jet::String k_pathParam;
    static const jet::String k_generationPeriodParam;
    static const jet::String k_rocketSpeedMultiplierParam;
    static const jet::String k_rocketAccelerationMultiplierParam;
    static const jet::String k_rocketDeaccelerationMultiplierParam;
    static const jet::String k_postRocketInvulnerabilityTimeParam;
    static const jet::String k_rocketCameraParam;
    static const jet::String k_rocketCameraTransitionTimeParam;
    static const jet::String k_rocketPostFXParam;
    static const jet::String k_rocketBananaRefParam;
    static const jet::String k_rocketFlyingAltitudeParam;
    static const jet::String k_rocketSafeLandingNodeCountParam;
    static const jet::String k_rocketPostFxMaterialParam;
    static const jet::String k_rocketPostFxTimeIntroParam;
    static const jet::String k_rocketPostFxTimeOutroParam;
    static const jet::String k_rocketPostFxMinShiftParam;
    static const jet::String k_rocketPostFxMaxShiftParam;
    static const jet::String k_rocketCollisionTilemapEasyPoolParam;
    static const jet::String k_rocketCollisionTilemapMediumPoolParam;
    static const jet::String k_rocketCollisionTilemapHardPoolParam;
    static const jet::String k_rocketCollisionTilemapTileLengthParam;
    static const jet::String k_paramTutorialGuiObject;
    static const jet::String k_postFxShiftUniformName;
    static const float k_disengageRocketTime;
};

#endif
