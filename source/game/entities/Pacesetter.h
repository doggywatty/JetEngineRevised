// source/game/entities/Pacesetter.h
#ifndef SOURCE_GAME_ENTITIES_PACESETTER_H
#define SOURCE_GAME_ENTITIES_PACESETTER_H

#include <vector>
#include "GameEntity.h"
#include "pacesettercontroller.h"
#include "core/delayer.h"
#include "../leveltemplate/gameplaytemplateinstancelistener.h"
#include "boost/shared_ptr.hpp"
#include "game/gameenums.h"

class GameplayTemplateInstance;
class PacesetterFollower;
class PathCommon;
class JumpDefinition;
struct JumpDefinitionControl;

// The point the minion's run follows along the level template lanes: it moves at a speed, changes lane and reports to its followers.
// A controller (the rocket's path) can take it over.
class Pacesetter : public GameEntity, public GameplayTemplateInstanceListener {  // sizeof 0x140
public:
    // what the followers are told: the controller took (0) or gave back (1) the pacesetter
    struct PacesetterEvent { int type; };
    struct PacesetterEventType { enum { ControlStarted = 0, ControlEnded = 1 }; };
    // the place found in the lane of a template, for the next call to start from
    struct PacestterCacheData { GameplayTemplateInstance *instance; unsigned segment; float t; };
    static jet::String k_tmplName;
    explicit Pacesetter(const clara::Template *t);
    virtual ~Pacesetter();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render() {}
    virtual const jet::String &GetBaseTemplateName() const { return k_tmplName; }
    virtual void NotifyTransformChanged(bool);
    void RegisterFollower(PacesetterFollower*);
    void UnregisterFollower(PacesetterFollower*);
    void SetController(PacesetterController*);
    PacesetterController *GetController() const { return m_controller; }
    void SendPacesetterEvent(const PacesetterEvent&);
    // the template instance the pacesetter runs on: it is told when that one goes
    void SetCurrentGameplayTemplateInstance(GameplayTemplateInstance*);
    GameplayTemplateInstance *GetCurrentGameplayTemplateInstance() const { return m_currentGti; }
    virtual void OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent&);
    LaneIndex GetCurrentLane() const { return (LaneIndex)m_currentLane; }
    unsigned GetCurrentSegment() const { return m_segment; }
    float GetCurrentSegmentT() const { return m_t; }
    float GetSpeed() const { return m_speed.GetCurrentValue(); }
    float GetTargetSpeed() const { return m_speed.GetTargetValue(); }
    // changes the speed to the given one in time ms (negative: as the acceleration takes)
    void SetSpeed(float speed, int time = -1);
    void SetSpeedAcceleration(float a) { m_speedAcceleration = a; }
    // holds a change of speed for ms
    void HoldSpeedChange(int ms) { m_speedHold = ms; }
    // the distance of this frame is this (once)
    void ForceStep(float distance) { m_forcedStep = distance; }
    float GetStep() const { return m_step; }
    bool IsSpeedChanging() const { return m_speed.IsUpdating(); }
    // @0x575974: after a crash the pacesetter is at the start of its segment, in the middle of its lane
    void ResetLaneForCrash();
    const math::vec3<float> &GetLaneCachePosition(unsigned lane) const { return m_laneCachePos[lane]; }
    void SetPaused(bool b) { m_paused = b; }
    bool IsPaused() const { return m_paused; }
    void SetLaneCheckDisabled(bool b) { m_laneCheckDisabled = b; }
    // what the minion jumps along: it replaces the step by the jump's movement
    void SetJump(JumpDefinition *j, JumpDefinitionControl *c) { m_jump = j; m_jumpCtrl = c; }
    // lanes: the one asked for (3 is the current one)
    void GoToLane(LaneIndex lane, int time);
    void SetChangeLaneMu(float mu, int time);
    float GetChangeLaneMu() const { return m_laneMu.GetCurrentValue(); }
    float GetTargetChangeLaneMu() const { return m_laneMu.GetTargetValue(); }
    void GetTransformInPath(math::vec3<float>&, math::quat<float>&, LaneIndex lane = (LaneIndex)3);
    void GetAdvancedTransformInPath(float distance, math::vec3<float>&, math::quat<float>&, LaneIndex lane = (LaneIndex)3);
    bool GetAdvancedInfo(float distance, LaneIndex lane, GameplayTemplateInstance *&gti, boost::shared_ptr<PathCommon> &path, unsigned &segment, float &t);
    bool GetAdvancedSegmentNodeTransformInPath(int segments, math::vec3<float>&, math::quat<float>&, LaneIndex lane = (LaneIndex)3);
    float GetTraveledDistanceOnCurrentPath();
    void ApplyCurrentLaneTransform();
    int GetCurrentDummyIndex(LaneIndex);
    bool CanReachLane(LaneIndex, bool checkCollisions) const;
    bool IsInSingleLane() const;
    bool CanDodgeInConditionalBifurcation() const;
    bool CanDodgeLeft(bool checkCollisions) const;
    bool CanDodgeRight(bool checkCollisions) const;
    // puts the pacesetter at the closest place of the lane to the point (going on to the following templates), advanced by a distance;
    // offset gets how far the point is from the lane
    bool SetLanePosition(const math::vec3<float> &pos, float advance, LaneIndex lane, math::vec3<float> *offset);
    // how far the point is along the lane from here (positive ahead); false when the templates do not reach it
    bool GetDistanceInLane(const math::vec3<float> &pos, float &distance, std::vector<PacestterCacheData> &cache, float &direction, LaneIndex lane);
    float GetTimeToReachPlane(const math::vec3<float> &pos, const math::quat<float> &rot, std::vector<PacestterCacheData> &cache, float *distance);
private:
    void _Update();
    void UpdateCurrentLane();
    GameplayTemplateInstance *m_currentGti;  // 0x90
    int m_currentLane;                       // 0x94
    unsigned m_segment;                      // 0x98
    float m_t;                               // 0x9C
    JumpDefinition *m_jump;                  // 0xA0
    JumpDefinitionControl *m_jumpCtrl;       // 0xA4
    float m_jumpSpeed;                       // 0xA8
    jet::core::Delayer<float, jet::core::LinearStep> m_laneMu;  // 0xAC: how far between lanes, in lanes
    jet::core::Delayer<float, jet::core::LinearStep> m_speed;   // 0xC4
    float m_speedAcceleration;               // 0xDC
    int m_speedHold;                         // 0xE0
    bool m_paused;                           // 0xE4
    bool m_isBonusNode;                      // 0xEC
    bool m_prevLaneValid;                    // 0xED
    float m_forcedStep;                      // 0xF0
    float m_step;                            // 0xF4
    bool m_laneCheckDisabled;                // 0xF8
    std::vector<math::vec3<float> > m_laneCachePos;   // 0xFC
    std::vector<math::quat<float> > m_laneCacheRot;   // 0x10C
    std::vector<bool> m_laneCacheValid;               // 0x11C
    std::vector<PacesetterFollower*> m_followers;  // 0x130
    PacesetterController *m_controller;      // 0x13C
};

#endif
