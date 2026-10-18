// source/game/entities/InteractiveAnimatedPusher.h
#ifndef SOURCE_GAME_ENTITIES_INTERACTIVEANIMATEDPUSHER_H
#define SOURCE_GAME_ENTITIES_INTERACTIVEANIMATEDPUSHER_H

#include "Object.h"
#include "SynchronizedObject.h"
#include "core/pstring.h"
#include "smath.h"

class ActorState;
class JumpDefinition;
class Launcher;

// A pusher (the data's "InteractiveAnimatedPusher"; 0x1AC bytes in the .so): the minion that runs onto it is pushed along the animation
// on contact. It waits (Idle) for a minion that touches it and counts the jumper statistic for it, then the minion is taken over (Pushing:
// Minion::OnInteractiveAnimatedPusherStart); the pusher lets it go (Released) when the minion changes state, or at once when the launcher
// activates another pusher (Launcher::ActivatePusher, which sets the state directly).
class InteractiveAnimatedPusher : public Object, public SynchronizedObject {
public:
    enum PusherState { Idle = 0, Pushing = 1, Released = 2 };

    explicit InteractiveAnimatedPusher(const clara::Template*);
    virtual ~InteractiveAnimatedPusher();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual void SetSpawnLane(int lane) { SynchronizedObject::SetSpawnLane((LaneIndex)lane); }
    virtual void OnMinionRevive();
    PusherState GetPusherState() const { return (PusherState)m_pusherState; }

    static jet::String k_tmplName;
protected:
    // @0x56c400: the state of the pusher; Pushing starts the minion's push, Released stops it
    void SetPusherState(PusherState);
private:
    friend class Launcher;

    int m_pusherState;                    // 0x150
    jet::String m_animOnContact;          // 0x154
    jet::String m_stateOnContact;         // 0x158
    JumpDefinition *m_jumpDefinition;     // 0x15C
    float m_frontSpeed;                   // 0x160
    float m_upSpeed;                      // 0x164
    int m_frontSpeedRestoreTime;          // 0x168
    int m_engineEventOnPushed;            // 0x16C: -1 for none
    bool m_countForJumperStatistics;      // 0x170
    math::vec2<float> m_minSpeedInfoPair; // 0x174: the pacesetter speed and the animation speed (percent) at the minimum
    math::vec2<float> m_maxSpeedInfoPair; // 0x17C: and at the maximum
    ActorState *m_minionPlayingState;     // 0x184: the state of the minion when it was pushed
    clara::Entity *m_exitPoint;           // 0x188
    bool m_ownedByLauncher;               // 0x18C: set on the pusher a launcher made

    static const jet::String k_animOnContact, k_stateOnContact, k_jumpDefinition, k_upSpeed, k_frontSpeed, k_frontSpeedRestoreTime,
        k_minSpeedInfoPair, k_maxSpeedInfoPair, k_useEngineEventParam, k_engineEventParam, k_countForJumperStatistics, k_exitPointParam;
};

#endif
