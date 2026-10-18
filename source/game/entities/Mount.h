// source/game/entities/Mount.h
#ifndef SOURCE_GAME_ENTITIES_MOUNT_H
#define SOURCE_GAME_ENTITIES_MOUNT_H

class Mount : public Actor, public PacesetterFollower {  // sizeof 0x218
public:
    Mount(const clara::Template*);
    virtual ~Mount();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void Reset();
    virtual void SM_OnChangeState(ActorState*, ActorState*);
    void EnableMount(Minion*);
    void DisableMount();
    virtual void OnMovingUp();
    virtual void OnMovingTop();
    virtual void OnMovingDown();
    virtual void OnGroundHitted();
    virtual void OnStrafeLeft();
    virtual void OnStrafeRight();
    virtual void OnPushItemCollect();
    virtual void OnRideEngage() = 0;
    virtual void OnRideStart() = 0;
    virtual void OnRideEnded() = 0;
    virtual void OnRideDisengage() = 0;
    virtual void OnRideDisengaged() = 0;
    virtual void ForceDismount();
    void ChangePhysicsValue(Mount::ChangeablePhysicsDef, float);
    float GetPhysicsValue(Mount::ChangeablePhysicsDef);
    virtual const jet::String &GetBaseTemplateName();
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&);
    virtual void HandleAccelerometer(float, float);
    virtual void HandleHoldInput();
    bool IsMountEnabled();
    virtual int GetRideInitStateLabel();
    virtual int GetRideRunStateLabel();
    virtual int GetRideEngageStateLabel();
    virtual int GetRideEndStateLabel();
    virtual int GetRideDisengageStateLabel();
    virtual float GetSpeedMultiplier();
    virtual float GetAccelerationMultiplier();
    virtual float GetDeaccelerationMultiplier();
    virtual math::quat<float> GetTargetRotation();
    virtual math::vec3<float> GetTargetPosition();
    static jet::String k_tmplName;
protected:
    Minion *m_desiredRider;  // 0x1E4
    Minion *m_rider;  // 0x1E8
    Mount::ChangeablePhysicsValues m_physicsValues;  // 0x1EC
};

#endif
