// source/game/entities/Rocket.h
#ifndef SOURCE_GAME_ENTITIES_ROCKET_H
#define SOURCE_GAME_ENTITIES_ROCKET_H

class Rocket : public Mount {  // sizeof 0x298
public:
    Rocket(const clara::Template*);
    virtual ~Rocket();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void UpdateSpecific();
    virtual void OnRideEngage();
    virtual void OnRideStart();
    virtual void OnRideEnded();
    virtual void OnRideDisengage();
    virtual void OnRideDisengaged();
    virtual void OnStrafeLeft();
    virtual void OnStrafeRight();
    virtual void OnNitroStart();
    virtual void OnNitroEnd();
    virtual void InitPathHeights();
    virtual void UpdatePathHeights();
    virtual void HandleAccelerometer(float, float);
    void SetRocketPostFxMaterial(MaterialDef*);
    MaterialDef *GetRocketPostFxMaterial();
    virtual void SM_OnChangeState(ActorState*, ActorState*);
    virtual int GetRideInitStateLabel();
    virtual int GetRideRunStateLabel();
    virtual int GetRideEngageStateLabel();
    virtual int GetRideEndStateLabel();
    virtual int GetRideDisengageStateLabel();
    float GetFlyingAltitude();
    void SetFlyingAltitude(float);
    void GetLastPathSegmentIndexAndHeight(int&, float&);
    unsigned GetPathHeightCount();
    float GetPathHeight(unsigned);
    virtual float GetSpeedMultiplier();
    void SetNitro(float, int);
    void SetPostRocketInvulnerabilityTime(float);
    float GetPostRocketInvulnerabilityTime();
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&);
    virtual void OnPacesetterEvent(const Pacesetter::PacesetterEvent&);
    void GetRelativePositionFromCenterLane(math::vec3<float>&);
    virtual math::quat<float> GetTargetRotation();
    virtual math::vec3<float> GetTargetPosition();
private:
    void UpdateTargetOffset();
public:
    static jet::String k_tmplName;
private:
    static jet::String ms_rocketName;
    bool m_isFlying;  // 0x218
    float m_postRocketInvulneribilityTime;  // 0x21C
    jet::core::Delayer<float,jet::core::LinearStep> m_flyingDistanceDelayer;  // 0x220
    jet::core::Averager<float> m_heightAverager;  // 0x238
    std::deque<float,std::allocator<float> > m_pathHeights;  // 0x250
    int m_lastSegmentComputed;  // 0x26C
    MaterialDef *m_rocketPostFxMaterial;  // 0x270
    int m_segmentsAdvanced;  // 0x274
    float m_flyingAltitude;  // 0x278
    float m_nitroMultiplier;  // 0x27C
    int m_nitroTimer;  // 0x280
    math::vec3<float> m_relativeEnterPathPos;  // 0x284
    int m_pathInterpolationTimer;  // 0x290
    float m_rightTargetOffset;  // 0x294
    static const int k_pathInterpolationTime;
    static const float k_standardPostInvulnerabilityTime;
    static const float k_changeLineSpeed;
    static const float k_rocketDefaultSpeedMultiplier;
    static const unsigned k_rocket_precomputedHeights;
    static const int k_rocket_heightAveragerSamplesPerSide;
    static const int k_rocket_heightAveragerTotalSamples;
    static const unsigned short k_rocket_collisionGroup;
    static const unsigned short k_rocket_collisionMask;
};

#endif
