// source/game/entities/FluffyGameplay.h
#ifndef SOURCE_GAME_ENTITIES_FLUFFYGAMEPLAY_H
#define SOURCE_GAME_ENTITIES_FLUFFYGAMEPLAY_H

class FluffyGameplay : public GameEntity {  // sizeof 0x1F4
public:
    FluffyGameplay(const clara::Template*);
    virtual ~FluffyGameplay();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void NotifyParamChanged(const jet::String&);
    virtual void Render();
    void StopFluffy();
    void Start();
    void End();
    bool IsOnFallOut();
    bool HasReachedTop();
    void SetLocation(const Location&);
    jet::String GetNextBackgroundName();
private:
    void InitTilemapPools();
    void InitCameraValues();
    void InitFluffyMovement();
    void UpdateFluffyMovement();
    void UpdateGeneration();
    void GenerateInitialBananas();
    void ClearGeneratedEntities();
    void UpdateFallOut();
    void UpdateEndGame();
    void Exit(bool);
    void SetState(FluffyGameplay::FluffyGameplayStates);
    void InitDebugSliders();
    void EndDebugSliders();
    void UpdateDebugSliders();
    void GenerateEntitiesInRow(int);
    void GenerateEntity(unsigned char, int, int);
    void LoadBackgrounds();
    void RandPoolSelection(Tilemap::TilemapPoolType, unsigned);
    short GetSelectionPoolW();
    short GetSelectionPoolH();
    unsigned GetSelectionRow(unsigned);
    FluffyGameplay::TilemapSelection &GetSelection(unsigned);
public:
    static const int k_start_time;
    static const int k_win_time;
    static jet::String k_tmplName;
private:
    ustl::vector<BackgroundPool *> m_backgrounds;  // 0x90
    BackgroundPool *m_currentLocationBackgroundPool;  // 0xA0
    GuiObject *m_tutorialGuiObject;  // 0xA4
    FluffyGameplay::FluffyGameplayStates m_state;  // 0xA8
    int m_timerCount;  // 0xAC
    bool m_cameraValuesSet;  // 0xB0
    float m_cameraLeft;  // 0xB4
    float m_cameraRight;  // 0xB8
    float m_screenWidth;  // 0xBC
    float m_bananaDownLimit;  // 0xC0
    GameEntity *m_bananaReference;  // 0xC4
    GameEntity *m_tokenReference;  // 0xC8
    math::vec3<float> m_originalMinionPosition;  // 0xCC
    math::quat<float> m_fluffyRotation;  // 0xD8
    math::vec3<float> m_fluffyPosition;  // 0xE8
    math::vec3<float> m_fluffyVelocity;  // 0xF4
    math::vec3<float> m_fluffyAcceleration;  // 0x100
    math::vec3<float> m_fluffyForce;  // 0x10C
    float m_fallingDistance;  // 0x118
    float m_topDistance;  // 0x11C
    bool m_isFallOut;  // 0x120
    bool m_hasReachedTop;  // 0x121
    math::vec3<float> m_initialCreationPos;  // 0x124
    std::list<GameEntity *,std::allocator<GameEntity *> > m_generatedEntities;  // 0x130
    math::vec2<float> m_oldCurvature;  // 0x148
    math::vec3<float> m_gravity;  // 0x150
    math::vec3<float> m_bananaBoostForce;  // 0x15C
    math::vec3<float> m_orientationForce;  // 0x168
    math::vec3<float> m_fluffyInitialVelocity;  // 0x174
    float m_maxSpeedOnFalling;  // 0x180
    float m_maxSpeedOnRising;  // 0x184
    float m_maxLateralSpeed;  // 0x188
    float m_maxAcceleration;  // 0x18C
    int m_lastCreatedRow;  // 0x190
    math::vec3<float> m_spawnColumnDir;  // 0x194
    math::vec3<float> m_spawnRowDir;  // 0x1A0
    float m_rowOffset;  // 0x1AC
    float m_columnOffset;  // 0x1B0
    math::vec3<float> m_lastSpawnPosition;  // 0x1B4
    float m_spawnInterval;  // 0x1C0
    Tilemap::TilemapPoolType m_nextTilemap;  // 0x1C4
    std::vector<std::vector<Tilemap *,std::allocator<Tilemap *> >,std::allocator<std::vector<Tilemap *,std::allocator<Tilemap *> > > > m_tilemapPools;  // 0x1C8
    std::vector<FluffyGameplay::TilemapSelection,std::allocator<FluffyGameplay::TilemapSelection> > m_tilemapSelection;  // 0x1DC
    bool m_wasSkyDomeVisible;  // 0x1F0
    static const jet::String k_pathParam;
    static const jet::String k_generationPeriodParam;
    static const jet::String k_fluffyBananaRefParam;
    static const jet::String k_fluffyTokenRefParam;
    static const jet::String k_bananaDownLimit;
    static const jet::String k_fluffySpeedMultiplierParam;
    static const jet::String k_fluffyAccelerationMultiplierParam;
    static const jet::String k_postFluffyInvulnerabilityTimeParam;
    static const jet::String k_fluffyCameraParam;
    static const jet::String k_fluffyCameraTransitionTimeParam;
    static const jet::String k_fluffyPostFXParam;
    static const jet::String k_fluffyEnableDebugParam;
    static const jet::String k_fluffyEnableSlidersParam;
    static const jet::String k_fluffyGravityParam;
    static const jet::String k_fluffyBananaBoostForceParam;
    static const jet::String k_fluffyOrientationForceParam;
    static const jet::String k_fluffyInitialVelocityParam;
    static const jet::String k_fluffyMaxSpeedOnFallingParam;
    static const jet::String k_fluffyMaxSpeedOnRisingParam;
    static const jet::String k_fluffyMaxLateralSpeedParam;
    static const jet::String k_fluffyMaxAccelerationParam;
    static const jet::String k_fluffyEasyTilemapPoolParam;
    static const jet::String k_fluffyMediumTilemapPoolParam;
    static const jet::String k_fluffyHardTilemapPoolParam;
    static const jet::String k_fluffyExtraTilemapPoolParam;
    static const jet::String k_FluffyBackgroundsParam;
    static const jet::String k_paramTutorialGuiObject;
    static const float k_fluffyTiltAngularSpeedInv;
    static const float k_maxFallingDistance;
    static const float k_maxExitDistance;
    static const float k_fluffyMass;
    static const float k_fluffyMassI;
    static const int k_timePorExtraTilemap;
};

#endif
