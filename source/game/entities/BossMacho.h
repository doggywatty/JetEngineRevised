// source/game/entities/BossMacho.h
#ifndef SOURCE_GAME_ENTITIES_BOSSMACHO_H
#define SOURCE_GAME_ENTITIES_BOSSMACHO_H

#include "Boss.h"
#include "core/delayer.h"

// El Macho: a boss that changes lanes along the minion's pacesetter (its lane offset eases between the lanes, the path offset eases in along
// the pacesetter's front). Its members start at 0x210 in the .so, after the Boss part, and end at 0x268. Its engine events are the shot 0x7B,
// the minion's start and end 0x7E and -1, and the dying and the exit 0x7F; its sound label is the empty string.
class BossMacho : public Boss {
public:
    BossMacho(const clara::Template*);
    virtual ~BossMacho();
    virtual void PostInit();
    // @0x5a6998: whether the boss is on the lane (its current lane is the one asked for)
    bool IsInLane(LaneIndex) const;
    virtual void StartAttack();
    virtual void OnShoot();
    virtual math::vec3<float> GetBombShootPosition() const;
    virtual math::vec3<float> GetShootPosition() const;
    virtual const jet::String &GetBossSoundLabel() const;
    virtual int GetStartMinionEvent() const;
    virtual int GetEndMinionEvent() const;
    virtual int GetBossStartDyingMinionEvent() const;
    virtual int GetBossExitMinionEvent() const;
    virtual void OnMinionHittedByBossObstacle(ObstacleTemplate*);
    virtual void OnDie();
    static jet::String k_tmplName;
protected:
    virtual void UpdatePersecution();
private:
    // @0x5a67e4: the boss's place on the lanes of the minion's pacesetter
    void UpdatePosition();
    int m_lastLane;                                     // 0x210: the lane it was in before (set to 1 by the constructor; its readers are not decoded)
    int m_lane;                                         // 0x214: the lane it is in
    jet::core::Delayer<float, jet::core::LinearStep> m_laneDelayer;  // 0x218: the lane change (origin, current, target), in lanes
    float m_offsetInPath;                               // 0x230: how far the boss is along the path (the offset's target)
    jet::core::Delayer<float, jet::core::QuadraticEaseInOut> m_offsetInPathDelayer;  // 0x234: the offset along the path
    math::quat<float> m_originalRot;                    // 0x24C: the rotation the boss has when it is made
    int m_changeLaneWaitTime;                           // 0x25C: ms the boss waits before it changes lane
    int m_changeLaneTime;                               // 0x260: ms the lane change takes
    int m_engineEventOnCrash;                           // 0x264: the engine event the minion gets when the boss is crashed into (-1 when none)
    static const jet::String k_offsetInPathParam;
    static const jet::String k_changeLaneWaitTimeParam;
    static const jet::String k_changeLaneTimeParam;
    static const jet::String k_engineEventOnCrashParam;
    static const jet::String k_machoSoundLabel;
};

#endif
