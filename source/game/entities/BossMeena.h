// source/game/entities/BossMeena.h
#ifndef SOURCE_GAME_ENTITIES_BOSSMEENA_H
#define SOURCE_GAME_ENTITIES_BOSSMEENA_H

#include "Boss.h"
#include "core/delayer.h"
#include "../input/InputMgr.h"

// Meena: a boss that runs along the lanes of the minion's pacesetter, changes lanes (the lane change is a delayer of the lane offset, and the
// lane the boss is in follows the offset's whole part), and rams the minion when it is in its lane and close (the ram's engine events, and its
// win event when the ram ends). Her members start at 0x214 in the .so. The second base is the input tutorial's listener (the .so's 0x210): the
// first ram shows a tutorial (SM_OnEngineEvent) that ends when its input is done (OnTutorialInputDone).
class BossMeena : public Boss, public InputMgr::TutorialInputListener {
public:
    BossMeena(const clara::Template*);
    virtual ~BossMeena();
    virtual void PostInit();
    // @0x5a7684 (the listener's virtual, reached through the thunk at 0x5a7728): the input's tutorial is done
    virtual void OnTutorialInputDone(const InputMgr::TutorialInputId&);
    // @0x5a74f8: whether the boss is in the lane asked for
    bool IsInLane(LaneIndex);
    // @0x5a750c: whether the boss is in the lane and close enough to the minion to block it
    bool IsBlockingMinion(LaneIndex);
    virtual void StartAttack();
    virtual void OnShoot();
    virtual bool CanShoot() const;
    virtual math::vec3<float> GetBombShootPosition() const;
    virtual math::vec3<float> GetShootPosition() const;
    virtual const jet::String &GetBossSoundLabel() const;
    virtual int GetStartMinionEvent() const;
    virtual int GetEndMinionEvent() const;
    virtual int GetBossStartDyingMinionEvent() const;
    virtual int GetBossExitMinionEvent() const;
    virtual void OnMinionHittedByBossObstacle(ObstacleTemplate*);
    virtual bool SM_OnEngineEvent(int);
    static jet::String k_tmplName;
protected:
    virtual void UpdatePersecution();
    virtual void SM_OnChangeState(ActorState*, ActorState*);
    virtual void OnDie();
private:
    // @0x5a7150: the ram: the minion in the boss's lane and close to it starts the ram (the crash event goes to the minion, and the ram's win event
    // to the boss)
    void UpdateMeleeAttack();
    // @0x5a7270: the lane the boss is in follows the lane change's whole part
    void UpdateCurrentLane();
    // @0x5a7344: the boss's place on the lanes of the minion's pacesetter
    void UpdatePosition();
    // @0x5a7684: the tutorial is done: its message is hidden, the input's tutorial is cleared, the time runs again, and the ram's tutorial is kept
    void OnTutorialDoneCallback();

    int m_lastLane;                                                  // 0x214: the lane it was in before the last state change
    int m_currentLane;                                               // 0x218: the lane it is in
    jet::core::Delayer<float, jet::core::LinearStep> m_laneDelayer;  // 0x220: the lane change (origin, current, target), in lanes
    float m_offsetInPath;                                            // 0x238: how far the boss is along the path (the offset's target)
    jet::core::Delayer<float, jet::core::QuadraticEaseInOut> m_offsetInPathDelayer;  // 0x23C: the offset along the path (updating at 0x250)
    math::quat<float> m_originalRot;                                 // 0x254: the rotation the boss has when it is made
    int m_changeLaneWaitTime;                                        // 0x264: ms the boss waits before it changes lane
    int m_changeLaneTime;                                            // 0x268: ms the lane change takes
    int m_engineEventOnCrash;                                        // 0x26C: the minion's engine event when the ram hits it (-1 when none)
    int m_ramAttackTime;                                             // 0x270: ms of the ram's attack (the offset goes to 0 in it)
    int m_ramBackTime;                                               // 0x274: ms the ram goes back in (the offset goes to the path's offset)
    int m_ramWinEngineEvent;                                         // 0x278: the boss's engine event when the ram is done
    bool m_ramTutoDisplayed;                                         // 0x27C: the ram's tutorial has been shown (Player's flag)

    static const jet::String k_offsetInPathParam;
    static const jet::String k_changeLaneWaitTimeParam;
    static const jet::String k_changeLaneTimeParam;
    static const jet::String k_engineEventOnCrashParam;
    static const jet::String k_ramAttackTimeParam;
    static const jet::String k_ramBackTimeParam;
    static const jet::String k_ramWinEngineEventParam;
    static const jet::String k_puppetShootDummy;
    static const jet::String k_meenaSoundLabel;
};

#endif
