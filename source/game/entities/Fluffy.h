// source/game/entities/Fluffy.h
#ifndef SOURCE_GAME_ENTITIES_FLUFFY_H
#define SOURCE_GAME_ENTITIES_FLUFFY_H

class Fluffy : public Mount {  // sizeof 0x238
public:
    Fluffy(const clara::Template*);
    virtual ~Fluffy();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void Reset();
    virtual void OnRideEngage();
    virtual void OnRideStart();
    virtual void OnRideEnded();
    virtual void OnRideDisengage();
    virtual void OnRideDisengaged();
    void OnGoingRight();
    void OnGoingLeft();
    void OnPush();
    virtual void OnPushItemCollect();
    int ConsumePushItems();
    void SetPostFluffyInvulnerabilityTime(float);
    void SetFluffyPostFxMaterial(MaterialDef*);
    int GetPostFluffyInvulnerabilityTime();
    MaterialDef *GetFluffyPostFxMaterial();
    virtual void HandleAccelerometer(float, float);
    virtual void HandleHoldInput();
    float GetTiltAngle();
    math::quat<float> ComputeTiltRotation();
    void EnableDebug(bool);
    void EnableSliders(bool);
    bool IsDebugEnabled();
    bool IsSliderEnabled();
    static jet::String k_tmplName;
private:
    static jet::String ms_fluffyName;
    int m_pushItem;  // 0x218
    float m_lastTiltAngle;  // 0x21C
    float m_postFluffyInvulneribilityTime;  // 0x220
    MaterialDef *m_fluffyPostFxMaterial;  // 0x224
    bool m_enableDebug;  // 0x228
    bool m_enableSliders;  // 0x229
    math::vec3<float> m_decoOriginalLocalPos;  // 0x22C
    static const float k_standardPostInvulnerabilityTime;
    static const float k_angleVariation;
    static const float k_initAngle;
    static const float k_minAngle;
    static const float k_maxAngle;
    static const float k_hideDecoOffset;
};

#endif
