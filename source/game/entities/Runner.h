// source/game/entities/Runner.h
#ifndef SOURCE_GAME_ENTITIES_RUNNER_H
#define SOURCE_GAME_ENTITIES_RUNNER_H

#include "Actor.h"
#include "pacesetterfollower.h"
#include "ObstacleTemplate.h"
#include "game/gameenums.h"

class Pacesetter;
class Minion;

// An actor that runs in a lane of the level beside the minion, on a pacesetter of its own (the data's "Runner" template). It starts running
// when the minion comes to it, stops when the minion touches it (its engine event for the minion, and the minion's despicable points) or
// passes it, and is deleted with its template. Its pacesetter takes the runner's lane and speed (the minion's speed times the data's factor).
// Its members start at 0x1DC in the .so (after the PacesetterFollower at 0x1CC and the ObstacleTarget at 0x1D4), and the pacesetter it owns is at 0x200.
// The runners of a large minion wave (LargeMinionGameplay::UpdateRunnerSpawn sets the flag at 0x1F8) take the large minion branch of the running
// state (UpdateLargeMinionGameplay @0x520958). The second touch test of Update (block D @0x520d54) is UpdateObstacleEffectTouch.
class Runner : public Actor, public PacesetterFollower, public ObstacleTarget {
public:
    // the states of the runner (the .so's RunnerState): waiting for the minion, running, stopping after a hit, stopped
    struct RunnerState { enum { Waiting = 0, Running = 1, Stopping = 2, Stopped = 3 }; };
    Runner(const clara::Template*);
    virtual ~Runner();
    virtual void PostInit();
    virtual void Update();
    virtual bool IsOutOfScene() const;
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&);
    virtual void OnGameplayTemplateInstanceDestroyed();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual void OnObstacleHit(ObstacleTemplate*);
    virtual void SetSpawnLane(int);
    virtual void OnMinionRevive();
    void SetSpeedPgt(float, int);
    void SetLargeMinionRunner(bool b) { m_largeMinionRunner = b; }  // @0x56fcbc: the flag the large minion's spawn sets at 0x1F8
    static jet::String k_tmplName;
private:
    void SetRunnerState(int);
    void InitPacesetter();
    void StartPacesetter();
    void StopPacesetter();
    float GetDistanceSqToPlane(const math::vec3<float>&, const math::quat<float>&) const;
    void UpdateNPCVoiceTrigger();
    void UpdateLargeMinionGameplay(float distSq);  // @0x520958
    void UpdateObstacleEffectTouch(Minion *minion);  // @0x520d54
    int m_runnerState;                  // 0x1DC
    float m_startTime;                  // 0x1E0: the distance (scaled) at which the minion starts it
    float m_distanceToStopSq;           // 0x1E4
    LaneIndex m_spawnLane;              // 0x1E8
    float m_speedPgt;                   // 0x1EC
    int m_speedTime;                    // 0x1F0
    int m_minionEngineEventOnHit;       // 0x1F4
    bool m_largeMinionRunner;           // 0x1F8: the runner of a large minion wave (set by LargeMinionGameplay::UpdateRunnerSpawn)
    bool m_triggerNPCVoice;             // 0x1F9
    float m_despicablePoints;           // 0x1FC
    Pacesetter *m_pacesetter;           // 0x200: the pacesetter the runner owns
    static const jet::String k_speedPgtParamName;
    static const jet::String k_speedTimeParamName;
    static const jet::String k_startTimeParamName;
    static const jet::String k_endDistanceParamName;
    static const jet::String k_despicablePointsParamName;
    static const jet::String k_EventOnMinionHitParamName;
    static const float k_distanceToTriggerNPCVoiceSq;
    static const jet::String k_NPCVoiceLabel;
    static const jet::String k_NPCSfxRunningLabel;
};

#endif
