// source/game/entities/SpeederTemplate.h
#ifndef SOURCE_GAME_ENTITIES_SPEEDERTEMPLATE_H
#define SOURCE_GAME_ENTITIES_SPEEDERTEMPLATE_H

#include "Object.h"
#include "SynchronizedObject.h"

// A speeder: the minion that runs onto it is pushed forward a distance in a time (the data's "SpeederTemplate", type 0x12; 0x180 bytes in the
// .so). The contact is CollidesWith (@0x522200). Not ported yet: GameLevel::NotifyIncreasedSpeed (@0x53ebb4, see JumperTemplate.h), which the
// contact calls after the speeder statistic.
class SpeederTemplate : public Object, public SynchronizedObject {
public:
    explicit SpeederTemplate(const clara::Template*);
    virtual ~SpeederTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual void SetSpawnLane(int lane) { SynchronizedObject::SetSpawnLane((LaneIndex)lane); }
    virtual void OnMinionRevive();
    float GetDistance() const { return m_distance; }
    float GetTime() const { return m_time; }
    bool IsReady() const { return m_sendEngineEvent; }
    void SetReady(bool b) { m_sendEngineEvent = b; }
    static jet::String k_tmplName;
    static const jet::String k_distanceParam, k_timeParam;
private:
    float m_distance;        // 0x150
    float m_time;            // 0x154
    bool m_sendEngineEvent;  // 0x158
};

#endif
