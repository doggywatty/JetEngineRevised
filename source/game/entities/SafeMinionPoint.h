// source/game/entities/SafeMinionPoint.h
#ifndef SOURCE_GAME_ENTITIES_SAFEMINIONPOINT_H
#define SOURCE_GAME_ENTITIES_SAFEMINIONPOINT_H

#include "GameEntity.h"
#include "game/leveltemplate/gameplaytemplateinstancelistener.h"

class GameplayTemplateInstance;

// A place of a platform the minion can be put back on after a fall or a revive (the data's "SafeMinionPoint", 0xB4 bytes in the .so): it is
// registered with the level while its template lives, with the plane across the way at it.
class SafeMinionPoint : public GameEntity, public GameplayTemplateInstanceListener {
public:
    explicit SafeMinionPoint(const clara::Template*);
    virtual ~SafeMinionPoint();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render() {}
    virtual void OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent&);
    // the point is registered with the level and listens to the template it was made in
    void Register(GameplayTemplateInstance*);
    void ComputeCollisionPlane();
    float GetDistance(const math::vec3<float>&);
    float GetDistanceToPlane(const math::vec3<float> &p) { return m_plane.getDistanceTo(p); }
    virtual void SetSpawnLane(int lane) { m_lane = lane; }
    int GetSpawnLane() const { return m_lane; }
    GameplayTemplateInstance *GetContainerGameplayTemplateInstance() const { return m_notifier; }
    bool GetIsLandable() const { return m_isLandable; }
    static jet::String k_tmplName;
    static const jet::String k_isLandableParam;
    // the distance (squared) within which the minion checks the point
    static const float k_distanceToCheckSq;
private:
    void RegisterSafeMinionPoint();
    void UnregisterSafeMinionPoint();
    void RegisterToNotifier(GameplayTemplateInstance*);
    void UnregisterFromNotifier();
    bool m_isRegistered;                       // 0x90
    GameplayTemplateInstance *m_notifier;      // 0x94
    math::plane<float> m_plane;                // 0x98
    int m_lane;                                // 0xA8
    bool m_unregisterFromNotifier;             // 0xAC
    bool m_isLandable;                         // 0xAD
};

#endif
